
all: main.c display.c snake.c vector.c server/server.c server/sgame.c queue.c
	gcc -g -Wall -Wextra -o snekMMO main.c display.c snake.c vector.c server/server.c server/sgame.c queue.c -lpthread
