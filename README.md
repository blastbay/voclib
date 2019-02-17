# Introduction
This is an implementation of a traditional channel vocoder which is generally used to add interesting effects to speech.


The vocoder is written in C and is distributed as a single header file. It should compile on pretty much any platform with a C89 compiler as long as floats are available. If you find a platform on which it does not work, please let me know.


This is not the fastest implementation around but it should still be fast enough for real-time use.

# Features
voclib has the following notable features:
* Easy to integrate (the library is stored in a single file).
* Runs in both stereo and mono.
* Configure the number of bands and the number of filters used per band.
* Configure how quickly the output follows changes in the voice.
* Shift the formants of the voice.
* High quality output.
* Portable ANSI C89 code with no external dependencies.
* No licensing restrictions (public domain or MIT licensed with no attribution requirements).

# Description
A vocoder takes two input signals and produces a third signal as its output. The first signal, called the carrier, is usually an instrument or similar (white noise works well too). The carrier should generally have a lot of harmonics. The second input signal, called the modulator, is most often comprised of human speech. The vocoder superimposes the characteristics of the speech upon the instrument, so that the result sounds as if the instrument is singing.


The vocoder performs approximately the following steps:
* Divide the carrier and the modulator into frequency bands, the sum of which cover the entire frequency spectrum.
* Smooth each modulator band with an envelope follower.
* Multiply the resulting envelope for each band with the corresponding band of the carrier.


There are a bunch of settings that can be tweaked to generate different types of effects; see the public API in voclib.h for more details. If you are adventurous, you can change the settings in the middle of the audio to generate transitions as well.

# Command Line Shell
In addition to the library itself, there is a shell program which takes two wave files as input and produces a third as its output. The shell depends on dr_wav from https://github.com/mackron/dr_libs - a copy of which is included in this repository as well.


To build the shell, simply compile vocshell.c and make sure that both dr_wav.h and voclib.h are on your include path.

# License
This software is available under 2 licenses -- choose whichever you prefer.

## ALTERNATIVE A - MIT No Attribution License
Copyright (c) 2019 Philip Bennefall

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so.


THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.


In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
