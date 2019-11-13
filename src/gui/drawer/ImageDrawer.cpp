#include "ImageDrawer.h"
#include "../../exceptions/ImageNotLoadable.h"
#include "../../config/ConfigManager.h"

std::optional<QImage> loadImage(std::string path) {
    std::optional<QImage> result;
    result.reset();

    auto config = ConfigManager::getOrLoadConfig();

    QImage image(path.c_str());

    if (image.isNull()) {
        return result;
    }

    auto width = image.width();
    auto height = image.height();

    if (config.getMaxImageHeight() + config.getMaxImageWidth() > 0) {
        if (config.getMaxImageWidth() > 0 && width > config.getMaxImageWidth()) {
            auto scale = (double) config.getMaxImageWidth() / width;
            int new_height = height * scale;
            image = image.scaledToHeight(new_height);
        }

        if (config.getMaxImageHeight() > 0 && height > config.getMaxImageHeight()) {
            auto scale = (double) config.getMaxImageHeight() / height;
            int new_width = width * scale;
            image = image.scaledToWidth(new_width);
        }
    }

    result = image;
    return result;
}

Shape ImageDrawer::drawNextShape(ShapeProperties shapeProperties, Shape shape) {
    auto config = ConfigManager::getOrLoadConfig();

    auto img = loadImage(shape.image->getPath());

    if (!img.has_value()) {
        throw ImageNotLoadable();
    }

    int width = img.value().width(),
            height = img.value().height();

    XPoint imagePos = XPoint();
    imagePos.x = shapeProperties.position.x + shapeProperties.dimensions.x / 2 - width / 2;
    imagePos.y = shapeProperties.position.y + shapeProperties.dimensions.y / 2 - height / 2;

    QPainter paint = QPainter();
    paint.begin(&pixmap);
    paint.drawImage(imagePos.x, imagePos.y, img.value());

    QFont font;
    font.setFamily(font.defaultFamily());
    QFontMetrics fm(font);

    QString displayName = QString::fromUtf8(shape.image->getFilenameWithoutExtension().c_str());
    unsigned int textWidth = 0;
    auto maxTextWidth = shapeProperties.dimensions.x - 20;

    do {
        textWidth = fm.horizontalAdvance(displayName);

        if(textWidth >= maxTextWidth) {
            displayName = displayName.left(displayName.length() - 1);
        }
    } while (textWidth >= maxTextWidth);

    paint.setFont(font);
    paint.setPen(Qt::white);
    paint.drawText(
            shapeProperties.position.x + shapeProperties.dimensions.x / 2 - textWidth / 2,
            shapeProperties.position.y + shapeProperties.nameRect.y,
            displayName
    );

    return shape;
}

ShapeProperties ImageDrawer::calcShapeProps() {
    auto config = ConfigManager::getOrLoadConfig();

    ShapeProperties shapeProperties{
            .dimensions = Dimensions(config.getMaxImageWidth() + config.getXPadding() * 2,
                                     config.getMaxImageHeight() + config.getYPadding() * 2),
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

XPoint ImageDrawer::getNextShapePosition(ShapeProperties shapeProperties, Dimensions windowDimensions,
                                         std::optional<XPoint> lastShapePosition) {
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
        QPainter painter;
        painter.begin(&pixmap);
        painter.setPen(Qt::red);
        painter.drawRect(QRect(pos.x, pos.y, shapeProperties.dimensions.x, shapeProperties.dimensions.y));
        painter.end();
    }
}

void ImageDrawer::clearSelectedShapeIndicator(ShapeProperties shapeProperties, Shape shape) {
    auto pos = shape.position;
    QPainter painter;
    painter.begin(&pixmap);

    painter.setCompositionMode(QPainter::CompositionMode_Source);

    painter.fillRect(
            pos.x - selectedShapeLineWidth,
            pos.y - selectedShapeLineWidth,
            shapeProperties.dimensions.x + (2 * selectedShapeLineWidth),
            selectedShapeLineWidth * 2,
            Qt::transparent);

    painter.fillRect(
            pos.x - selectedShapeLineWidth,
            pos.y - selectedShapeLineWidth + shapeProperties.dimensions.y,
            shapeProperties.dimensions.x + (2 * selectedShapeLineWidth),
            selectedShapeLineWidth * 2,
            Qt::transparent);

    painter.fillRect(
            pos.x - selectedShapeLineWidth,
            pos.y - selectedShapeLineWidth,
            (2 * selectedShapeLineWidth),
            shapeProperties.dimensions.y + selectedShapeLineWidth * 2,
            Qt::transparent);

    painter.fillRect(
            pos.x - selectedShapeLineWidth + shapeProperties.dimensions.x,
            pos.y - selectedShapeLineWidth,
            (2 * selectedShapeLineWidth),
            shapeProperties.dimensions.y + selectedShapeLineWidth * 2,
            Qt::transparent);

    painter.end();
}

void ImageDrawer::clearShape(ShapeProperties shapeProperties, Shape shape) {
    // TODO: these parameters are wack
    QPainter painter;
    painter.begin(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    painter.fillRect(
            shape.position.x - 2,
            shape.position.y - 2,
            shapeProperties.dimensions.x + 4,
            shapeProperties.dimensions.y + 4,
            Qt::transparent);

    painter.end();
}
