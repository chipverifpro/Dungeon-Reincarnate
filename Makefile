# usage  make run 	 - makes and runs the program
#        make        - only makes the program
#        make clean  - gets rid of object files
INCLUDES := -I/opt/homebrew/Cellar/sdl2/2.30.7/include/SDL2 -I/opt/homebrew/Cellar/sdl2/2.30.7/include -I/opt/homebrew/Cellar/sdl2_image/2.8.2_2/include -I/opt/homebrew/Cellar/sdl2_ttf/2.22.0/include
LINKS    := -lSDL2 -lSDL2_image -lSDL2_ttf
LIBDIRS  := -L/opt/homebrew/Cellar/sdl2/2.30.7/lib -L/opt/homebrew/Cellar/sdl2_image/2.8.2_2/lib -L/opt/homebrew/Cellar/sdl2_ttf/2.22.0/lib
#COMP_OPTS:= -Wall -Og
COMP_OPTS:= -Wall -O0 -g -fsanitize=address

dungeon: main.o dungeon.o player.o sdl_draw.o map_io.o sdl_time.o sdl_symbols.o items.o buttons.o user_input.o messages.o monster.o battle.o
	gcc ${COMP_OPTS} main.o dungeon.o player.o sdl_draw.o sdl_time.o sdl_symbols.o map_io.o items.o buttons.o user_input.o messages.o monster.o battle.o ${LINKS} ${LIBDIRS} -odungeon

main.o: main.c *.h
	gcc ${COMP_OPTS} -c main.c ${INCLUDES}

dungeon.o: dungeon.c *.h
	gcc ${COMP_OPTS} -c dungeon.c ${INCLUDES}

player.o: player.c *.h
	gcc ${COMP_OPTS} -c player.c ${INCLUDES}

sdl_draw.o: sdl_draw.c *.h
	gcc ${COMP_OPTS} -c sdl_draw.c ${INCLUDES}

user_input.o: user_input.c *.h
	gcc ${COMP_OPTS} -c user_input.c ${INCLUDES}

sdl_time.o: sdl_time.c *.h
	gcc ${COMP_OPTS} -c sdl_time.c ${INCLUDES}

sdl_symbols.o: sdl_symbols.c *.h
	gcc ${COMP_OPTS} -c sdl_symbols.c ${INCLUDES}

map_io.o: map_io.c *.h
	gcc ${COMP_OPTS} -c map_io.c ${INCLUDES}

items.o: items.c *.h
	gcc ${COMP_OPTS} -c items.c ${INCLUDES}

buttons.o: buttons.c *.h
	gcc ${COMP_OPTS} -c buttons.c ${INCLUDES}

messages.o: messages.c *.h
	gcc ${COMP_OPTS} -c messages.c ${INCLUDES}

monster.o: monster.c *.h
	gcc ${COMP_OPTS} -c monster.c ${INCLUDES}

battle.o: battle.c *.h
	gcc ${COMP_OPTS} -c battle.c ${INCLUDES}

run: dungeon
	./dungeon

.PHONY: clean leaks

clean:
	rm -f *.o

leaks:
	leaks --atExit -- ./dungeon | tee leaks_out.txt