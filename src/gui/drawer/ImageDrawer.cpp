#include <Imlib2.h>
#include "ImageDrawer.h"
#include "../../exceptions/ImageNotLoadable.h"
#include "../../config/ConfigManager.h"


Shape ImageDrawer::calcNextShape(ShapeProperties properties, Image *hotkey, bool selected, long index) {
    return Shape();
}

Shape ImageDrawer::drawNextShape(ShapeProperties shapeProperties, Shape shape) {
    auto config = ConfigManager::getOrLoadConfig();
    Imlib_Image img;
    Pixmap pix;
    int width, height;

    img = imlib_load_image(shape.image->getPath().c_str());

    if (!img) {
        throw ImageNotLoadable();
    }

    imlib_context_set_image(img);
    width = imlib_image_get_width();
    height = imlib_image_get_height();

    if (config.getMaxImageHeight() + config.getMaxImageWidth() > 0) {
        if (config.getMaxImageWidth() > 0 && width > config.getMaxImageWidth()) {
            auto scale = (double)config.getMaxImageWidth() / width;
            int new_height = height * scale;
            img = imlib_create_cropped_scaled_image(0, 0, width, height, config.getMaxImageWidth(), new_height);
            imlib_free_image();
            imlib_context_set_image(img);

            width = config.getMaxImageWidth();
            height = new_height;
        }

        if (config.getMaxImageHeight() > 0 && height > config.getMaxImageHeight()) {
            auto scale = (double)config.getMaxImageHeight() / height;
            int new_width = width * scale;
            img = imlib_create_cropped_scaled_image(0, 0, width, height, new_width, config.getMaxImageHeight());
            imlib_free_image();
            imlib_context_set_image(img);

            width = new_width;
            height = config.getMaxImageHeight();
        }
    }

    pix = XCreatePixmap(windowManager->getDisplay(), windowManager->getWindow(), width, height,
                        windowManager->getDepth());

    imlib_context_set_display(windowManager->getDisplay());
    imlib_context_set_visual(windowManager->getVisual());
    imlib_context_set_colormap(windowManager->getColorMap());
    imlib_context_set_drawable(windowManager->getWindow());

    XPoint imagePos = XPoint();
    imagePos.x = shapeProperties.position.x + shapeProperties.dimensions.x / 2 - width / 2;
    imagePos.y = shapeProperties.position.y + shapeProperties.dimensions.y / 2 - height / 2;

    imlib_render_image_on_drawable(imagePos.x, imagePos.y);

    lastShapePosition = shapeProperties.position;

    imlib_free_image();
    XFreePixmap(windowManager->getDisplay(), pix);

    auto font = XLoadQueryFont(windowManager->getDisplay(), "fixed");
    XSetFont(windowManager->getDisplay(), textGC, font->fid);

    auto displayName = shape.image->getFilenameWithoutExtension();
    unsigned int textWidth = 0;
    auto maxTextWidth = shapeProperties.dimensions.x - 20;

    do {
        textWidth = XTextWidth(font, displayName.c_str(), displayName.length());

        if(textWidth >= maxTextWidth) {
            displayName = displayName.substr(0, displayName.length() - 1);
        }
    } while (textWidth >= maxTextWidth);

    XDrawString(
            windowManager->getDisplay(),
            windowManager->getWindow(),
            textGC,
            lastShapePosition.value().x + shapeProperties.dimensions.x / 2 - textWidth / 2,
            lastShapePosition.value().y + shapeProperties.nameRect.y,
            displayName.c_str(),
            (int) displayName.length()
    );

    return shape;
}

ShapeProperties ImageDrawer::calcShapeProps(Window window) {
    auto config = ConfigManager::getOrLoadConfig();

    ShapeProperties shapeProperties{
            .dimensions = Dimensions(config.getMaxImageWidth() + config.getXPadding() * 2,  config.getMaxImageHeight() + config.getYPadding() * 2),
            .margins = Dimensions(config.getXMargin(), config.getYMargin()),
            .itemCounts = Dimensions(config.getCols() > 0 ? config.getCols() : 4,
                                     config.getRows() > 0 ? config.getRows() : 4),
            .nameRect = XRectangle{
                    .x = 0,
                    .y = static_cast<short>(config.getMaxImageHeight() + config.getYPadding() * 1.5),
                    .width = 0,
                    .height = 0
            },
            .position = XPoint()
    };

    return shapeProperties;
}

XPoint ImageDrawer::getNextShapePosition(ShapeProperties shapeProperties, Dimensions windowDimensions) {
    auto lastShapePosition = this->lastShapePosition;
    XPoint newShapePosition;

    unsigned int xMargin, yMargin;

    unsigned int oneRowWidth = shapeProperties.dimensions.x * shapeProperties.itemCounts.x +
                               (shapeProperties.margins.x * shapeProperties.itemCounts.x - 1);
    unsigned int oneColumnHeight = shapeProperties.dimensions.y * shapeProperties.itemCounts.y +
                                   (shapeProperties.margins.y * shapeProperties.itemCounts.y - 1);

    xMargin = (windowDimensions.x - oneRowWidth) / 2;
    yMargin = (windowDimensions.y - oneColumnHeight) / 2;

    if (!lastShapePosition.has_value()) {
        newShapePosition = XPoint{
                .x = (short) xMargin,
                .y = (short) yMargin
        };
    } else {
        XPoint offset;

        if (lastShapePosition->x + shapeProperties.margins.x + shapeProperties.dimensions.x > oneRowWidth + xMargin) {
            // move to next line
            offset.y = (short) (shapeProperties.dimensions.y + shapeProperties.margins.y + lastShapePosition->y);
            offset.x = (short) xMargin;
        } else {
            offset.x = (short) (lastShapePosition->x + shapeProperties.dimensions.x + shapeProperties.margins.x);
            offset.y = (short) (lastShapePosition->y);
        }

        newShapePosition = XPoint{
                .x = (short) (offset.x),
                .y = (short) (offset.y)
        };
    }

    return newShapePosition;
}

void ImageDrawer::drawSelectedShapeIndicator(ShapeProperties shapeProperties, Shape shape) {
    if (shape.selected) {
        auto pos = shape.position;
        XDrawRectangle(windowManager->getDisplay(), windowManager->getWindow(), selectedShapeGC, pos.x, pos.y,
                       shapeProperties.dimensions.x, shapeProperties.dimensions.y);
    }
}

void ImageDrawer::clearSelectedShapeIndicator(ShapeProperties shapeProperties, Shape shape) {
    auto pos = shape.position;

    XClearArea(
            windowManager->getDisplay(),
            windowManager->getWindow(),
            pos.x - selectedShapeLineWidth,
            pos.y - selectedShapeLineWidth,
            shapeProperties.dimensions.x + (2 * selectedShapeLineWidth),
            selectedShapeLineWidth * 2,
            false
    );

    XClearArea(
            windowManager->getDisplay(),
            windowManager->getWindow(),
            pos.x - selectedShapeLineWidth,
            pos.y - selectedShapeLineWidth + shapeProperties.dimensions.y,
            shapeProperties.dimensions.x + (2 * selectedShapeLineWidth),
            selectedShapeLineWidth * 2,
            false
    );

    XClearArea(
            windowManager->getDisplay(),
            windowManager->getWindow(),
            pos.x - selectedShapeLineWidth,
            pos.y - selectedShapeLineWidth,
            (2 * selectedShapeLineWidth),
            shapeProperties.dimensions.y + selectedShapeLineWidth * 2,
            false
    );

    XClearArea(
            windowManager->getDisplay(),
            windowManager->getWindow(),
            pos.x - selectedShapeLineWidth + shapeProperties.dimensions.x,
            pos.y - selectedShapeLineWidth,
            (2 * selectedShapeLineWidth),
            shapeProperties.dimensions.y + selectedShapeLineWidth * 2,
            false
    );
}
