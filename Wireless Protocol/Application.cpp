
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

Data* wpData = new Data();

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
	LPSTR lspszCmdParam, int nCmdShow)
{
	wpData->currentSyncByte = 0x00;
	wpData->connected = false;
	wpData->status = IDLE;
	wpData->hComm = NULL;
	wpData->sentdEnq = false;
	wpData->fileUploaded = false;
	static TCHAR Name[] = TEXT("Wireless Protocol");
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

	wpData->hwnd = CreateWindow(Name, Name, WS_OVERLAPPEDWINDOW, (GetSystemMetrics(0) / 2 - 500), (GetSystemMetrics(1) / 2 - 400),
		1000, 800, NULL, NULL, hInst, NULL);
	setMenuButton(wpData->hwnd, IDM_CONNECT, MF_GRAYED);
	setMenuButton(wpData->hwnd, IDM_DISCONNECT, MF_GRAYED);
	
	//This is here to test a text box 
	//HWND hWndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Edit"), TEXT("test"),
	//	WS_CHILD | WS_VISIBLE , 0, 0, 1000, 800, wpData->hwnd, NULL, NULL, NULL);


	ShowWindow(wpData->hwnd, nCmdShow);
	UpdateWindow(wpData->hwnd);

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
	TextOut(wpData->hdc, *x,  *y,  str, strlen(str));
	SIZE size;
	TEXTMETRIC tm;
	GetTextMetrics(wpData->hdc, &tm);
	GetTextExtentPoint32(wpData->hdc,  str, strlen(str), &size);
	*x += size.cx; // increment the screen x-coordinate
	if (*x >= 580 && *x <= 600) { // move down one line if we're near the end of the window
		*x = 0;
		*y = *y + tm.tmHeight + tm.tmExternalLeading;
	}
	ReleaseDC(wpData->hwnd, wpData->hdc);
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
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	HANDLE readThread = NULL;
	DWORD threadId;
	LPCSTR portNumber = (LPCSTR)"COM1";




	switch (Message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_COM1:
			if (wpData->hComm == NULL) {
				wpData->hComm = OpenPort((LPCWSTR) portNumber);
				ConfigPort(wpData->hwnd, wpData->hComm, portNumber);
				setMenuButton(wpData->hwnd, IDM_CONNECT, MF_ENABLED);

			}
			else {
				ConfigPort(wpData->hwnd, wpData->hComm, portNumber);
			}

			break;

		case IDM_SETTINGS:
			//printToWindow(wpData->hwnd, wpData->hdc, s, &xC, &yC);
			break;
		case IDM_CONNECT:

				sendThread = CreateThread(NULL, 0, ThreadSendProc, &wpData, 0, &threadId);
			if (wpData->connected == false) {
				wpData->connected = true;
				wpData->hdc = GetDC(wpData->hwnd);
				if (readThread == NULL) {
					readThread = CreateThread(NULL, 0, ThreadReceiveProc, &wpData, 0, &threadId);
					setMenuButton(wpData->hwnd, IDM_CONNECT, MF_GRAYED);
					setMenuButton(wpData->hwnd, IDM_DISCONNECT, MF_ENABLED);

				}
			}

			break;

		case IDM_UPLOADFILE:
			if (addFile(ofn)) {
				if (packetizeFile(ofn.lpstrFile) != 1) {
					MessageBox(NULL, TEXT("Error occured while trying to packetize the file."), TEXT("ERROR | DataLink Layer"), MB_OK);
					
				}else {
					wpData->fileUploaded = true;
					vector<char*> a = dataLink->uploadedFrames;
					int b = a.size();
				}
				//if you want to test check frame function, uncomment the codes below
				//else {
				//	dataLink->incomingFrames.push_back(dataLink->uploadedFrames.at(0));
				//	checkFrame();
				//}
			}
			else {
				MessageBox(NULL, TEXT("Error occured while trying to select the file."), TEXT("ERROR | Session Layer"), MB_OK);
			}

			MessageBox(NULL, ofn.lpstrFile, TEXT("File Name"), MB_OK);
			break;

		case IDM_DISCONNECT:
			wpData->connected = false;
			wpData->status = COMMAND_MODE;
			setMenuButton(wpData->hwnd, IDM_CONNECT, MF_ENABLED );
			setMenuButton(wpData->hwnd, IDM_DISCONNECT, MF_GRAYED);
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
		//Write(wpData->hComm, wParam);
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