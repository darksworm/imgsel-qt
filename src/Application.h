#pragma once


#include <QtWidgets/QApplication>
#include <QtCore/QSharedMemory>

class Application : public QApplication {

public:
    Application(int &argc, char **argv);
    ~Application() override;
    bool lock();

private:
    QSharedMemory *_singular;
};



