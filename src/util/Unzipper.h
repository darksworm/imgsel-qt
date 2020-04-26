#pragma once

#include <quazip.h>
#include <quazipfile.h>

class Unzipper {
public:
    bool unzipAllFilesToPath(QString outputPath);
    Unzipper(QString archivePath);
    ~Unzipper();
private:

    QString archivePath;
    QuaZip* archive;
};
