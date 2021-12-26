.PHONY: memworld

memworld: memworld.c glad.c
	gcc -o memworld memworld.c glad.c -lglfw3 -lpthread -framework Cocoa -framework OpenGL -framework IOKit

debug: memworld.c glad.c
	gcc -o memworld memworld.c glad.c -lglfw3 -lpthread -framework Cocoa -framework OpenGL -framework IOKit -DDEBUG

debug-one: memworld.c glad.c
	gcc -o memworld memworld.c glad.c -lglfw3 -lpthread -framework Cocoa -framework OpenGL -framework IOKit -DDEBUG -DDEBUG_ONE_PIXEL