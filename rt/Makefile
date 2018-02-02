deb: *.c *.h rt.rc
	x86_64-w64-mingw32-windres rt.rc -o rs.o
	x86_64-w64-mingw32-gcc -g -DUNICODE -D_WIN32_IE=0x0400 -DRT_LANG=EN -Wall -o rt.exe *.c rs.o -lgcc -lkernel32 -lcomctl32 -luser32 -lshell32 -lcomdlg32

rel: *.c *.h rt.rc
	x86_64-w64-mingw32-windres rt.rc -o rs.o
	x86_64-w64-mingw32-gcc -O2 -s -DUNICODE -D_WIN32_IE=0x0400 -DRT_LANG=EN -Wall -mwindows -o rt.exe *.c rs.o -lgcc -lkernel32 -lcomctl32 -luser32 -lshell32 -lcomdlg32

dll: *.c *.h rt.rc
	x86_64-w64-mingw32-windres rt.rc -o rs.o
	x86_64-w64-mingw32-gcc -O2 -s -D__DLL -DUNICODE -D_WIN32_IE=0x0400 -DRT_LANG=EN -Wall -shared -mconsole -e_RtDlEntry -o rt.dll *.c rs.o -lgcc -lkernel32 -lcomctl32 -luser32 -lshell32 -lcomdlg32

gcov: *.c *.h rt.rc
	x86_64-w64-mingw32-windres rt.rc -o rs.o
	x86_64-w64-mingw32-gcc -pg -g -DUNICODE -D_WIN32_IE=0x0400 -DRT_LANG=EN -Wall -mconsole -o rt.exe *.c rs.o -lgcc -lkernel32 -lcomctl32 -luser32 -lshell32 -lcomdlg32

clean:
	rm -rf *.o *.exe *.dll *.7z *.bak

bak:
	rm -rf *.bak *.s *.7z

7z:
	"C:\Program Files\7-Zip\7z" a rt.7z *.c *.h *.ico *.rc Makefile rt.exe rt.dll
