#include "Application.h"

Application::Application(int &argc, char *argv[]) : QApplication(argc, argv, true) {
    _singular = new QSharedMemory("IMGSEL", this);
}

Application::~Application() {
    if (_singular->isAttached()) {
        _singular->detach();
    }
}

bool Application::lock() {
    if (_singular->attach(QSharedMemory::ReadOnly)) {
        _singular->detach();
        return false;
    }

    return _singular->create(1);
}