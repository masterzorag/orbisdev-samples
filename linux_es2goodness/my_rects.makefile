# make -f my_rects.makefile
# make -f my_rects.makefile clean
# make -f my_rects.makefile && ./egl_demo_my_rects


# orbisdev
# export ORBISDEV=/Archive/PS4-work/orbisdev/orbisdev
# export PATH=$ORBISDEV/bin:$PATH

INC  = -I$(ORBISDEV)/usr/include/orbis/freetype-gl \
       -I$(ORBISDEV)/usr/include/orbis/freetype \
       -Iinc

EXE := egl_demo_my_rects

SRC_DIR  := $(ORBISDEV)/../orbisdev-portlibs/freetype-gl/source
#SRC_DIR  += /home/user/Documents/orbisdev-portlibs/MiniAPI/source
#SRC_DIR := /home/user/Documents/orbisdev-portlibs/freetype-gl/source
SRC      := egl.c fileIO.c shader_common.c ls_dir.c demo-font.c
SRC      += GLES2_rects.c png.c GLES2_textures.c GLES2_ani.c GLES2_scene.c GLES2_menu.c
SRC      += pixelshader.c
SRC      += jsmn.c json_simple.c
#			/home/user/Documents/orbisdev-liborbis/source/liborbisAudio/orbisAudio.c
SRC      += $(wildcard $(SRC_DIR)/*.c)

OBJ_DIR  := obj
OBJ      := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CC       := clang
CPPFLAGS := 
CFLAGS   := -Wall -O3 -DMY_RECT -DPNG_ICONS -DFT_DEMO -DES_UI -ggdb

# where libs are?
#LDFLAGS  :=	-L/home/user/Downloads/liborbis/portlibs/MiniAPI/lib -lMiniAPI
#LDFLAGS := -v

LDLIBS   := -lX11 -lEGL -lGLESv2 -lm -lfreetype -lpng
LDLIBS   += -lao -lpthread
#			-L/Archive/PS4-work/OrbisLink/samples/pc_es2template/source/ -lMiniAPI


.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(INC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@ 

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(INC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir $@

clean:
	$(RM) $(OBJ_DIR)/*.o $(EXE)
