#pragma once

#include <QPainter>
#include <QtGui/QImage>
#include <string>
#include "ShapeDrawer.h"

class ImageDrawer : public ShapeDrawer {
protected:
    Shape drawNextShape(ShapeProperties shapeProperties, Shape shape) override;

    ShapeProperties calcShapeProps() override;

    QPoint getNextShapePosition(ShapeProperties shapeProperties, Dimensions windowDimensions,
                                std::optional<QPoint> lastShapePosition) override;

    void drawSelectedShapeIndicator(ShapeProperties shapeProperties, Shape shape) override;

    void clearSelectedShapeIndicator(ShapeProperties shapeProperties, Shape shape) override;

    void clearShape(ShapeProperties shapeProperties, Shape shape) override;

public:
    ImageDrawer(QPixmap &pixmap) : ShapeDrawer(pixmap) {};
};



