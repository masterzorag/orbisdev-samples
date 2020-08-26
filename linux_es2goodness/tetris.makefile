# make -f tetris.makefile
# make -f tetris.makefile clean
# make -f tetris.makefile && ./egl_demo_tetris


# orbisdev
# export ORBISDEV=/Archive/PS4-work/orbisdev/orbisdev
# export PATH=$ORBISDEV/bin:$PATH

INC  = -I$(ORBISDEV)/usr/include/orbis/freetype-gl \
       -I$(ORBISDEV)/usr/include/orbis/freetype \
       -I$(ORBISDEV)/usr/include/orbis/MiniAPI \
       -Iinc \
       -I../tetris/include

EXE := egl_demo_tetris

SRC_DIR  := $(ORBISDEV)/../orbisdev-portlibs/freetype-gl/source
#SRC_DIR  += /home/user/Documents/orbisdev-portlibs/MiniAPI/source
#SRC_DIR := /home/user/Documents/orbisdev-portlibs/freetype-gl/source
SRC      := egl.c fileIO.c shader_common.c glslsandbox.c lines_and_rects.c rects.c ls_dir.c \
			../tetris/source/primlib.c \
			../tetris/source/game.c \
			../tetris/source/userInterface.c \
			$(ORBISDEV)/../orbisdev-liborbis/source/liborbisAudio/orbisAudio.c
SRC      += $(wildcard $(SRC_DIR)/*.c)

OBJ_DIR  := obj
OBJ      := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CC       := clang
CPPFLAGS := 
CFLAGS   := -Wall -O3 -DHAVE_LIBAO -DGLSLSANDBOX -DLINE_AND_RECT -DTETRIS -ggdb

# where libs are?
#LDFLAGS  :=	-L/home/user/Downloads/liborbis/portlibs/MiniAPI/lib -lMiniAPI
#LDFLAGS := -v

LDLIBS   := -lX11 -lEGL -lGLESv2 -lm -lfreetype
LDLIBS   += -lao -lpthread \
			-L/Archive/PS4-work/OrbisLink/samples/pc_es2template/source/ -lMiniAPI


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
