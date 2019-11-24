//#include <windows.h>
#include "Physical.h"
#include "DumbMenu.h"
#include "Application.h"
#include "DataLink.h"
OPENFILENAME ofn;
char szFile[1000];

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

boolean addFile() {
	ZeroMemory(&ofn, sizeof(ofn));
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

	GetOpenFileNameA((LPOPENFILENAMEA)&ofn);
	return true;
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


int ConfigurePort(HWND hwnd, HANDLE hComm, LPCSTR lpszCommName) {
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
			//sendThread = CreateThread(NULL, 0, ThreadSendProc, &data, 0, &threadSendId);
			//receiveThread = CreateThread(NULL, 0, ThreadReceiveProc, &data, 0, &threadReceiveId);
		}
		setMenuButton(hwnd, IDM_CONNECT, MF_GRAYED);
		setMenuButton(hwnd, IDM_DISCONNECT, MF_ENABLED);
	}
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WndProc
--
-- DATE: September 30, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Tommy Chang
--
-- PROGRAMMER: Tommy Chang
--
-- INTERFACE: LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
--				HWND hwnd: Handle to the window
--				UINT Message: Event message received
--				WPARAM wParam: contains the virtual key code that identifies the key that was pressed. 
--				LPARAM lParam: contains more information about the message
--
-- RETURNS: LRESULT
--
-- NOTES:
-- This is the default function that is called when a message is dispatched.
----------------------------------------------------------------------------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	char str[80] = "";
	HDC hdc;
	PAINTSTRUCT paintstruct;
	OVERLAPPED o1 = { 0 };
	HANDLE receiveThread = NULL;
	HANDLE sendThread = NULL;


	switch (Message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_COM1:
			if (wpData->hComm == NULL) {
				wpData->hComm = OpenPort((LPCWSTR) "COM1");
				ConfigurePort(hwnd, wpData->hComm, TEXT("COM1"));
				setMenuButton(hwnd, IDM_CONNECT, MF_ENABLED);
			}
			else {
				ConfigurePort(hwnd, wpData->hComm, TEXT("COM1"));
			}
			break;

		case IDM_SETTINGS:

			break;
		case IDM_CONNECT:

			Connect( receiveThread,  sendThread, hwnd);

			break;
		case IDM_UPLOADFILE:

			addFile();
			OutputDebugStringA("HelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHello");
			for (int i = 0; i < dataLink->uploadedFrames->size(); i++) {
				char* frame = dataLink->uploadedFrames->at(i) + 0x00;
				while (frame != nullptr) {
					OutputDebugStringA(frame);
				}
				OutputDebugStringA("woiejfo");
			}

			MessageBox(NULL, ofn.lpstrFile, TEXT("File Name"), MB_OK);

			
			break;
		case IDM_DISCONNECT:
			if (wpData->connected == true) {
				wpData->connected = false;
					setMenuButton(hwnd, IDM_DISCONNECT, MF_GRAYED);
					setMenuButton(hwnd, IDM_CONNECT, MF_ENABLED);
			}
			break;
		case IDM_HELP:
			MessageBox(NULL, TEXT("1) Select \"Port Configuration\"\n2) Set your desired settings\n3) Click \"Connect\""), 
				TEXT("Help"), MB_OK);
			break;
		case IDM_EXIT:
			if (wpData->hComm) {
				CloseHandle(wpData->hComm);
			}
			PostQuitMessage(0);
		}
		break;
	case WM_CHAR:
		if (!wpData->connected) {
			break;
		}
		if (wParam == VK_ESCAPE) {
			MessageBox(NULL, TEXT("You have been disconnected!"), TEXT(""), MB_OK);
			wpData->connected = false;
			CloseHandle(wpData->hComm);
			wpData->hComm = NULL;
			setMenuButton(hwnd, IDM_CONNECT, MF_GRAYED);
			DrawMenuBar(hwnd);
			break;
		}
		Write(wpData->hComm, wParam);
		break;

	case WM_PAINT:		// Process a repaint message
		hdc = BeginPaint(hwnd, &paintstruct); // Acquire DC
		TextOut(hdc, 0, 0, str, strlen(str)); // output character
		EndPaint(hwnd, &paintstruct); // Release DC
		break;

	case WM_DESTROY:	// Terminate program
		if (wpData->hComm) {
			CloseHandle(wpData->hComm);
			delete wpData;
		}
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}
