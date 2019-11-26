//#include <windows.h>
#include "Session.h"
#include <tchar.h>
char testr[2];
int ss = 0;

char szFile[1000];
char str[80] = "";
HDC hdc;
PAINTSTRUCT paintstruct;
OVERLAPPED o1 = { 0 };
/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Session.c - A Windows application that will act as a dumb terminal
-- that writes to a serial port and reads from a serial port and displays it on the screen
--
-- PROGRAM: Dumb Terminal
--
-- FUNCTIONS:
-- void ConfigurePort(HWND hwnd, HANDLE hComm, LPCWSTR lpszCommName)
-- LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
--			
--
--
--
--
-- DATE: September 30, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Tommy Chang
--
-- PROGRAMMER: Tommy Chang
--
-- NOTES:
-- 
----------------------------------------------------------------------------------------------------------------------*/

boolean addFile(OPENFILENAME &ofn) {
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = _TEXT("All\0*.*\0Text\0*.TXT\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	return GetOpenFileNameA((LPOPENFILENAMEA)&ofn);
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: ConfigurePort
--
-- DATE: September 30, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Tommy Chang
--
-- PROGRAMMER: Tommy Chang
--
-- INTERFACE: void ConfigurePort(HWND hwnd, HANDLE hComm, LPCWSTR lpszCommName)
--				HWND hwnd: Handle to the window
--				HANDLE hComm: Handle to the serial port to configure
--				LPCWSTR lpszCommName: Name of the serial port, used to pull up the commconfig dialog
-- RETURNS: int
--
-- NOTES:
-- 
----------------------------------------------------------------------------------------------------------------------*/


int ConfigPort(HWND hwnd, HANDLE hComm, LPCSTR lpszCommName) {
	COMMCONFIG cc;
	cc.dwSize = sizeof(COMMCONFIG);
	cc.wVersion = 0x100;
	GetCommConfig(hComm, &cc, &cc.dwSize);
	CommConfigDialog(lpszCommName, hwnd, &cc); // Open dialogue box for user
	if (InitializePort(hComm, cc, cc.dwSize)) {
		return 1;
	}
	return 0;
}

void Connect(HANDLE receiveThread, HANDLE sendThread, HWND hwnd) {
	DWORD threadSendId;
	DWORD threadReceiveId;
	if (wpData->connected == false) {
		wpData->connected = true;
		if (receiveThread == NULL && sendThread == NULL) {
			sendThread = CreateThread(NULL, 0, ThreadSendProc, &wpData, 0, &threadSendId);
			receiveThread = CreateThread(NULL, 0, ThreadReceiveProc, &wpData, 0, &threadReceiveId);
		}
		setMenuButton(hwnd, IDM_CONNECT, MF_GRAYED);
		setMenuButton(hwnd, IDM_DISCONNECT, MF_ENABLED);
	}
}

void Disconnect(HWND hwnd) {
	if (wpData->connected == true) {
		wpData->connected = false;
		setMenuButton(hwnd, IDM_DISCONNECT, MF_GRAYED);
		setMenuButton(hwnd, IDM_CONNECT, MF_ENABLED);
	}
}



