These are emojigun install instructions for linux. If you are looking for the windows version, you should just [download the latest release](https://github.com/darksworm/imgsel/releases/latest/download/emojigun.zip).

## Generic prerequisites
**Emojigun is currently only tested on linux systems which use the X window system (X11)**
*  CMake 3.8+
*  QT5
*  Xlib
*  xclip
*  qt5 x11 extras
*  tk

## Ubuntu prerequisites
```bash 
apt-get install qt5-default libqt5x11extras5 cmake libx11-dev xclip g++ tk-dev tk8.5-dev libqt5x11extras5-dev
```

## Compilation
```bash
git clone https://github.com/darksworm/emojigun.git
mkdir emojigun/src/build
cd emojigun/src/build
cmake ..
make
```

## Installation
See compliation, then
```bash
sudo make install
```
and then emojigun should be available in your shell. Launch it with
```bash
emojigun
```

see usage instructions: [USAGE.md](USAGE.md)
