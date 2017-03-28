Anti-Grain Evolution [![Build Status](https://travis-ci.org/tyoma/agge.svg?branch=master)](https://travis-ci.org/tyoma/agge)
====

This project is based on ideas found in Maxim (mcseem) Shemanarev's Anti-Grain Geometry library, but oriented towards maximizing performance and stability. This engine sits on top of vector rasterizer and renderer rewritten from scratch in test-first style.

Here is how a simple drawing code looks like in agge:

```c++
using namespace agge;

...

bitmap<pixel32, platform::raw_bitmap> surface(150, 100);
rasterizer<unclipped> ras;
renderer ren;

add_path(ras, circle(10.0f /*x*/, 20.0f /*y*/, 5.0f /*radius*/).iterate());

ras.sort();

ren(surface, 0 /*no windowing*/, ras /*mask*/, solid_color_blender(0, 128, 255), winding());

// ... and, if on Windows:
surface.blit(hdc, 0, 0, 150, 100);
```
