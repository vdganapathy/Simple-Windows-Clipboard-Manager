#include <Windows.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <wingdi.h>
#include <string>
#include <vector>
#include<Psapi.h>
using namespace std;
// GLOBAL VARIABLES
char c;
int kcode,prevkcode=0;
KBDLLHOOKSTRUCT *kbdst;
MOUSEHOOKSTRUCT *mhs;
LPSTR windtext;
DWORD curpid, tid;
LPDWORD curpidmain;
string str = "";
const char * newchar = "";
bool releaseclipboard = TRUE;
int icount = 0;
vector <string> textlist;
vector<string>::iterator pos;
typedef bool(*SetWindowHandle)(HWND);
SetWindowHandle swh;
typedef bool(*install)();
install instal;
typedef bool(*uninstall)();
uninstall uninstal;
HINSTANCE hinst;
HWND hWnd, SelWinH,pwindh;
static HWND hwndNextViewer;
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
// GLOBAL DECLARATION
void printclipbrd();
void sendin(int, INPUT[]);
BOOL clipbrdsetdata();
int main()
{
	MSG msg;
	BOOL bRet;
	static const char* class_name = "DUMMY_CLASS";
	WNDCLASSEX wx = {};
	wx.cbSize = sizeof(WNDCLASSEX);
	wx.lpfnWndProc = WndProc;        // function which will handle messages
	wx.hInstance = hinst;
	wx.lpszClassName = class_name;
	if (RegisterClassEx(&wx)) {
		hWnd = CreateWindowEx(0, class_name, "dummy_name", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
	}

	hinst = LoadLibrary(TEXT("GlobalHookDLL.dll"));
	if (!hinst)
	{
		printf("The DLL could not be found.\n");
	}

	// MOUSE HOOK INSTALLATION
	instal = (install)GetProcAddress(hinst, "install");
	if (!instal())
	{
		printf("func couldn't be executed");
	}
	else{
		printf("\nProgram successfully hooked.");
	}



	// SETTING THIS WINDOW HANDLE TO THE DLL
	swh = (SetWindowHandle)GetProcAddress(hinst, "setwindowhandle");
	if (!swh(hWnd))
	{
		printf("\nDLL FAILED TO ADDED THIS WINDOW HANDLE");
	}
	else{
		printf("\nDLL ADDED THIS WINDOW HANDLE");
	}



	// HOTKEY REGISTRATION
	if (RegisterHotKey(hWnd, 888, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 0x43)){
		printf("\n HOT KEY REGISTERED");
	}
	else{
		DWORD errorno;
		errno = GetLastError();
		printf("\nHOT KEY REGISTRATION FAILED \t ERROR NO = %d", errno);
	}
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		//TranslateMessage(&msg);
		DispatchMessage(&msg);
		
	}
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	DWORD pid;
	
	switch (message)
	{
	
	case WM_CREATE:

		// Add the window to the clipboard viewer chain. 

		hwndNextViewer = SetClipboardViewer(hWnd);
		break;
	case WM_CHANGECBCHAIN:

		// If the next window is closing, repair the chain. 

		if ((HWND)wParam == hwndNextViewer)
			hwndNextViewer = (HWND)lParam;

		// Otherwise, pass the message to the next link. 

		else if (hwndNextViewer != NULL)
			SendMessage(hwndNextViewer, message, wParam, lParam);

		break;


	case WM_USER + 1:
		
		mhs = (MOUSEHOOKSTRUCT*)lParam;
		cout <<endl<< "x=" << mhs->pt.x<<" y="<<mhs->pt.y;
		SelWinH = mhs->hwnd;
		cout << endl << "selwinh = " << SelWinH;
		cout << endl << "the foreground window = " << GetForegroundWindow();
		cout << endl << "Selected window process id=" << GetProcessId(SelWinH);
		tid = GetWindowThreadProcessId(mhs->hwnd, &pid);
		cout << endl << "tid =" << tid << "   " << "pid = " << pid;
		cout << endl << "---------------------------------------------------------------------------------";
		
		break;
	case WM_USER + 2:
		kbdst = (KBDLLHOOKSTRUCT*)lParam;
		c=MapVirtualKey(kbdst->vkCode, MAPVK_VK_TO_CHAR);
		cout << endl << c;
		kcode = MapVirtualKey(kbdst->scanCode, MAPVK_VSC_TO_VK);
		if (kcode == 86 && prevkcode == VK_CONTROL)//86 code for 'v'
		{
			if (icount+1 > textlist.size())
			{
				MessageBox(NULL, "LIST ENDED", "WARNING", MB_OK);
				OpenClipboard(0);
				EmptyClipboard();
				CloseClipboard();
				icount = 0;
			}
			else{
				cout << endl << "CTRL+V"<<"icount ="<<icount;
				clipbrdsetdata();
			}
			
		}
		else{
			prevkcode = kcode;
		}
		
		
		
		break;
	case WM_DRAWCLIPBOARD:
		
		if (releaseclipboard)
		{
			printf("\nClipboard Changed");
			printclipbrd();
		}
		break;
	case WM_HOTKEY:
		cout << endl << "Hot Key Pressed";
		textlist.clear();
		icount = 0;
		break;

	case WM_DESTROY:
		uninstal = (uninstall)GetProcAddress(hinst, "uninstall");
		if (uninstal())
		{
			printf("\nGLOBAL UNHOKED");
		}
		else
		{
			printf("\nUnhook Error");

		}
		ChangeClipboardChain(hWnd, hwndNextViewer);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;

}

void printclipbrd()
{
	HGLOBAL   hglb;
	LPVOID    lptstr;
	char* str = "";
	if (!OpenClipboard(nullptr))
	{
		printf("Error Opening ClipBoard");
	}
	hglb = GetClipboardData(CF_TEXT);
	if (hglb != NULL)
	{
		lptstr = GlobalLock(hglb);
		if (lptstr != NULL)
		{
			str = (char *)lptstr;
			GlobalUnlock(hglb);
		}
	}
	CloseClipboard();
	if (str == ""){}
	else{
cout << "\nCopied = " << str;
	cout << endl << "icount =" << icount;
	textlist.push_back(str);
	cout << "\nThe size = " << textlist.size();
	cout << endl << "the contents of textlist =";
	for (int i = 0; i != textlist.size(); i++)
	{
		cout << endl << i << " " << textlist.at(i);
	}
	}
	


}
BOOL clipbrdsetdata()
{

	str = textlist.at(icount);
	newchar = str.c_str();
	releaseclipboard = FALSE;
	const size_t len = strlen(newchar) + 1;
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
	memcpy(GlobalLock(hMem), newchar, len);
	GlobalUnlock(hMem);
	OpenClipboard(0);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hMem);
	CloseClipboard();
	releaseclipboard = TRUE;
	icount++;
	return true;
}