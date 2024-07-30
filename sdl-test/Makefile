LFLAGS_MAC=`sdl2-config --cflags --libs` -lSDL2_ttf -DTTF_PATH=\"/System/Library/Fonts/Helvetica.ttc\"


00-deb-mac:
	$(CC) -g -o 00-rand-dots 00-rand-dots.c $(LFLAGS_MAC)

00-rel-mac:
	$(CC) -O2 -o 00-rand-dots 00-rand-dots.c $(LFLAGS_MAC)

01-deb-mac:
	$(CC) -g -o 01-puzzle15 01-puzzle15.c $(LFLAGS_MAC)

01-rel-mac:
	$(CC) -O2 -o 01-puzzle15 01-puzzle15.c $(LFLAGS_MAC)

clean:
	rm -rf 00-rand-dots 01-puzzle15 *.dsym
