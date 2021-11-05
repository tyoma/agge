Anti-Grain Evolution [![Build/Test](https://github.com/tyoma/agge/actions/workflows/cmake.yml/badge.svg?branch=master)](https://github.com/tyoma/agge/actions/workflows/cmake.yml)
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

Note: to build for Android vs-android is required. This is planned to change.
