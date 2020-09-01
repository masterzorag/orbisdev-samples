ifndef ORBISDEV
$(error ORBISDEV, is not set)
endif

target    ?= ps4_elf_sce
TargetFile = homebrew.elf

include $(ORBISDEV)/make/ps4sdk.mk

LinkerFlags += -lkernel_stub -lSceLibcInternal_stub -lSceSysmodule_stub -lSceSystemService_stub -lSceNet_stub -lSceUserService_stub -lScePigletv2VSH_stub -lSceVideoOut_stub -lSceGnmDriver_stub -lorbisGl -lorbis -lScePad_stub -lSceAudioOut_stub -lSceIme_stub

CompilerFlags += -D__ORBIS__ -DTARGET_UNIX -DTYRIAN_DIR='"/app0/media"' -DHG_REV='"24df4a4651f7+ sdl2"' -D_REENTRANT -DNDEBUG -std=c11 -pedantic -Wall -Wno-missing-field-initializers

#LinkerFlags += -L/home/user/Documents/orbisdev-portlibs/SDL/build -lSDL
#LinkerFlags += -L/opt/orbisdev-portlibs/SDL/build -lSDL




IncludePath += -I$(ORBISDEV)/usr/include -I$(ORBISDEV)/usr/include/c++/v1 -I$(ORBISDEV)/usr/include/orbis
IncludePath += -I$(ORBISDEV)/usr/include/orbis/SDL2
IncludePath += -Isource

install:
	@cp $(OutPath)/homebrew.self /hostapp/homebrew.self
	@echo "Installed!"
oelf:
	orbis-elf-create bin/homebrew.elf bin/homebrew.oelf


AUTH_INFO = 000000000000000000000000001C004000FF000000000080000000000000000000000000000000000000008000400040000000000000008000000000000000080040FFFF000000F000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000

eboot:
	python2 $(ORBISDEV)/bin/make_fself.py --auth-info $(AUTH_INFO) bin/homebrew.oelf bin/homebrew.self
pkg_build:
	cp bin/homebrew.self pkg/eboot.bin && cd pkg && pkgTool pkg_build Project.gp4 . && cp *.pkg ../bin/
