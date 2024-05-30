deb:
	windres rt.rc -o rs.o
	gcc -g -DUNICODE -D_WIN32_IE=0x0400 -DRT_LANG=EN -Wall -o rt.exe *.c rs.o -lgcc -lkernel32 -lcomctl32 -luser32 -lshell32 -lcomdlg32

rel:
	windres rt.rc -o rs.o
	gcc -O2 -s -DUNICODE -D_WIN32_IE=0x0400 -DRT_LANG=EN -Wall -mwindows -o rt.exe *.c rs.o -lgcc -lkernel32 -lcomctl32 -luser32 -lshell32 -lcomdlg32

gcov:
	windres rt.rc -o rs.o
	gcc -pg -g -DUNICODE -D_WIN32_IE=0x0400 -DRT_LANG=EN -Wall -mconsole -o rt.exe *.c rs.o -lgcc -lkernel32 -lcomctl32 -luser32 -lshell32 -lcomdlg32

clean:
	rm -rf *.o *.exe *.dll *.7z *.bak tags *.out

bak:
	rm -rf *.bak *.s *.7z
