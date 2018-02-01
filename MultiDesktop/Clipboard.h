#pragma once

//Clipboard functions
//Clipboard
int Clipboard();
int WndTxtToClipboard(HWND hWnd);
void ClipGetHtml(void (*)(PCSTR, PVOID), PVOID);
void ClipGetData(void (*)(SIZE_T, PVOID, PVOID), UINT, PVOID, BOOL);
void ClipSetData(UINT, HANDLE);

