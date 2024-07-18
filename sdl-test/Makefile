LFLAGS_MAC=`sdl2-config --cflags --libs` -lSDL2_ttf -DTTF_PATH=\"/System/Library/Fonts/Helvetica.ttc\"


deb-mac:
	$(CC) -g -o test-0 test-0.c $(LFLAGS_MAC)

rel-mac:
	$(CC) -O2 -o test-0 test-0.c $(LFLAGS_MAC)

clean:
	rm -rf test-0 test-0.dsym
