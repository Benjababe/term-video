# C++ Video to ASCII

A CLI application to convert videos into an ASCII output on your terminal.

Currently has the following options:
- Non-coloured line printing
- Non-coloured buffer printing
- Coloured line printing (with optimisation settings)
- Coloured buffer printing

[Coloured line printing sample](https://www.youtube.com/watch?v=H09VKJ3H3Lk)

[Coloured buffer printing sample](https://www.youtube.com/watch?v=KUX2ZvlXs1w)

## Prerequisites
CMake version >= 3.5 is required as it's what I have installed. Versions below are untested, feel free to edit CMakeLists.txt

For libraries, OpenCV is required. 4.6 is what I have installed but older versions probably work with this project.

## Building The Project
From project root, run
```
cmake -S "." -B "./out/build";
cmake --build "./out/build" --config Debug;
```
and the executable should be in `out/build` or `out/build/Debug/`

## Coloured Printing Optimisation
For coloured line printing, ANSI colour codes are used to switch the foreground text colours which may cause long print times if many different colours are used in 1 frame which leads to flickering. For optimisation, 2 arguments can be used: 
- `-s | --skip-frames` 
- `-t | --threshold`.

`--skip-frames` will skip the number of frames provided for every frame rendered. So if you pass `--skip-frames 3`, it will skip 3 frames for every frame drawn, cutting the framerate by 1/4.

`--threshold` will include a threshold on how different a colour must be with the previous colour used to use an ANSI code. If you pass `--threshold 4`, and the previous colour was RGB(120, 200, 255), the next pixel will only use an ANSI colour code if it's outside of RGB([116,124], [196,204], [251,255]). 

## Coloured Buffer Printing Limitation
With Coloured buffer printing, as far as I know, each pixel will be limited to only 16 colours specified [here for Windows](https://learn.microsoft.com/en-us/windows/console/char-info-str) and I'm still planning on using ncurses for Linux, just haven't gotten to it yet.
