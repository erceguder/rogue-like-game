#$(test_no) = 1

all: world.c monster.c logging.c utils.c
	gcc -o world world.c logging.c utils.c
	gcc -o monster monster.c utils.c

test: io/inp$(test_no).txt io/myout$(test_no).txt
	./world < io/inp$(test_no).txt > io/myout$(test_no).txt
	diff io/out$(test_no).txt io/myout$(test_no).txt
