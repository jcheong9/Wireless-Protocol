#include <windows.h>
#include "Session.h"
#include <tchar.h>
#include "Application.h"

#pragma warning (disable: 4096)

/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Application.c - An application that will act as a dumb terminal
-- Provides a high-level GUI with menu buttons for users to access and use communication functions
--
--
-- PROGRAM: Dumb Terminal
--
-- FUNCTIONS:
--				int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
--				LPSTR lspszCmdParam, int nCmdShow)
--				void setMenuButton(HWND hwnd, UINT uIDEnableItem, UINT uEnable)
--				void printToWindow(HWND hwnd, HDC hdc, char* str, unsigned int* x, unsigned int* y)
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
-- Displays Menu items to configure port settings, enter connect mode, 
-- view a help message, and exit the application.
----------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WinMain
--
-- DATE: September 30, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Tommy Chang
--
-- PROGRAMMER: Tommy Chang
--
-- INTERFACE: int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
	LPSTR lspszCmdParam, int nCmdShow)
--				HINSTANCE hINST: A handle to the current instance of the application.
--				HINSTANCE hprevInstance: A handle to the previous instance of the application. 
										This parameter is always NULL.
--				LPSTR lspszCmdParam: The command line for the application, excluding the program name
--				int nCmdShow: Specifies how the application windows should be displayed
--
-- RETURNS: int
--
-- NOTES:
-- This is the user-provided entry point for a graphical Windows-based application
-- Registers the Windows Class and displays the Window
----------------------------------------------------------------------------------------------------------------------*/

Data* data = new Data();
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
	LPSTR lspszCmdParam, int nCmdShow)
{
	data->connected = false;
	data->hComm = NULL;
	static TCHAR Name[] = TEXT("Dumb Terminal");
	MSG Msg{ 0 };
	WNDCLASSEX Wcl;

	Wcl.cbSize = sizeof(WNDCLASSEX);
	Wcl.style = CS_HREDRAW | CS_VREDRAW;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // large icon 
	Wcl.hIconSm = NULL; // use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);  // cursor style

	Wcl.lpfnWndProc = WndProc;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //white background
	Wcl.lpszClassName = Name;
	Wcl.lpszMenuName = TEXT("TerminalMenu"); // The menu Class
	Wcl.cbClsExtra = 0;      // no extra memory needed
	Wcl.cbWndExtra = 0;

	if (!RegisterClassEx(&Wcl))
		return 0;

	data->hwnd = CreateWindow(Name, Name, WS_OVERLAPPEDWINDOW, 10, 10,
		610, 400, NULL, NULL, hInst, NULL);
	//setMenuButton(data->hwnd, IDM_CONNECT, MF_GRAYED);
	setMenuButton(data->hwnd, IDM_DISCONNECT, MF_GRAYED);

	ShowWindow(data->hwnd, nCmdShow);
	UpdateWindow(data->hwnd);


	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: setMenuButton
--
-- DATE: September 30, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Tommy Chang
--
-- PROGRAMMER: Tommy Chang
--
-- INTERFACE: void setMenuButton(HWND hwnd, UINT uIDEnableItem, UINT uEnable)
--					HWND hwnd - handle to the window
--					UINT uIDEnableItem - ID of the menu item
--					UINT uEnable - New status of the menu item
-- RETURNS: void
--
-- NOTES:
-- This function sets enables, disables, or grays the specified menu item according to the parameters
----------------------------------------------------------------------------------------------------------------------*/

void setMenuButton(HWND hwnd, UINT uIDEnableItem, UINT uEnable) {
	HMENU hMenu = GetMenu(hwnd);
	EnableMenuItem(hMenu, uIDEnableItem, uEnable);
	DrawMenuBar(hwnd);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: printToWindow
--
-- DATE: September 30, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Tommy Chang
--
-- PROGRAMMER: Tommy Chang
--
-- INTERFACE: void printToWindow(HWND hwnd, HDC hdc, char* str, unsigned int* x, unsigned int* y)
--				HWND hwnd: Dandle to the window
--				HDC hdc: Device Context of the hwnd
--				char* str: Buffer to be written to the window
--				unsigned int* x: Location of the x-coordinate to write the buffer to the window
--				unsigned int* y: Location of the x-coordinate to write the buffer to the window
-- RETURNS: void
--
-- NOTES:
-- This function prints the character stored in the str buffer to a particular x and y coordinate of the window.
----------------------------------------------------------------------------------------------------------------------*/


void printToWindow(HWND hwnd, HDC hdc, char* str, unsigned int* x, unsigned int* y)
{
	TextOut(hdc, *x, *y,  str, strlen(str));
	SIZE size;
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);
	GetTextExtentPoint32(data->hdc,  str, strlen(str), &size);
	*x += size.cx; // increment the screen x-coordinate
	if (*x >= 580 && *x <= 600) { // move down one line if we're near the end of the window
		*x = 0;
		*y = *y + tm.tmHeight + tm.tmExternalLeading;
	}
	ReleaseDC(hwnd, hdc);
}
