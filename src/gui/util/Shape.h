#pragma once

#include "../../util/image.h"
#include "Dimensions.h"

struct ShapeProperties {
    Dimensions dimensions;
    Dimensions margins;
    Dimensions itemCounts;

    QRect nameRect;
    QPoint position;

    int getOneRowWidth() {
        return this->dimensions.x * this->itemCounts.x +
                               (this->margins.x * this->itemCounts.x - 1);
    }

    int getOneColumnHeight() {
        return this->dimensions.y * this->itemCounts.y +
                                   (this->margins.y * this->itemCounts.y - 1);
    }

    int getOneColumnWidth() {
        return this->dimensions.x + this->margins.x;
    }

    int getOneRowHeight() {
        return this->dimensions.y + this->margins.y;
    }

    Dimensions getImagePos() {
        return Dimensions(
            this->position.x() + this->margins.x,
            this->position.y() + this->margins.y
        );
    }

    Dimensions getImageSize() {
    }

   // int getXMargin() {
   //     return ((int)geo.width() - oneRowWidth) / 2;
   // }

   // int getYMargin() {
   //     return ((int)geo.height() - oneColumnHeight) / 2;
   // }
};

struct Shape {
    QPoint position;

    bool selected;
    long index;

    Image *image;
};

