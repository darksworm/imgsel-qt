#pragma once

#include "../image/image.h"
#include "dimensions.h"

struct ShapeProperties {
    Dimensions dimensions;
    Dimensions margins;
    Dimensions itemCounts;

    QRect nameRect;
    QPoint position;
};

struct Shape {
    QPoint position;

    bool selected;
    long index;

    Image *image;
};

