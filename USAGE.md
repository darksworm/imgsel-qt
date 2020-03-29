## CLI parameters:
```
EMOJIGUN - EMOJI sharing tool.
Usage: ./emojigun [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  --files TEXT:FILE ... REQUIRED
                              List of images to display.
  --vim                       Set the initial mode to VIM mode.
  --rows TEXT REQUIRED Needs: --cols
                              How many rows to display
  --cols TEXT Needs: --rows   How many cols to display
  --max-width TEXT Needs: --max-height
                              The max image width. Any images larger than this will be scaled to this width
  --max-height TEXT REQUIRED Needs: --max-width
                              The max image height. Any images larger than this will be scaled to this height
  --x-padding TEXT Needs: --y-padding
                              Padding between image and selection box in pixels on the x axis
  --y-padding TEXT Needs: --x-padding
                              Padding between image and selection box in pixels on the y axis
  --x-margin TEXT Needs: --y-margin
                              Margin between images in pixels on the x axis
  --y-margin TEXT Needs: --x-margin
                              Margin between images in pixels on the y axis
  --window-width TEXT Needs: --window-height
                              Window width
  --window-height TEXT Needs: --window-width
                              Window height
  --print-path                Write file path to stdout instead of copying it's contents to the clipboard.
```

## Examples:
#### Basic 6x10 fullscreen (params are good for 1920x1080)
```bash
emojigun --files ~/.emoji/*.png --rows 6 --cols 10 --max-width 64 --max-height 64 --files ~/.temoji/*.png
```

#### Stitch two emojis together (left-to-right) (requires imagemagcik)
```bash
alias emojigun-default='emojigun --rows 6 --cols 10 --max-width 64 --max-height 64 --files ~/.temoji/*.png --print-path'
convert -gravity center -background none $(emojigun-default) $(emojigun-default) +append /tmp/last-merged-emoji.png
cat /tmp/last-merged-emoji.png | xclip -selection clipboard -target image/png -i
```
