#pragma once

#include "../../util/config/ConfigManager.h"
#include "../util/Shape.h"
#include <climits>
#include <numeric>
#include <optional>
#include <QPainter>

class ShapeDrawer : public QObject {
Q_OBJECT
    friend class ImagePickerDrawer;

protected:
    unsigned int selectedShapeLineWidth = 2;

    QPixmap &pixmap;

    virtual Shape drawNextShape(ShapeProperties shapeProperties, Shape shape) = 0;

    virtual ShapeProperties calcShapeProps() = 0;

    virtual QPoint getNextShapePosition(ShapeProperties shapeProperties, Dimensions windowDimensions,
                                        std::optional<QPoint> lastShapePosition) = 0;

    virtual void drawSelectedShapeIndicator(ShapeProperties shapeProperties, Shape shape) = 0;

    virtual void clearSelectedShapeIndicator(ShapeProperties shapeProperties, Shape shape) = 0;

    virtual void clearShape(ShapeProperties shapeProperties, Shape shape) = 0;

    void clearPixmap() {
        pixmap.fill(getBackgroundColor());
    }

    QColor getBackgroundColor() {
        return QColor(QRgba64::fromRgba(0, 0, 0, 200));
    }

public:
    ShapeDrawer(QPixmap &pixmap) : pixmap(pixmap), QObject() {
    }
};
