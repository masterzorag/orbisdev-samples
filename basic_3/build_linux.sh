# this builds opentyrian for host linux from stock, using SDL2

clang source/opentyrian/*.c -Iinclude -I/usr/include/SDL2 -DUSE_SDL2 -DTARGET_UNIX -DTYRIAN_DIR='"./pkg/media"' -DHG_REV='"24df4a4651f7+ sdl2"' -D_REENTRANT  -DNDEBUG -std=iso9899:1999 -pedantic -MMD -Wall -Wno-missing-field-initializers -lSDL2 -lm -ggdb -o opentyrian_SDL

# run
# ./opentyrian_SDL -t pkg/media
