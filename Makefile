.PHONY: memworld

memworld:
	gcc -o memworld memworld.c glad.c -lglfw3 -lpthread -framework Cocoa -framework OpenGL -framework IOKit

debug:
	gcc -o memworld memworld.c glad.c -lglfw3 -lpthread -framework Cocoa -framework OpenGL -framework IOKit -DDEBUG

debug-one:
	gcc -o memworld memworld.c glad.c -lglfw3 -lpthread -framework Cocoa -framework OpenGL -framework IOKit -DDEBUG -DDEBUG_ONE_PIXEL