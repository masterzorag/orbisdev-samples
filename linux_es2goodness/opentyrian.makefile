# make -f opentyrian.makefile
# make -f opentyrian.makefile clean
# make -f opentyrian.makefile && ./egl_demo_opentyrian


# orbisdev
# export ORBISDEV=/Archive/PS4-work/orbisdev/orbisdev
# export PATH=$ORBISDEV/bin:$PATH

INC  = -I$(ORBISDEV)/usr/include/orbis/freetype-gl \
       -I$(ORBISDEV)/usr/include/orbis/freetype \
       -Iinc \
       -I/home/user/Documents/basic_3/include \
       -I/usr/include/SDL2

EXE := egl_demo_opentyrian


SRC_DIR  := /home/user/Documents/basic_3/source/opentyrian
SRC      := egl.c fileIO.c shader_common.c cmd_build.c \
			/home/user/Documents/basic_3/source/myGLES2.c \
			/home/user/Documents/basic_3/source/mySDL.c \
			$(ORBISDEV)/../orbisdev-liborbis/source/liborbisAudio/orbisAudio.c
SRC      += $(wildcard $(SRC_DIR)/*.c)

OBJ_DIR  := obj
OBJ      := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CC       := clang
CPPFLAGS := 
CFLAGS   := -Wall -O3 -DHAVE_LIBAO -DOPENTYRIAN -ggdb \
			-DTARGET_UNIX -DTYRIAN_DIR='"/home/user/Downloads/tyrian/opentyrian/tyrian21"' -DHG_REV='"24df4a4651f7+ sdl2"' -D_REENTRANT -DNDEBUG -pedantic -MMD -Wall -Wno-missing-field-initializers

# where libs are?
#LDFLAGS  :=	-L/home/user/Downloads/liborbis/portlibs/MiniAPI/lib -lMiniAPI
#LDFLAGS := -v

LDLIBS   := -lX11 -lEGL -lGLESv2 -lm -lfreetype 
#-lSDL2
LDLIBS   += -lao -lpthread

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
