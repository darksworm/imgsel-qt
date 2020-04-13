Emojigun can currently only be run on linux systems which use the X window system (X11)

Prerequisites:
*  cmake 3.8+
*  QT5
*  Xlib
*  xclip
*  qt5 x11 extras
*  tk

Prerequisite package list for ubuntu: `qt5-default libqt5x11extras5 cmake libx11-dev xclip g++ tk-dev tk8.5-dev libqt5x11extras5-dev`

## Compiling
```bash
git clone https://github.com/darksworm/emojigun.git
mkdir emojigun/src/build
cd emojigun/src/build
cmake ..
make
```

## Installing
See compiling, then
```bash
sudo make install
```
and then emojigun should be available in your shell.

see usage instructions: [USAGE.md](USAGE.md)
