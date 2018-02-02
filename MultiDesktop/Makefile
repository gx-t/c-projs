all: multidesktop.exe

CC=x86_64-w64-mingw32-gcc
LD=x86_64-w64-mingw32-gcc
RC=x86_64-w64-mingw32-windres

CCFLAGS=-O2 -s -D_WIN32_IE=0x0400 -c
LDFLAGS=-s -mwindows 

#CCFLAGS=-g -D_WIN32_IE=0x0400 -c
#LDFLAGS=-mwindows -nostdlib -e__main

LIBS=-lgcc -lkernel32 -luser32 -ladvapi32 -lcomdlg32 -lshell32 -lws2_32 -lole32 -lrpcrt4 -lcomctl32 -lnetapi32 -loleaut32

SRCS=common.c \
	Bace64.c \
	Clipboard.c \
	DesktopList.c \
	Edit.c \
	Encryption.c \
	Files.c \
	Format.c \
	HashMD5.c \
	Html.c \
	MainWnd.c \
	Menu.c \
	MessageLoop.c \
	MultiDesktop.c \
	NetSend.c \
	Network.c \
	Path.c \
	Rc4EncryptDecrypt.c \
	ScanFiles.c \
	Shoutcast.c \
	Text.c \
	Tools.c \
	TreeCtrl.c

OBJS=common.o \
	Bace64.o \
	Clipboard.o \
	DesktopList.o \
	Edit.o \
	Encryption.o \
	Files.o \
	Format.o \
	HashMD5.o \
	Html.o \
	MainWnd.o \
	Menu.o \
	MessageLoop.o \
	MultiDesktop.o \
	NetSend.o \
	Network.o \
	Path.o \
	Rc4EncryptDecrypt.o \
	ScanFiles.o \
	Shoutcast.o \
	Text.o \
	Tools.o \
	TreeCtrl.o

RSRC=resource.o

multidesktop.exe: $(OBJS) $(RSRC)
	$(LD) $(LDFLAGS) -omultidesktop.exe $(OBJS) $(RSRC) $(LIBS)

$(OBJS): $(SRCS)
	$(CC) $(CCFLAGS) $(SRCS)

$(RSRC): MultiDesktop.rc
	$(RC) MultiDesktop.rc -D_WIN32_IE=0x0400 -o$(RSRC)

clean:
	rm -f *.o *.exe
