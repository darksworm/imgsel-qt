#pragma once

#include <signal.h>
#include <unistd.h>
#include <initializer_list>

// this'll catch unix exit signals and close the qApp
// gracefully github.com/darksworm/emojigun/issues/78
void catchUnixExitSignals(std::initializer_list<int> quitSignals);
