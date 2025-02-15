# Detect the operating system
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
  # macOS
  CFLAGS := `sdl2-config --cflags --libs` -lSDL2_ttf -DTTF_PATH=\"/System/Library/Fonts/Helvetica.ttc\"
else ifeq ($(UNAME_S),Linux)
  # Linux
  CFLAGS := `sdl2-config --cflags --libs` -lSDL2_ttf -DTTF_PATH=\"/usr/share/fonts/truetype/freefont/FreeSans.ttf\"
else
  # Windows
  CFLAGS := `sdl2-config --cflags --libs` -lSDL2_ttf -DTTF_PATH=\"C:/Windows/Fonts/times.ttf\"
endif


00-deb:
	$(CC) -Wall -g -o 00-rand-dots 00-rand-dots.c $(CFLAGS)

00-rel:
	$(CC) -Wall -O2 -o 00-rand-dots 00-rand-dots.c  $(CFLAGS)

01-deb:
	$(CC) -Wall -g -o 01-puzzle15 01-puzzle15.c $(CFLAGS)

01-rel:
	$(CC) -Wall -O2 -s -o 01-puzzle15 01-puzzle15.c $(CFLAGS)

clean:
	rm -rf 00-rand-dots 01-puzzle15 *.dsym


