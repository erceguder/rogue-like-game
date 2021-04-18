all: world.c logging.c utils.c
	gcc -o world world.c logging.c utils.c
	gcc -o monster monster.c utils.c 
