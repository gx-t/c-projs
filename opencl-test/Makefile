MAKEFLAGS += -j 2
.PHONY: deb rel 00-deb 00-rel 01-deb 01-rel clean

# Detect platform
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
    PLATFORM_CFLAGS := -Wall
    PLATFORM_LDFLAGS := -framework OpenCL
    PLATFORM_DEFINE := -DPLATFORM_APPLE
else
    PLATFORM_CFLAGS := -Wall
    PLATFORM_LDFLAGS := -lOpenCL
    PLATFORM_DEFINE := -DPLATFORM_LINUX
endif

CFLAGS_DEBUG := $(PLATFORM_CFLAGS) -g $(PLATFORM_DEFINE)
CFLAGS_RELEASE := $(PLATFORM_CFLAGS) -O3 $(PLATFORM_DEFINE)
LDFLAGS := $(PLATFORM_LDFLAGS)

deb: 00-deb 01-deb
rel: 00-rel 01-rel

00-deb:
	$(CC) $(CFLAGS_DEBUG) -o 00-get-info 00-get-info.c $(LDFLAGS)

00-rel:
	$(CC) $(CFLAGS_RELEASE) -o 00-get-info 00-get-info.c $(LDFLAGS)

01-deb:
	$(CC) $(CFLAGS_DEBUG) -o 01-simple-calc 01-simple-calc.c $(LDFLAGS)

01-rel:
	$(CC) $(CFLAGS_RELEASE) -o 01-simple-calc 01-simple-calc.c $(LDFLAGS)

clean:
	rm -rf 00-get-info 01-simple-calc
