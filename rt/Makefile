deb: *.c *.h rt.rc
	windres rt.rc -o rs.o
	mingw32-gcc -g -nostdlib -DUNICODE -D_WIN32_IE=0x0400 -DRT_LANG=EN -Wall -e_RtMain -o rt.exe *.c rs.o -lgcc -lkernel32 -lcomctl32 -luser32 -lshell32 -lcomdlg32 -lcrtdll

rel: *.c *.h rt.rc
	windres rt.rc -o rs.o
	mingw32-gcc -O2 -s -nostdlib -DUNICODE -D_WIN32_IE=0x0400 -DRT_LANG=EN -Wall -mwindows -e_RtMain -o rt.exe *.c rs.o -lgcc -lkernel32 -lcomctl32 -luser32 -lshell32 -lcomdlg32 -lcrtdll

dll: *.c *.h rt.rc
	windres rt.rc -o rs.o
	mingw32-gcc -O2 -s -nostdlib -D__DLL -DUNICODE -D_WIN32_IE=0x0400 -DRT_LANG=EN -Wall -shared -mconsole -e_RtDlEntry -o rt.dll *.c rs.o -lgcc -lkernel32 -lcomctl32 -luser32 -lshell32 -lcomdlg32 -lcrtdll

gcov: *.c *.h rt.rc
	windres rt.rc -o rs.o
	mingw32-gcc -pg -g -nostdlib -DUNICODE -D_WIN32_IE=0x0400 -DRT_LANG=EN -Wall -mconsole -e_RtMain -o rt.exe *.c rs.o -lgcc -lkernel32 -lcomctl32 -luser32 -lshell32 -lcomdlg32 -lcrtdll

clean:
	rm -rf *.o *.exe *.dll *.7z *.bak

bak:
	rm -rf *.bak *.s *.7z

7z:
	"C:\Program Files\7-Zip\7z" a rt.7z *.c *.h *.ico *.rc Makefile rt.exe rt.dll
