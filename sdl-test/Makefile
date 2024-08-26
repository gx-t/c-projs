FLAGS_MAC=`sdl2-config --cflags --libs` -lSDL2_ttf -DTTF_PATH=\"/System/Library/Fonts/Helvetica.ttc\"
FLAGS=`sdl2-config --cflags --libs` -lSDL2_ttf -DTTF_PATH=\"/usr/share/fonts/truetype/freefont/FreeSans.ttf\"
FLAGS_WIN=`sdl2-config --cflags --libs` -lSDL2_ttf -DTTF_PATH=\"C:/Windows/Fonts/times.ttf\"


00-deb-mac:
	$(CC) -g -o 00-rand-dots 00-rand-dots.c $(FLAGS_MAC)

00-rel-mac:
	$(CC) -O2 -o 00-rand-dots 00-rand-dots.c $(FLAGS_MAC)

00-deb:
	$(CC) -g -o 00-rand-dots 00-rand-dots.c $(FLAGS)

00-rel:
	$(CC) -O2 -s -o 00-rand-dots 00-rand-dots.c $(FLAGS)

00-deb-win:
	$(CC) -g -o 00-rand-dots 00-rand-dots.c $(FLAGS_WIN)

00-rel-win:
	$(CC) -O2 -s -o 00-rand-dots 00-rand-dots.c $(FLAGS_WIN)

01-deb-mac:
	$(CC) -g -o 01-puzzle15 01-puzzle15.c $(FLAGS_MAC)

01-rel-mac:
	$(CC) -O2 -o 01-puzzle15 01-puzzle15.c $(FLAGS_MAC)

01-deb:
	$(CC) -g -o 01-puzzle15 01-puzzle15.c $(FLAGS)

01-rel:
	$(CC) -O2 -s -o 01-puzzle15 01-puzzle15.c $(FLAGS)

01-deb-win:
	$(CC) -g -o 01-puzzle15 01-puzzle15.c $(FLAGS_WIN)

01-rel-win:
	$(CC) -O2 -s -o 01-puzzle15 01-puzzle15.c $(FLAGS_WIN)

clean:
	rm -rf 00-rand-dots 01-puzzle15 *.dsym


