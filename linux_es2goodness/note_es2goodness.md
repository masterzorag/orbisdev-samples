### requirements
clang, make
on linux we use shared libraries, so: runtime and devel packages for libao, libEGL, libGLESv2, libSDL
note: same code on ps4 instead links against static libraries

### build something
each makefile build a different sample, actually:
_egl_demo_text_ani, egl_demo_lines_and_rects, egl_demo_tetris, egl_demo_opentyrian_

### exporting envs
```sh
# set path to orbisdev toolchain:
export ORBISDEV=/Archive/PS4-work/orbisdev/orbisdev
# not needed on linux, just for reference:
# export PATH=$ORBISDEV/bin:$PATH
# latest mesa issue, so set your default linux EGL_PLATFORM:
export EGL_PLATFORM=x11
```

### testing
```sh
# build:
make -f newone.makefile
# exec:
./egl_demo_text_ani
```