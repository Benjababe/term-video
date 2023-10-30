# Terminal Vidya Player

A terminal application to play videos with an ASCII output.

[Coloured line printing sample](https://www.youtube.com/watch?v=H09VKJ3H3Lk)

[Coloured buffer printing sample](https://www.youtube.com/watch?v=KUX2ZvlXs1w)

## Prerequisites
Depending on the build for video processing, either opencv4 or FFmpeg + libao is required. Refer to the top of  `CMakeLists.txt` for the build type.

## Building The Project
From project root, run
```
cmake -S "." -B "./out/build";
cmake --build "./out/build" --config Debug;
```
and the executable should be in `out/build` or `out/build/Debug/`

## Usage

`term-video --file <filepath>`

| Argument | Details |
| -------- | ------- |
| `-f`, `--file` | Relative path of the file from your current working directory. |
| `-fa`, `--force-aspect` | Flag whether to use the source video's aspect ratio in playback. |
| `-s`, `--skip-frames` | Number of frames to skip for every 1 frame. |
| `-b`, `--buffer` | Write directly to the console buffer instead of conventional printing. |
| `-c`, `--color`, `--colour` | To use colour output in playback. |
| `-ct`, `--color-threshold`, `--colour-threshold` | In ANSI RGB printing, the absolute difference in colour before using a new ANSI code. Refer to `src/optimiser.cpp` |
| `-al`, `--audio-language` | Choose a preferred audio language, expects 3 letter [ISO 639-2](https://en.wikipedia.org/wiki/List_of_ISO_639-2_codes) codes. |
| `-alumi`, `--avg-lumi` | Use average of RGB values instead of relative luminance for luminance. Refer to `src/colour.cpp`. |


## Optimisation Settings

For coloured line printing, ANSI colour codes are used to switch the foreground text colours which may cause long print times if many different colours are used in 1 frame which leads to slowdown. For optimisation, 2 arguments can be used: 
- `-s | --skip-frames` 
- `-t | --threshold`.

`--skip-frames` will skip the number of frames provided for every frame rendered. So if you pass `--skip-frames 2`, it will skip 2 frames for every frame drawn, cutting the framerate by 1/3.

`--threshold` will include a threshold on how different a colour must be with the previous colour used to use an ANSI code. If you pass `--threshold 4`, and the previous colour was RGB(120, 200, 255), the next pixel will only use an ANSI colour code if it's outside of RGB([116,124], [196,204], [251,255]). 

## Coloured Buffer Printing Limitation
With Coloured buffer printing, as far as I know, each pixel will be limited to only 16 colours specified [here for Windows](https://learn.microsoft.com/en-us/windows/console/char-info-str).

## To Dos

- [x] Read video and output frames as ASCII
- [x] Support colour
- [x] Force video aspect ratio
- [x] Write output to buffer
- [x] Audio playback
- [x] Handle resolution changes
- [ ] Audio track selection
- [ ] Subtitle selection
- [ ] Video seeking
- [ ] Proper linux buffer output colours
- [ ] More character support