//---------------------------------------------------------------------------
// Copyright 2002-2008 Andre Burgaud <andre@burgaud.com>
// See license.txt
// $Id: wscitecm.cpp 497 2008-12-07 03:17:37Z andre $
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// wscitecm.cpp
// Defines the entry point for the DLL application.
//---------------------------------------------------------------------------

#ifndef STRICT
#define STRICT
#endif

#define INC_OLE2

#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>

#define GUID_SIZE 128
#define MAX_FILES 10
#define MAX_CMDSTR (MAX_PATH * MAX_FILES)
#define ResultFromShort(i) ResultFromScode(MAKE_SCODE(SEVERITY_SUCCESS, 0, (USHORT)(i)))

#pragma data_seg(".text")
#define INITGUID
#include <initguid.h>
#include <shlguid.h>
#include "resource.h"
#include "wscitecm.h"
#pragma data_seg()

//---------------------------------------------------------------------------
//  Global variables
//---------------------------------------------------------------------------
UINT _cRef = 0; // COM Reference count.
HINSTANCE _hModule = NULL; // DLL Module.

typedef struct {
	HKEY  hRootKey;
	LPTSTR szSubKey;
	LPTSTR lpszValueName;
	LPTSTR szData;
} DOREGSTRUCT, *LPDOREGSTRUCT;

char szSciTEName[] = "SciTE.exe";
char szShellExtensionTitle[] = "SciTE";

BOOL RegisterServer(CLSID, LPTSTR);
BOOL UnregisterServer(CLSID, LPTSTR);
void MsgBox(LPTSTR);
void MsgBoxDebug(LPTSTR);
void MsgBoxError(LPTSTR);

//---------------------------------------------------------------------------
// DllMain
//---------------------------------------------------------------------------
extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved) {
	if (dwReason == DLL_PROCESS_ATTACH) {
		_hModule = hInstance;
	}
	return 1;
}

//---------------------------------------------------------------------------
// DllCanUnloadNow
//---------------------------------------------------------------------------
STDAPI DllCanUnloadNow(void) {
	return (_cRef == 0 ? S_OK : S_FALSE);
}

//---------------------------------------------------------------------------
// DllGetClassObject
//---------------------------------------------------------------------------
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvOut) {
	*ppvOut = NULL;
	if (IsEqualIID(rclsid, CLSID_ShellExtension)) {
		CShellExtClassFactory *pcf = new CShellExtClassFactory;
		return pcf->QueryInterface(riid, ppvOut);
	}
	return CLASS_E_CLASSNOTAVAILABLE;
}

//---------------------------------------------------------------------------
// DllRegisterServer
//---------------------------------------------------------------------------
STDAPI DllRegisterServer() {
	return (RegisterServer(CLSID_ShellExtension, szShellExtensionTitle) ? S_OK : E_FAIL);
}

//---------------------------------------------------------------------------
// DllUnregisterServer
//---------------------------------------------------------------------------
STDAPI DllUnregisterServer(void) {
	return (UnregisterServer(CLSID_ShellExtension, szShellExtensionTitle) ? S_OK : E_FAIL);
}

//---------------------------------------------------------------------------
// CheckSciTE
//---------------------------------------------------------------------------
BOOL CheckSciTE() {
	return TRUE; // TDMS 3 June 2017, since we are deploying via an installer, assume the existence of SciTE

	TCHAR szModuleFullName[MAX_PATH];
	TCHAR szExeFullName[MAX_PATH];
	int nLenPath = 0;
	TCHAR* pDest;
	LPTSTR *lpFilePart = NULL;

	GetModuleFileName(_hModule, szModuleFullName, MAX_PATH);
	pDest = strrchr(szModuleFullName, '\\');
	pDest++;
	pDest[0] = 0;

	DWORD dw = SearchPath(szModuleFullName, szShellExtensionTitle, ".exe", MAX_PATH, szExeFullName, lpFilePart);

	return (dw ? TRUE : FALSE);
}

//---------------------------------------------------------------------------
// RegisterServer
//---------------------------------------------------------------------------
BOOL RegisterServer(CLSID clsid, LPTSTR lpszTitle) {
	int      i;
	HKEY     hKey;
	LRESULT  lResult;
	DWORD    dwDisp;
	TCHAR    szSubKey[MAX_PATH];
	TCHAR    szCLSID[MAX_PATH];
	TCHAR    szModule[MAX_PATH];
	LPWSTR   pwsz;

	if (!CheckSciTE()) {
		MsgBoxError("To register the SciTE context menu extension,\r\ninstall wscitecm.dll in the same directory as SciTE.exe.");
		return FALSE;
	}

	StringFromIID(clsid, &pwsz);
	if (pwsz) {
#ifdef UNICODE
		lstrcpy(szCLSID, pwsz);
#else
		WideCharToMultiByte(CP_ACP, 0, pwsz, -1, szCLSID, ARRAYSIZE(szCLSID), NULL, NULL);
#endif
		//free the string
		LPMALLOC pMalloc;
		CoGetMalloc(1, &pMalloc);
		pMalloc->Free(pwsz);
		pMalloc->Release();
	}

	//get this app's path and file name
	GetModuleFileName(_hModule, szModule, MAX_PATH);

	DOREGSTRUCT ClsidEntries[] = {
		HKEY_CLASSES_ROOT,   TEXT("CLSID\\%s"),                              NULL,                   lpszTitle,
		HKEY_CLASSES_ROOT,   TEXT("CLSID\\%s\\InprocServer32"),              NULL,                   szModule,
		HKEY_CLASSES_ROOT,   TEXT("CLSID\\%s\\InprocServer32"),              TEXT("ThreadingModel"), TEXT("Apartment"),
		HKEY_CLASSES_ROOT,   TEXT("*\\shellex\\ContextMenuHandlers\\SciTE"), NULL,                   szCLSID,
		NULL,                NULL,                                           NULL,                   NULL
	};

	// Register the CLSID entries
	for (i = 0; ClsidEntries[i].hRootKey; i++) {
		// Create the sub key string - for this case, insert the file extension
		wsprintf(szSubKey, ClsidEntries[i].szSubKey, szCLSID);
		lResult = RegCreateKeyEx(ClsidEntries[i].hRootKey, szSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisp);
		if (NOERROR == lResult) {
			TCHAR szData[MAX_PATH];
			// If necessary, create the value string
			wsprintf(szData, ClsidEntries[i].szData, szModule);
			lResult = RegSetValueEx(hKey, ClsidEntries[i].lpszValueName, 0, REG_SZ, (LPBYTE)szData, (lstrlen(szData) + 1) * sizeof(TCHAR));
			RegCloseKey(hKey);
		}
		else
			return FALSE;
	}
	return TRUE;
}

//---------------------------------------------------------------------------
// UnregisterServer
//---------------------------------------------------------------------------
BOOL UnregisterServer(CLSID clsid, LPTSTR lpszTitle) {
	TCHAR szCLSID[GUID_SIZE + 1];
	TCHAR szCLSIDKey[GUID_SIZE + 32];
	TCHAR szKeyTemp[MAX_PATH + GUID_SIZE];
	LPWSTR pwsz;

	StringFromIID(clsid, &pwsz);
	if (pwsz) {
#ifdef UNICODE
		lstrcpy(szCLSID, pwsz);
#else
		WideCharToMultiByte(CP_ACP, 0, pwsz, -1, szCLSID, ARRAYSIZE(szCLSID), NULL, NULL);
#endif
		//free the string
		LPMALLOC pMalloc;
		CoGetMalloc(1, &pMalloc);
		pMalloc->Free(pwsz);
		pMalloc->Release();
	}

	lstrcpy(szCLSIDKey, TEXT("CLSID\\"));
	lstrcat(szCLSIDKey, szCLSID);

	wsprintf(szKeyTemp, TEXT("*\\shellex\\ContextMenuHandlers\\%s"), lpszTitle);
	RegDeleteKey(HKEY_CLASSES_ROOT, szKeyTemp);

	wsprintf(szKeyTemp, TEXT("%s\\%s"), szCLSIDKey, TEXT("InprocServer32"));
	RegDeleteKey(HKEY_CLASSES_ROOT, szKeyTemp);
	RegDeleteKey(HKEY_CLASSES_ROOT, szCLSIDKey);

	return TRUE;
}

//---------------------------------------------------------------------------
// MsgBoxDebug
//---------------------------------------------------------------------------
void MsgBoxDebug(LPTSTR lpszMsg) {
	MessageBox(NULL,
		lpszMsg,
		"DEBUG",
		MB_OK);
}

//---------------------------------------------------------------------------
// MsgBox
//---------------------------------------------------------------------------
void MsgBox(LPTSTR lpszMsg) {
	MessageBox(NULL,
		lpszMsg,
		"SciTE Extension",
		MB_OK);
}

//---------------------------------------------------------------------------
// MsgBoxError
//---------------------------------------------------------------------------
void MsgBoxError(LPTSTR lpszMsg) {
	MessageBox(NULL,
		lpszMsg,
		"SciTE Extension",
		MB_OK | MB_ICONSTOP);
}

//---------------------------------------------------------------------------
// CShellExtClassFactory
//---------------------------------------------------------------------------
CShellExtClassFactory::CShellExtClassFactory() {
	m_cRef = 0L;
	_cRef++;
}

CShellExtClassFactory::~CShellExtClassFactory() {
	_cRef--;
}

STDMETHODIMP CShellExtClassFactory::QueryInterface(REFIID riid, LPVOID FAR *ppv) {
	*ppv = NULL;
	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory)) {
		*ppv = (LPCLASSFACTORY)this;
		AddRef();
		return NOERROR;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CShellExtClassFactory::AddRef() {
	return ++m_cRef;
}

STDMETHODIMP_(ULONG) CShellExtClassFactory::Release()
{
	if (--m_cRef)
		return m_cRef;
	delete this;
	return 0L;
}

STDMETHODIMP CShellExtClassFactory::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID *ppvObj) {
	*ppvObj = NULL;
	if (pUnkOuter)
		return CLASS_E_NOAGGREGATION;
	LPCSHELLEXT pShellExt = new CShellExt();
	if (NULL == pShellExt)
		return E_OUTOFMEMORY;
	return pShellExt->QueryInterface(riid, ppvObj);
}

STDMETHODIMP CShellExtClassFactory::LockServer(BOOL fLock) {
	return NOERROR;
}

//---------------------------------------------------------------------------
// CShellExt
//---------------------------------------------------------------------------
CShellExt::CShellExt() {
	m_cRef = 0L;
	m_pDataObj = NULL;
	_cRef++;
	m_hSciteBmp = LoadBitmap(_hModule, MAKEINTRESOURCE(IDB_SCITE));
	HRESULT hr;
	hr = SHGetMalloc(&m_pAlloc);
	if (FAILED(hr))
		m_pAlloc = NULL;
}

CShellExt::~CShellExt() {
	if (m_pDataObj)
		m_pDataObj->Release();
	_cRef--;
	m_pAlloc->Release();
}

STDMETHODIMP CShellExt::QueryInterface(REFIID riid, LPVOID FAR *ppv) {
	*ppv = NULL;
	if (IsEqualIID(riid, IID_IShellExtInit) || IsEqualIID(riid, IID_IUnknown)) {
		*ppv = (LPSHELLEXTINIT)this;
	}
	else if (IsEqualIID(riid, IID_IContextMenu)) {
		*ppv = (LPCONTEXTMENU)this;
	}
	if (*ppv) {
		AddRef();
		return NOERROR;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CShellExt::AddRef() {
	return ++m_cRef;
}

STDMETHODIMP_(ULONG) CShellExt::Release() {
	if (--m_cRef)
		return m_cRef;
	delete this;
	return 0L;
}

STDMETHODIMP CShellExt::Initialize(LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObj, HKEY hRegKey) {
	HRESULT hres = 0;
	if (m_pDataObj)
		m_pDataObj->Release();
	if (pDataObj) {
		m_pDataObj = pDataObj;
		pDataObj->AddRef();
	}
	return NOERROR;
}

STDMETHODIMP CShellExt::QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags) {
	UINT idCmd = idCmdFirst;
	BOOL bAppendItems = TRUE;
	char szItemSciTE[] = "Edit with &SciTE in New Tab";
	char szItemSciTEw[] = "Edit with SciTE in New &Window";

	FORMATETC fmte = {
		CF_HDROP,
		(DVTARGETDEVICE FAR *)NULL,
		DVASPECT_CONTENT,
		-1,
		TYMED_HGLOBAL
	};

	HRESULT hres = m_pDataObj->GetData(&fmte, &m_stgMedium);

	if (SUCCEEDED(hres)) {
		if (m_stgMedium.hGlobal)
			m_cbFiles = DragQueryFile((HDROP)m_stgMedium.hGlobal, (UINT)-1, 0, 0);
	}

	UINT nIndex = indexMenu++;
	//idCmd ++;
	InsertMenu(hMenu, nIndex, MF_STRING | MF_BYPOSITION, idCmd++, szItemSciTE);
	if (m_hSciteBmp) {
		SetMenuItemBitmaps(hMenu, nIndex, MF_BYPOSITION, m_hSciteBmp, m_hSciteBmp);
	}

	nIndex += 1;
	//idCmd ++;
	InsertMenu(hMenu, nIndex, MF_STRING | MF_BYPOSITION, idCmd++, szItemSciTEw);
	if (m_hSciteBmp) {
		SetMenuItemBitmaps(hMenu, nIndex, MF_BYPOSITION, m_hSciteBmp, m_hSciteBmp);
	}

	return ResultFromShort(idCmd - idCmdFirst);
}


STDMETHODIMP CShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi) {
	HRESULT hr = E_INVALIDARG;

	if (!HIWORD(lpcmi->lpVerb)) {
		UINT idCmd = LOWORD(lpcmi->lpVerb);
		switch (idCmd) {
		case 0:
			hr = InvokeSciTE(lpcmi->hwnd, lpcmi->lpDirectory, lpcmi->lpVerb, " -check.if.already.open=1 ", lpcmi->nShow);
			break;

		case 1:
			//hr = InvokeSciTE(lpcmi->hwnd, lpcmi->lpDirectory, lpcmi->lpVerb, lpcmi->lpParameters, lpcmi->nShow);
			//char params[] = "-check.if.already.open=0";
			//lpcmi->lpParameters = " -check.if.already.open=1 ";

			hr = InvokeSciTE(lpcmi->hwnd, lpcmi->lpDirectory, lpcmi->lpVerb, "", lpcmi->nShow); //lpcmi->lpParameters, lpcmi->nShow);
			break;
		}
	}
	return hr;
}

STDMETHODIMP CShellExt::GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT FAR *reserved, LPSTR pszName, UINT cchMax) {
	if (uFlags == GCS_HELPTEXT && cchMax > 35)
		lstrcpy(pszName, "Edits the selected file(s) with SciTE");
	return NOERROR;
}

static void getSciTEName(char *name) {
	TCHAR szModuleFullName[MAX_PATH];
	int nLenPath = 0;
	TCHAR* pDest;

	name[0] = 0;
	GetModuleFileName(_hModule, szModuleFullName, MAX_PATH);
	pDest = strrchr(szModuleFullName, '\\');
	pDest++;
	pDest[0] = 0;
	strcpy_s(name, MAX_PATH, szModuleFullName);
	strcat_s(name, MAX_PATH, szSciTEName);

	if (name[0] == 0)
		strcpy_s(name, MAX_PATH, szSciTEName);
}

//STDMETHODIMP CShellExt::InvokeSciTE(HWND hParent, LPCSTR pszWorkingDir, LPCSTR pszCmd, LPCSTR pszParam, int iShowCmd) {
STDMETHODIMP CShellExt::InvokeSciTE(HWND hParent, LPCSTR pszWorkingDir, LPCSTR pszCmd, char *pszParam, int iShowCmd) {
	TCHAR szFileUserClickedOn[MAX_PATH];
	LPTSTR pszCommand;
	LPTSTR pszCommand2;
	UINT nSizeCommand;
	UINT i;

	FORMATETC fmte = {
		CF_HDROP,
		(DVTARGETDEVICE FAR *)NULL,
		DVASPECT_CONTENT,
		-1,
		TYMED_HGLOBAL
	};

	//  nSizeCommand = MAX_PATH * (m_cbFiles + 1) * sizeof(TCHAR);
	nSizeCommand = MAX_PATH * (m_cbFiles + 1) * sizeof(TCHAR) + sizeof(pszParam);
	pszCommand = (LPTSTR)m_pAlloc->Alloc(nSizeCommand);

	pszCommand2 = (LPTSTR)m_pAlloc->Alloc(nSizeCommand);
	pszCommand2[0] = 0;
	strcat_s(pszCommand2, nSizeCommand, pszParam); //" -check.if.already.open=1 ");

	if (pszCommand) {
		getSciTEName(pszCommand);
	}
	else {
		MsgBoxError("Insufficient memory available.");
		return E_FAIL;
	}

	for (i = 0; i < m_cbFiles; i++) {
		DragQueryFile((HDROP)m_stgMedium.hGlobal, i, szFileUserClickedOn, MAX_PATH);
		strcat_s(pszCommand2, nSizeCommand, " \"");
		strcat_s(pszCommand2, nSizeCommand, szFileUserClickedOn);
		strcat_s(pszCommand2, nSizeCommand, "\"");
		strcat_s(pszCommand2, nSizeCommand, " -check.if.already.open=0");
	}

	//strcat_s(pszCommand, nSizeCommand, pszParam);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_RESTORE;

	//MessageBox(hParent, pszCommand2, "SciTE Extension", MB_OK);

	//  if (!CreateProcess (NULL, pszCommand, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
	if (!CreateProcess(pszCommand, pszCommand2, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		MessageBox(hParent, pszCommand, "Scite Extension - debug information pszCommand", MB_OK);
		MessageBox(hParent, pszCommand2, "Scite Extension - debug information pszCommand2", MB_OK);

		MessageBox(hParent,
			"Error creating process: wscitecm.dll needs to be in the same directory as SciTE.exe",
			"SciTE Extension",
			MB_OK);
	}

	m_pAlloc->Free(pszCommand);
	return NOERROR;
}