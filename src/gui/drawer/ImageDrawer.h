#pragma once

#include <QPainter>
#include <QtGui/QImage>
#include <string>
#include "ShapeDrawer.h"

class ImageDrawer : public ShapeDrawer {
protected:
    Shape drawNextShape(ShapeProperties shapeProperties, Shape shape) override;

    ShapeProperties calcShapeProps() override;

    XPoint getNextShapePosition(ShapeProperties shapeProperties, Dimensions windowDimensions) override;

    void drawSelectedShapeIndicator(ShapeProperties shapeProperties, Shape shape) override;

    void clearSelectedShapeIndicator(ShapeProperties shapeProperties, Shape shape) override;

public:
    ImageDrawer(QPixmap &pixmap) : ShapeDrawer(pixmap) {};
};



