#pragma once
#pragma warning(disable:4201)
#pragma warning(disable:4100)
#define _WIN32_WINNT 0x0500
#define UNICODE

#include <winsock2.h>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlobj.h>
#include <mshtml.h>
#include <Lm.h>
#include <rpc.h>
#include <stdio.h>
#include <ctype.h>
#include <exdisp.h>
#include <shlwapi.h>

typedef PWSTR   *PPWSTR;
typedef PCWSTR  *PPCWSTR;

#include "crc32.h"
#include "crc16.h"
#include "HashMD5.h"
#include "Menu.h"
#include "Path.h"
#include "Clipboard.h"
#include "MessageLoop.h"
#include "Files.h"
#include "NetSend.h"
#include "DesktopList.h"
#include "MainWnd.h"
#include "TreeCtrl.h"
#include "Edit.h"
#include "Base64.h"
#include "Tools.h"
#include "Network.h"
#include "Shoutcast.h"
#include "ScanFiles.h"
#include "Text.h"
#include "Format.h"
#include "Html.h"
#include "Encryption.h"
#include "Rc4EncryptDecrypt.h"

#ifndef DESKTOPENUMPROC
#define DESKTOPENUMPROC DESKTOPENUMPROCW
#endif

#ifndef BIF_NONEWFOLDERBUTTON
#define BIF_NONEWFOLDERBUTTON  0x0200   // Do not add the "New Folder" button to the dialog.
                                        // Only applicable with BIF_NEWDIALOGSTYLE.
#endif

#ifdef __GNUC__

EXTERN_C const CLSID CLSID_ShellWindows;
EXTERN_C const IID IID_IShellWindows;
#define INTERFACE IShellWindows
DECLARE_INTERFACE_(IShellWindows,IDispatch)
{
	STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;
	STDMETHOD(GetTypeInfoCount)(THIS_ UINT*) PURE;
	STDMETHOD(GetTypeInfo)(THIS_ UINT,LCID,LPTYPEINFO*) PURE;
	STDMETHOD(GetIDsOfNames)(THIS_ REFIID,LPOLESTR*,UINT,LCID,DISPID*) PURE;
	STDMETHOD(Invoke)(THIS_ DISPID,REFIID,LCID,WORD,DISPPARAMS*,VARIANT*,EXCEPINFO*,UINT*) PURE;
	STDMETHOD_(ULONG,get_Count)(THIS,long*) PURE;
	STDMETHOD_(ULONG,Item)(THIS,VARIANT,IDispatch**) PURE;
	STDMETHOD_(ULONG,_NewEnum)(THIS,IUnknown**) PURE;
	STDMETHOD_(ULONG,Register)(THIS,IDispatch*,long,int,long*) PURE;
	STDMETHOD_(ULONG,RegisterPending)(THIS,long,VARIANT*,VARIANT*,int,long*) PURE;
	STDMETHOD_(ULONG,Revoke)(THIS,long) PURE;
	STDMETHOD_(ULONG,OnNavigate)(THIS,long,VARIANT*) PURE;
	STDMETHOD_(ULONG,OnActivated)(THIS,long,VARIANT_BOOL) PURE;
	STDMETHOD_(ULONG,FindWindowSW)(THIS,VARIANT*,VARIANT*,int,long*,int,IDispatch**) PURE;
	STDMETHOD_(ULONG,OnCreated)(THIS,long,IUnknown**) PURE;
	STDMETHOD_(ULONG,ProcessAttachDetach)(THIS,VARIANT_BOOL) PURE;
};
#undef INTERFACE

#endif //__GNUC__


extern PCWSTR g_szAppName;
extern HWND g_hMainWnd;

//Empty
int EmptyProc();

