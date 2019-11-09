#include <utility>

#include "ImagePickerDrawer.h"
#include "../exceptions/OutOfBounds.h"
#include "../exceptions/ImageNotLoadable.h"
#include "ImagePickerMove.h"
#include <memory>

ImagePickerDrawer::ImagePickerDrawer(WindowManager *windowManager, std::vector<Image> *images) {
    this->windowManager = windowManager;
    this->page = 0;
    this->selectedShape = nullptr;
    this->allImages = images;
    this->images = new std::vector<Image>(images->begin(), images->end());

    shapeDrawer = ShapeDrawerFactory::getShapeDrawer(ShapeType::IMAGE, windowManager);
    shapeProperties = shapeDrawer->calcShapeProps(windowManager->getWindow());
}

void ImagePickerDrawer::drawFrame(Image *selectedImage, bool redrawAll) {
    auto start = getPageImageStart();

    auto oldShapes = std::map<long, Shape>();
    oldShapes.insert(shapes.begin(), shapes.end());

    shapes.clear();
    shapeDrawer->lastShapePosition.reset();

    std::unique_ptr<Dimensions> windowDimensions(new Dimensions);
    windowManager->getWindowDimensions(&windowDimensions->x, &windowDimensions->y);

    unsigned int shapeCnt = shapeProperties.itemCounts.x * shapeProperties.itemCounts.y;
    int drawnShapeCnt = 0;

    auto it = start;
    redrawAll = redrawAll || redrawAllInNextFrame;

    for (; it != images->end(); ++it) {
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
                .index = std::distance(images->begin(), it),
                .image = &*it
        };

        bool shouldDrawShape = true;

        try {
            auto oldShape = oldShapes.at(drawnShapeCnt);

            if (!redrawAll && oldShape.image->getPath() == shape.image->getPath()) {
                shouldDrawShape = false;
                shape = oldShape;
                shapeDrawer->lastShapePosition = oldShape.position;

                if (oldShape.selected && !selected) {
                    shapeDrawer->clearSelectedShapeIndicator(shapeProperties, oldShape);
                }
            } else {
                if (!this->shapes.empty()) {
                    shapeDrawer->lastShapePosition = (--this->shapes.end())->second.position;
                }
                // TODO: these parameters are wack
                XClearArea(windowManager->getDisplay(), windowManager->getWindow(), oldShape.position.x - 2,
                           oldShape.position.y - 2, shapeProperties.dimensions.x + 4, shapeProperties.dimensions.y + 4,
                           false);
            }
        } catch (std::out_of_range &e) {
            // nothing to do here
        }

        shape.selected = selected;

        if (shouldDrawShape) {
            shapeProperties.position = shapeDrawer->getNextShapePosition(shapeProperties, *windowDimensions);
            shape.position = shapeProperties.position;

            try {
                shapeDrawer->drawNextShape(shapeProperties, shape);
            } catch (ImageNotLoadable &e) {
                images->erase(it);
                if (--it < start) {
                    break;
                } else {
                    continue;
                }
            }
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
            XClearArea(windowManager->getDisplay(), windowManager->getWindow(), oldShape.position.x - 2,
                       oldShape.position.y - 2, shapeProperties.dimensions.x + 4, shapeProperties.dimensions.y + 4,
                       false);
        } while (++i < oldShapes.size());
    }

    redrawAllInNextFrame = false;
}

std::vector<Image>::iterator ImagePickerDrawer::getPageImageStart() {
    int hotkeysPerPage = shapeProperties.itemCounts.y * shapeProperties.itemCounts.x;

    preloadToIndex(hotkeysPerPage - 1);

    if (page > 0 && images->size() < hotkeysPerPage) {
        throw OutOfBounds();
    }

    int offset = hotkeysPerPage * page;

    return images->begin() + offset;
}

void ImagePickerDrawer::preloadToIndex(unsigned int targetIndex) {
    if (filter.has_value() && images->size() < (long) targetIndex + 1) {
        //spdlog::debug("Loading page...");
        unsigned int offset = 0;

        if (lastPreloadedImageIndex) {
            if (allImages->begin() + lastPreloadedImageIndex + 1 == allImages->end()) {
                return;
            } else {
                offset = lastPreloadedImageIndex + 1;
            }
        }

        unsigned int hotkeysPerPage = shapeProperties.itemCounts.y * shapeProperties.itemCounts.x;
        unsigned int targetImageCount = ((targetIndex / hotkeysPerPage) + 1) * hotkeysPerPage;

        for (auto it = allImages->begin() + offset; it != allImages->end(); ++it) {
            if (filter.operator*()(&*it)) {
                images->push_back(*it);
                lastPreloadedImageIndex = std::distance(allImages->begin(), it);

                if (images->size() >= targetImageCount) {
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
    Image *image = &*(images->begin() + hotkeyIdx);

    page = getImagePage(hotkeyIdx);
    drawFrame(image);
}

bool ImagePickerDrawer::move(ImagePickerMove move, unsigned int steps) {
    bool canMove = false;
    long newSelectedShapeIdx = 0;

    char *debug = "NONE";

    switch (move) {
        case ImagePickerMove::LEFT:
            canMove = selectedShape->index >= steps;
            newSelectedShapeIdx = selectedShape->index - steps;
            debug = "LEFT";
            break;
        case ImagePickerMove::RIGHT:
            preloadToIndex(selectedShape->index + steps);
            canMove = selectedShape->index + steps < images->size();
            newSelectedShapeIdx = selectedShape->index + steps;
            debug = "RIGHT";
            break;
        case ImagePickerMove::UP:
            canMove = selectedShape->index - (steps * shapeProperties.itemCounts.x) >= 0;
            if (canMove) {
                newSelectedShapeIdx = selectedShape->index - (steps * shapeProperties.itemCounts.x);
            } else {
                newSelectedShapeIdx = selectedShape->index % shapeProperties.itemCounts.x;
                canMove = true;
            }
            debug = "UP";
            break;
        case ImagePickerMove::DOWN:
            preloadToIndex(selectedShape->index + (steps * shapeProperties.itemCounts.x));
            canMove = selectedShape->index + (steps * shapeProperties.itemCounts.x) < images->size();
            if (canMove) {
                newSelectedShapeIdx = selectedShape->index + (steps * shapeProperties.itemCounts.x);
            } else {
                newSelectedShapeIdx = ((images->size() / shapeProperties.itemCounts.x) * shapeProperties.itemCounts.x) +
                                      selectedShape->index % shapeProperties.itemCounts.x;
                newSelectedShapeIdx = newSelectedShapeIdx >= images->size() ? images->size() - 1 : newSelectedShapeIdx;
                canMove = true;
            }
            debug = "DOWN";
            break;
        case ImagePickerMove::END:
            preloadToIndex(INT_MAX);
            canMove = selectedShape->index != images->size() - 1;
            newSelectedShapeIdx = images->size() - 1;
            debug = "END";
            break;
        case ImagePickerMove::HOME:
            canMove = true;
            newSelectedShapeIdx = 0;
            debug = "HOME";
            break;
        case ImagePickerMove::LINE:
            preloadToIndex(steps > 0 && shapeProperties.itemCounts.x * steps);
            canMove = steps > 0 && shapeProperties.itemCounts.x * steps < images->size();
            if (canMove) {
                newSelectedShapeIdx = shapeProperties.itemCounts.x * (steps - 1);
            } else {
                canMove = true;
                newSelectedShapeIdx = images->size() - 1;
            }
            debug = "LINE";
            break;
        case ImagePickerMove::PG_DOWN:
            preloadToIndex(
                    selectedShape->index + (shapeProperties.itemCounts.x * shapeProperties.itemCounts.y * steps));
            newSelectedShapeIdx =
                    selectedShape->index + (shapeProperties.itemCounts.x * shapeProperties.itemCounts.y * steps);
            newSelectedShapeIdx = newSelectedShapeIdx > images->size() - 1 ? images->size() - 1 : newSelectedShapeIdx;
            canMove = newSelectedShapeIdx != selectedShape->index;
            debug = "PGDOWN";
            break;
        case ImagePickerMove::PG_UP:
            newSelectedShapeIdx =
                    selectedShape->index - (shapeProperties.itemCounts.x * shapeProperties.itemCounts.y * steps);
            newSelectedShapeIdx = newSelectedShapeIdx > 0 ? newSelectedShapeIdx : 0;
            canMove = selectedShape->index != newSelectedShapeIdx;
            debug = "PGUP";
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
    this->filterString = filterString;
    images->clear();
}

void ImagePickerDrawer::clearFilter() {
    this->page = 0;
    lastPreloadedImageIndex = 0;
    this->filterString = "";

    this->filter.reset();
    delete (this->images);
    this->images = new std::vector<Image>(allImages->begin(), allImages->end());

    redrawAllInNextFrame = true;
}
