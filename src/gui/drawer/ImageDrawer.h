#pragma once

#include <QPainter>
#include <QtConcurrent>
#include <QtGui/QImage>
#include <string>
#include "ShapeDrawer.h"

class ImageDrawer : public ShapeDrawer {
Q_OBJECT

private:
    std::map<std::string, std::optional<QImage>> imageCache;

protected:
    Shape drawNextShape(ShapeProperties shapeProperties, Shape shape) override;

    ShapeProperties calcShapeProps() override;

    QPoint getNextShapePosition(ShapeProperties shapeProperties, Dimensions windowDimensions,
                                std::optional<QPoint> lastShapePosition) override;

    void drawSelectedShapeIndicator(ShapeProperties shapeProperties, Shape shape) override;

    void clearSelectedShapeIndicator(ShapeProperties shapeProperties, Shape shape) override;

    void clearShape(ShapeProperties shapeProperties, Shape shape) override;

public:
    void cacheImages(std::vector<Image> images);
    void clearCache();

    ImageDrawer(QPixmap &pixmap) : ShapeDrawer(pixmap) {};
};



