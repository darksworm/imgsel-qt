#pragma once
#include "../lib/CLI11.hpp"
#include "../StringTools.h"

struct DirectoriesContainImages : public CLI::Validator {
    DirectoriesContainImages() {
        this->name("DirectoriesContainImages");

        this->func_ = [](const std::string &str) {
            auto paths = StringTools::split(str, " ");

            QStringList allowedExtensions;
            for(const auto& ext : Config::getImageExtensions()) {
                allowedExtensions << "*." + QString::fromStdString(ext);
            }

            for (auto &path : paths) {
                QDir dir(QString::fromStdString(path));

                if(dir.exists()) {
                    if(dir.isEmpty()) {
                        return "Passed directory " + path + " is empty!";
                    }

                    QStringList images = dir.entryList(allowedExtensions);

                    if(images.isEmpty()) {
                        return "Passed directory " + path + " contains no images!";
                    }
                }
            }

            return std::string("");
        };
    }
};
