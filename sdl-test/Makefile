.PHONY: 00-deb 00-rel 01-deb 01-rel clean

# Detect the operating system
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
  # macOS
  CFLAGS := `pkg-config --cflags --libs sdl3-ttf` -DTTF_PATH=\"/System/Library/Fonts/Helvetica.ttc\"
else ifeq ($(UNAME_S),Linux)
  # Linux
  CFLAGS := `pkg-config --cflags --libs sdl3-ttf` -DTTF_PATH=\"/usr/share/fonts/truetype/freefont/FreeSans.ttf\"
else
  # Windows
  CFLAGS := -lSDL3 -lSDL3_ttf -DTTF_PATH=\"C:/Windows/Fonts/times.ttf\" -mwindows
endif


00-deb:
	$(CC) -Wall -g -o 00-rand-dots 00-rand-dots.c $(CFLAGS)

00-rel:
	$(CC) -Wall -O2 -o 00-rand-dots 00-rand-dots.c  $(CFLAGS)
	strip 00-rand-dots

01-deb:
	$(CC) -Wall -g -o 01-puzzle15 01-puzzle15.c $(CFLAGS)

01-rel:
	$(CC) -Wall -O2 -o 01-puzzle15 01-puzzle15.c $(CFLAGS)
	strip 01-puzzle15

clean:
	rm -rf 00-rand-dots 01-puzzle15 *.dsym


