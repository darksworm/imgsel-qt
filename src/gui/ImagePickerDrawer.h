#pragma once

#include <QtGui/QPixmap>

#include <functional>
#include <memory>
#include <map>

#include "../image/image.h"
#include "Shape.h"
#include "drawer/ShapeDrawer.h"
#include "ImagePickerMove.h"

class ImagePickerDrawer {
private:
    WindowManager *windowManager;

    ShapeProperties shapeProperties;
    ShapeDrawer* shapeDrawer;

    Shape *selectedShape;
    int page = 0;

    std::vector<Image> images;
    std::vector<Image> allImages;
    std::map<long,Shape> shapes;

    std::vector<Image>::iterator getPageImageStart();

    int getImagePage(long index);

    void goToImage(long hotkeyIdx);

    std::optional<std::function<bool(Image*)>> filter;

    std::string filterString = "";

    unsigned int lastPreloadedImageIndex = 0;

    bool redrawAllInNextFrame = false;

    QPixmap &pixmap;

public:
    ImagePickerDrawer(QPixmap& pixmap);

    void drawFrame(Image* selectedImage, bool redrawAll = false);

    bool move(ImagePickerMove move, unsigned int steps = 1);

    void setFilter(std::function<bool(Image *)> filter, std::string filterString);

    void clearFilter();

    Image* getSelectedImage();

    std::string getFilterString() {
        return filterString;
    };

    void preloadToIndex(unsigned int targetIndex);
};


