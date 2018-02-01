#pragma once

#define MAX_FS_PATH               32768


//Path functions
BOOL GotoShellFolder(LPCWSTR szFolder);
BOOL GotoDataFolder();

//error code: 0 - error, 1 - success, 2 - cancelled
BOOL Location(PCWSTR szTitle, PDWORD pwdErrCode, UINT uFlags);

//time based file name
void TimeFileName(PWSTR szNameBuff, PCWSTR szPref, PCWSTR szPost);

