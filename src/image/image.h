#pragma once

#include <string>
#include <Imlib2.h>

class Image {
private:
    std::string path = "";
    std::string filename = "";
    std::string filenameWithoutExtension = "";
    std::string extension = "";
public:
    Image(std::string path) : path(std::move(path)) {
        auto lastDotPos = this->path.find_last_of('.');

        auto pathLen = this->path.length();

        if (lastDotPos != std::string::npos && lastDotPos + 1 < pathLen) {
            extension = this->path.substr(lastDotPos + 1);
        } else {
            extension = "";
        }

        auto lastSlashPos = this->path.find_last_of('/');

        if (lastSlashPos != std::string::npos && lastSlashPos + 1 < pathLen) {
            filename = this->path.substr(lastSlashPos + 1);
        } else {
            filename = this->path;
        }

        lastDotPos = filename.find_last_of('.');

        if (lastDotPos != std::string::npos) {
            filenameWithoutExtension = filename.substr(0, lastDotPos);
        }
    }

    std::string getPath() { return path; }

    std::string getFilename() {
        return filename;
    }

    std::string getExtension() {
        return extension;
    }

    std::string getFilenameWithoutExtension() {
        return filenameWithoutExtension;
    }

    bool operator==(Image &other) {
        return other.path == this->path;
    }
};