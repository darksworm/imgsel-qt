#pragma once

#ifdef WIN32
#include <quazip.h>
#include <quazipfile.h>
#else
#include <quazip5/quazip.h>
#include <quazip5/quazipfile.h>
#endif

class Unzipper {
public:
    bool unzipAllFilesToPath(QString outputPath);
    Unzipper(QString archivePath);
    ~Unzipper();
private:
    QuaZip* archive;
};
