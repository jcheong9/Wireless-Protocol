#include <windows.h>
#include "Physical.h"
#include <tchar.h>
#include "DumbMenu.h"
#include "Application.h"

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
OPENFILENAME ofn;
// a another memory buffer to contain the file name
char szFile[1000];

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
	DWORD threadId;

	switch (Message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_COM1:
			if (data->hComm == NULL) {
				data->hComm = OpenPort((LPCWSTR) "COM1");
				ConfigurePort(hwnd, data->hComm, TEXT("COM1"));
				setMenuButton(hwnd, IDM_CONNECT, MF_ENABLED);
			}
			else {
				ConfigurePort(hwnd, data->hComm, TEXT("COM1"));
			}
			break;

		case IDM_SETTINGS:

			break;
		case IDM_CONNECT:
			if (data->connected == false) {
				data->connected = true;
				if (receiveThread == NULL && sendThread == NULL) {
					//sendThread = CreateThread(NULL, 0, ThreadSendProc, &data, 0, &threadId);
					//receiveThread = CreateThread(NULL, 0, ThreadReceiveProc, &data, 0, &threadId);
				}
					setMenuButton(hwnd, IDM_CONNECT, MF_GRAYED);
					setMenuButton(hwnd, IDM_DISCONNECT, MF_ENABLED);
			}
			break;
		case IDM_UPLOADFILE:
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = NULL;
			ofn.lpstrFile =  szFile;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = _TEXT("All\0*.*\0Text\0*.TXT\0");
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

			GetOpenFileNameA((LPOPENFILENAMEA) &ofn);


			MessageBox(NULL, ofn.lpstrFile, TEXT("File Name"), MB_OK);

			
			break;
		case IDM_DISCONNECT:
			if (data->connected == true) {
				data->connected = false;
					setMenuButton(hwnd, IDM_DISCONNECT, MF_GRAYED);
					setMenuButton(hwnd, IDM_CONNECT, MF_ENABLED);
			}
			break;
		case IDM_HELP:
			MessageBox(NULL, TEXT("1) Select \"Port Configuration\"\n2) Set your desired settings\n3) Click \"Connect\""), 
				TEXT("Help"), MB_OK);
			break;
		case IDM_EXIT:
			if (data->hComm) {
				CloseHandle(data->hComm);
			}
			PostQuitMessage(0);
		}
		break;
	case WM_CHAR:
		if (!data->connected) {
			break;
		}
		if (wParam == VK_ESCAPE) {
			MessageBox(NULL, TEXT("You have been disconnected!"), TEXT(""), MB_OK);
			data->connected = false;
			CloseHandle(data->hComm);
			data->hComm = NULL;
			setMenuButton(hwnd, IDM_CONNECT, MF_GRAYED);
			DrawMenuBar(hwnd);
			break;
		}
		Write(data->hComm, wParam);
		break;

	case WM_PAINT:		// Process a repaint message
		hdc = BeginPaint(hwnd, &paintstruct); // Acquire DC
		TextOut(hdc, 0, 0, str, strlen(str)); // output character
		EndPaint(hwnd, &paintstruct); // Release DC
		break;

	case WM_DESTROY:	// Terminate program
		if (data->hComm) {
			CloseHandle(data->hComm);
			delete data;
		}
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}
