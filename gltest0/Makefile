NAME=gltest0
LINK=-lGL -lGLU
deb:
	gcc -Wall -g -o $(NAME) $(NAME).c $(LINK) `pkg-config --cflags --libs sdl`

rel:
	gcc -Wall -O2 -s -o $(NAME) $(NAME).c $(LINK) `pkg-config --cflags --libs sdl`

deb32:
	gcc -Wall -g -m32 -o $(NAME) $(NAME).c $(LINK) `pkg-config --cflags --libs sdl`

rel32:
	gcc -Wall -O2 -m32 -s -o $(NAME) $(NAME).c $(LINK) `pkg-config --cflags --libs sdl`

clean:
	rm $(NAME)

compress:
	tar czvf $(NAME).tar.gz $(NAME).c Makefile

extract:
	tar xvf $(NAME).tar.gz

