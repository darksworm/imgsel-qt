Emojigun can currently only be run on linux systems which use the X window system (X11)

Prerequisites:
*  QT5
*  Xlib
*  xclip

## Compiling
```bash
git clone https://github.com/darksworm/emojigun-qt.git
mkdir emojigun-qt/src/build
cd emojigun-qt/src/build
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
