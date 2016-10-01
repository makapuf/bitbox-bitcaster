NAME = bitcaster
GAME_C_FILES = $(NAME).c
GAME_C_OPTS = -DVGAMODE_400

include $(BITBOX)/kernel/bitbox.mk

$(NAME).c: terrain.h

terrain.h: mk_terrain.py terrain.png
	python mk_terrain.py terrain.png > terrain.h

clean:: 
	rm -f terrain.h