//#include <windows.h>
#include "Session.h"
char testr[2];
int ss = 0;

char szFile[1000];
char str[80] = "";
HDC hdc;
PAINTSTRUCT paintstruct;
OVERLAPPED o1 = { 0 };
/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Session.c - A Windows application that will act as a Wireless Protocol
-- that writes to a serial port and reads from a serial port and displays it on the screen
--
-- PROGRAM: Wireless Protocol
--
-- FUNCTIONS:
-- int ConfigPort(HWND hwnd, HANDLE hComm, LPCSTR lpszCommName);
-- boolean addFile(OPENFILENAME &ofn);
-- void prepareTransmission();
--			
--
-- DATE: December 5, 2019
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

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: addFile
--
-- DATE: December 5, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: void ConfigurePort(OPENFILENAME &ofn)
--			  OPENFILENAME &ofn - A pointer to the file path for the file to be opened
-- RETURNS: boolean
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
-- DATE: December 5, 2019
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


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: Connect
--
-- DATE: December 5, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Tommy Chang
--
-- PROGRAMMER: Tommy Chang
--
-- INTERFACE: void Connect(HANDLE receiveThread, HANDLE sendThread, HWND hwnd)
--				HWND receiveThread: Handle to the receiving thread
--				HANDLE sendThread: Handle to the serial port to configure
--				HWND hwnd: handle to the application window
-- RETURNS: void
--
-- NOTES: Creates the threads, changes the UI to signiffy that the application is now in Connect Mode
--
----------------------------------------------------------------------------------------------------------------------*/

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


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: Disconnect
--
-- DATE: December 5, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Tommy Chang
--
-- PROGRAMMER: Tommy Chang
--
-- INTERFACE: void Disconnect(HWND hwnd) 
--			  HWND hwnd: handle to the application window
--
-- RETURNS: void
--
-- NOTES: Kills the threads, changes the UI to signiffy that the application is now in disconnected
--
----------------------------------------------------------------------------------------------------------------------*/
void Disconnect(HWND hwnd) {
	if (wpData->connected == true) {
		wpData->connected = false;
		setMenuButton(hwnd, IDM_DISCONNECT, MF_GRAYED);
		setMenuButton(hwnd, IDM_CONNECT, MF_ENABLED);
	}
}




