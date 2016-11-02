NAME = bitcaster
GAME_C_FILES = $(NAME).c
DEFINES = VGA_MODE=400

include $(BITBOX)/kernel/bitbox.mk

$(NAME).c: terrain.h

terrain.h: mk_terrain.py terrain.png
	python mk_terrain.py terrain.png > terrain.h

clean:: 
	rm -f terrain.h