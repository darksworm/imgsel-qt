Imgsel can currently only be run on linux systems which use the X window system (X11)

Prerequisites:
*  QT5
*  Xlib
*  xclip

## Compiling
```bash
git clone https://github.com/darksworm/imgsel-qt.git
mkdir imgsel-qt/src/build
cd imgsel-qt/src/build
cmake ..
make
```

## Installing
See compiling, then
```bash
sudo make install
```
and then imgsel should be available in your shell.
