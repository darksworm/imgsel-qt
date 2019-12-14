#include "ImageDrawer.h"
#include "../../util/config/ConfigManager.h"
#include "../../util/exceptions/ImageNotLoadable.h"

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

    QPoint imagePos = QPoint(
            shapeProperties.position.x() + shapeProperties.dimensions.x / 2 - width / 2,
            shapeProperties.position.y() + shapeProperties.dimensions.y / 2 - height / 2
    );

    QPainter paint = QPainter();
    paint.begin(&pixmap);
    paint.drawImage(imagePos.x(), imagePos.y(), img.value());

    QFont font;
    font.setFamily(font.defaultFamily());
    QFontMetrics fm(font);

    QString displayName = QString::fromUtf8(shape.image->getFilenameWithoutExtension().c_str());
    unsigned int textWidth = 0;
    auto maxTextWidth = shapeProperties.dimensions.x - 20;

    do {
#if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
        textWidth = fm.horizontalAdvance(displayName);
#else
        textWidth = fm.width(displayName);
#endif

        if (textWidth >= maxTextWidth) {
            displayName = displayName.left(displayName.length() - 1);
        }
    } while (textWidth >= maxTextWidth);

    paint.setFont(font);
    paint.setPen(Qt::white);
    paint.drawText(
            shapeProperties.position.x() + shapeProperties.dimensions.x / 2 - textWidth / 2,
            shapeProperties.position.y() + shapeProperties.nameRect.y(),
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
            .nameRect = QRect{
                    0,
                    static_cast<short>(config.getMaxImageHeight() + config.getYPadding() * 1.5),
                    0,
                    0
            },
            .position = QPoint()
    };

    return shapeProperties;
}

QPoint ImageDrawer::getNextShapePosition(ShapeProperties shapeProperties, Dimensions windowDimensions,
                                         std::optional<QPoint> lastShapePosition) {
    QPoint newShapePosition;

    int oneRowWidth = shapeProperties.dimensions.x * shapeProperties.itemCounts.x +
                               (shapeProperties.margins.x * shapeProperties.itemCounts.x - 1);
    int oneColumnHeight = shapeProperties.dimensions.y * shapeProperties.itemCounts.y +
                                   (shapeProperties.margins.y * shapeProperties.itemCounts.y - 1);

    int xMargin = ((int)windowDimensions.x - oneRowWidth) / 2;
    int yMargin = ((int)windowDimensions.y - oneColumnHeight) / 2;

    if (!lastShapePosition.has_value()) {
        newShapePosition = QPoint(
                xMargin,
                yMargin
        );
    } else {
        QPoint offset;

        if (lastShapePosition->x() + shapeProperties.margins.x + shapeProperties.dimensions.x > oneRowWidth + xMargin) {
            // move to next line
            offset.setY(shapeProperties.dimensions.y + shapeProperties.margins.y + lastShapePosition->y());
            offset.setX(xMargin);
        } else {
            offset.setX(lastShapePosition->x() + shapeProperties.dimensions.x + shapeProperties.margins.x);
            offset.setY(lastShapePosition->y());
        }

        newShapePosition = QPoint(
                offset.x(),
                offset.y()
        );
    }

    return newShapePosition;
}

void ImageDrawer::drawSelectedShapeIndicator(ShapeProperties shapeProperties, Shape shape) {
    if (shape.selected) {
        auto pos = shape.position;
        QPainter painter;
        painter.begin(&pixmap);
        painter.setPen(Qt::red);
        painter.drawRect(QRect(pos.x(), pos.y(), shapeProperties.dimensions.x, shapeProperties.dimensions.y));
        painter.end();
    }
}

void ImageDrawer::clearSelectedShapeIndicator(ShapeProperties shapeProperties, Shape shape) {
    auto pos = shape.position;
    QPainter painter;
    painter.begin(&pixmap);

    painter.setCompositionMode(QPainter::CompositionMode_Source);

    painter.fillRect(
            pos.x() - selectedShapeLineWidth,
            pos.y() - selectedShapeLineWidth,
            shapeProperties.dimensions.x + (2 * selectedShapeLineWidth),
            selectedShapeLineWidth * 2,
            Qt::transparent);

    painter.fillRect(
            pos.x() - selectedShapeLineWidth,
            pos.y() - selectedShapeLineWidth + shapeProperties.dimensions.y,
            shapeProperties.dimensions.x + (2 * selectedShapeLineWidth),
            selectedShapeLineWidth * 2,
            Qt::transparent);

    painter.fillRect(
            pos.x() - selectedShapeLineWidth,
            pos.y() - selectedShapeLineWidth,
            (2 * selectedShapeLineWidth),
            shapeProperties.dimensions.y + selectedShapeLineWidth * 2,
            Qt::transparent);

    painter.fillRect(
            pos.x() - selectedShapeLineWidth + shapeProperties.dimensions.x,
            pos.y() - selectedShapeLineWidth,
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
            shape.position.x() - 2,
            shape.position.y() - 2,
            shapeProperties.dimensions.x + 4,
            shapeProperties.dimensions.y + 4,
            Qt::transparent);

    painter.end();
}
