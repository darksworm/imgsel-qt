#include "ImagePickerDrawer.h"

#include <utility>
#include <memory>

#include "../exceptions/OutOfBounds.h"
#include "../exceptions/ImageNotLoadable.h"
#include "../config/ConfigManager.h"

#include "drawer/ImageDrawer.h"
#include "ImagePickerMove.h"

ImagePickerDrawer::ImagePickerDrawer(QPixmap &pixmap) : pixmap(pixmap) {
    auto config = ConfigManager::getOrLoadConfig();

    this->page = 0;
    this->selectedShape = nullptr;

    this->allImages = config.getImages();
    this->images = config.getImages();

    shapeDrawer = new ImageDrawer(pixmap);
    shapeProperties = shapeDrawer->calcShapeProps();
}

void ImagePickerDrawer::drawFrame(Image *selectedImage, bool redrawAll) {
    auto config = ConfigManager::getOrLoadConfig();
    auto start = getPageImageStart();

    auto oldShapes = std::map<long, Shape>();
    oldShapes.insert(shapes.begin(), shapes.end());

    shapes.clear();
    std::optional<XPoint> lastShapePosition;
    lastShapePosition.reset();

    std::unique_ptr<Dimensions> windowDimensions(new Dimensions);

    windowDimensions->x = config.getScreenGeometry().width();
    windowDimensions->y = config.getScreenGeometry().height();

    unsigned int shapeCnt = shapeProperties.itemCounts.x * shapeProperties.itemCounts.y;
    int drawnShapeCnt = 0;

    auto it = start;
    redrawAll = redrawAll || redrawAllInNextFrame;

    for (; it != images.end(); ++it) {
        if (filter.has_value() && !filter.operator*()(&*it)) {
            continue;
        }

        if (selectedImage == nullptr) {
            selectedImage = &*it;
        }

        bool selected = it->getPath() == selectedImage->getPath();

        Shape shape{
                .position = XPoint(),
                .selected = false,
                .index = std::distance(images.begin(), it),
                .image = &*it
        };

        bool shouldDrawShape = true;

        try {
            auto oldShape = oldShapes.at(drawnShapeCnt);

            if (!redrawAll && oldShape.image->getPath() == shape.image->getPath()) {
                shouldDrawShape = false;
                shape = oldShape;
                lastShapePosition = oldShape.position;

                if (oldShape.selected && !selected) {
                    shapeDrawer->clearSelectedShapeIndicator(shapeProperties, oldShape);
                }
            } else {
                if (!this->shapes.empty()) {
                    lastShapePosition = (--this->shapes.end())->second.position;
                }

                shapeDrawer->clearShape(shapeProperties, oldShape);
            }
        } catch (std::out_of_range &e) {
            // nothing to do here
        }

        shape.selected = selected;

        if (shouldDrawShape) {
            shapeProperties.position = shapeDrawer->getNextShapePosition(shapeProperties, *windowDimensions, lastShapePosition);
            shape.position = shapeProperties.position;

            try {
                shapeDrawer->drawNextShape(shapeProperties, shape);
            } catch (ImageNotLoadable &e) {
                images.erase(it);
                if (--it < start) {
                    break;
                } else {
                    continue;
                }
            }

            lastShapePosition = shape.position;
        }

        shapes.emplace(drawnShapeCnt, shape);

        if (selected) {
            this->selectedShape = &(--this->shapes.end())->second;
            shapeDrawer->drawSelectedShapeIndicator(shapeProperties, shape);
        }

        if (++drawnShapeCnt >= shapeCnt) {
            break;
        }
    }

    // Clear all old trailing images
    if (drawnShapeCnt < shapeCnt && oldShapes.size() > drawnShapeCnt) {
        unsigned i = drawnShapeCnt;
        do {
            auto oldShape = oldShapes.at(i);
            shapeDrawer->clearShape(shapeProperties, oldShape);} while (++i < oldShapes.size());
    }

    redrawAllInNextFrame = false;
}

std::vector<Image>::iterator ImagePickerDrawer::getPageImageStart() {
    int hotkeysPerPage = shapeProperties.itemCounts.y * shapeProperties.itemCounts.x;

    preloadToIndex(hotkeysPerPage - 1);

    if (page > 0 && images.size() < hotkeysPerPage) {
        throw OutOfBounds();
    }

    int offset = hotkeysPerPage * page;

    return images.begin() + offset;
}

void ImagePickerDrawer::preloadToIndex(unsigned int targetIndex) {
    if (filter.has_value() && images.size() < (long) targetIndex + 1) {
        unsigned int offset = 0;

        if (lastPreloadedImageIndex) {
            if (allImages.begin() + lastPreloadedImageIndex + 1 == allImages.end()) {
                return;
            } else {
                offset = lastPreloadedImageIndex + 1;
            }
        }

        unsigned int hotkeysPerPage = shapeProperties.itemCounts.y * shapeProperties.itemCounts.x;
        unsigned int targetImageCount = ((targetIndex / hotkeysPerPage) + 1) * hotkeysPerPage;

        for (auto it = allImages.begin() + offset; it != allImages.end(); ++it) {
            if (filter.operator*()(&*it)) {
                images.push_back(*it);
                lastPreloadedImageIndex = std::distance(allImages.begin(), it);

                if (images.size() >= targetImageCount) {
                    break;
                }
            }
        }
    }
}

int ImagePickerDrawer::getImagePage(long index) {
    return (int) (index / (this->shapeProperties.itemCounts.x * this->shapeProperties.itemCounts.y));
}

void ImagePickerDrawer::goToImage(long hotkeyIdx) {
    Image *image = &*(images.begin() + hotkeyIdx);

    page = getImagePage(hotkeyIdx);
    drawFrame(image);
}

bool ImagePickerDrawer::move(ImagePickerMove move, unsigned int steps) {
    bool canMove = false;
    long newSelectedShapeIdx = 0;

    switch (move) {
        case ImagePickerMove::LEFT:
            canMove = selectedShape->index >= steps;
            newSelectedShapeIdx = selectedShape->index - steps;
            break;
        case ImagePickerMove::RIGHT:
            preloadToIndex(selectedShape->index + steps);
            canMove = selectedShape->index + steps < images.size();
            newSelectedShapeIdx = selectedShape->index + steps;
            break;
        case ImagePickerMove::UP:
            canMove = selectedShape->index - (steps * shapeProperties.itemCounts.x) >= 0;
            if (canMove) {
                newSelectedShapeIdx = selectedShape->index - (steps * shapeProperties.itemCounts.x);
            } else {
                newSelectedShapeIdx = selectedShape->index % shapeProperties.itemCounts.x;
                canMove = true;
            }
            break;
        case ImagePickerMove::DOWN:
            preloadToIndex(selectedShape->index + (steps * shapeProperties.itemCounts.x));
            canMove = selectedShape->index + (steps * shapeProperties.itemCounts.x) < images.size();
            if (canMove) {
                newSelectedShapeIdx = selectedShape->index + (steps * shapeProperties.itemCounts.x);
            } else {
                newSelectedShapeIdx = ((images.size() / shapeProperties.itemCounts.x) * shapeProperties.itemCounts.x) +
                                      selectedShape->index % shapeProperties.itemCounts.x;
                newSelectedShapeIdx = newSelectedShapeIdx >= images.size() ? images.size() - 1 : newSelectedShapeIdx;
                canMove = true;
            }
            break;
        case ImagePickerMove::END:
            preloadToIndex(INT_MAX);
            canMove = selectedShape->index != images.size() - 1;
            newSelectedShapeIdx = images.size() - 1;
            break;
        case ImagePickerMove::HOME:
            canMove = true;
            newSelectedShapeIdx = 0;
            break;
        case ImagePickerMove::LINE:
            preloadToIndex(steps > 0 && shapeProperties.itemCounts.x * steps);
            canMove = steps > 0 && shapeProperties.itemCounts.x * steps < images.size();
            if (canMove) {
                newSelectedShapeIdx = shapeProperties.itemCounts.x * (steps - 1);
            } else {
                canMove = true;
                newSelectedShapeIdx = images.size() - 1;
            }
            break;
        case ImagePickerMove::PG_DOWN:
            preloadToIndex(
                    selectedShape->index + (shapeProperties.itemCounts.x * shapeProperties.itemCounts.y * steps));
            newSelectedShapeIdx =
                    selectedShape->index + (shapeProperties.itemCounts.x * shapeProperties.itemCounts.y * steps);
            newSelectedShapeIdx = newSelectedShapeIdx > images.size() - 1 ? images.size() - 1 : newSelectedShapeIdx;
            canMove = newSelectedShapeIdx != selectedShape->index;
            break;
        case ImagePickerMove::PG_UP:
            newSelectedShapeIdx =
                    selectedShape->index - (shapeProperties.itemCounts.x * shapeProperties.itemCounts.y * steps);
            newSelectedShapeIdx = newSelectedShapeIdx > 0 ? newSelectedShapeIdx : 0;
            canMove = selectedShape->index != newSelectedShapeIdx;
            break;
    }


    if (canMove) {
        goToImage(newSelectedShapeIdx);
    }

    return canMove;
}

Image *ImagePickerDrawer::getSelectedImage() {
    if (selectedShape != nullptr) {
        return selectedShape->image;
    } else {
        return nullptr;
    }
}

void ImagePickerDrawer::setFilter(std::function<bool(Image *)> filter, std::string filterString) {
    this->filter = std::move(filter);
    this->page = 0;
    lastPreloadedImageIndex = 0;
    redrawAllInNextFrame = true;
    this->filterString = std::move(filterString);
    images.clear();
}

void ImagePickerDrawer::clearFilter() {
    this->page = 0;
    lastPreloadedImageIndex = 0;
    this->filterString = "";

    this->filter.reset();
    this->images = std::vector<Image>(allImages.begin(), allImages.end());

    redrawAllInNextFrame = true;
}
