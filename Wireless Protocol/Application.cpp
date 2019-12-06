
#include "Application.h"
#include <stdio.h>
#pragma warning (disable: 4096)

/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Application.cpp - An application that will act as a Wireless Protocol interface for half duplex links
-- Provides a high-level GUI with menu buttons for users to access and use communication functions
--
--
-- PROGRAM: Wireless Protocol
--
-- FUNCTIONS:
--				int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
--				LPSTR lspszCmdParam, int nCmdShow)
--				void setMenuButton(HWND hwnd, UINT uIDEnableItem, UINT uEnable)
--				void ToWindow(HWND hwnd, HDC hdc, char* str, unsigned int* x, unsigned int* y)
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

static HWND hList = NULL;  // List View identifier
LVCOLUMN LvCol;
LVITEM LvItem;
char Temp[255] = "";

//columns for send and receive
LVCOLUMN lcl;
LVCOLUMN rcl;

//column counter 
static int col = 0;

//Textbox handlers for send and receive
HWND textHwnd;
HWND textHwndRx;

//Handlers for the tables for send and receive
HWND hWndListView;
HWND hWndListViewRx;
char* buff;
char* buffNewText;
char* newBuffer;
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
	OutputDebugString(_T("/n....IDLE Set from win main.../n"));
	wpData->status = COMMAND_MODE;
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

	//Creates and centers the window
	wpData->hwnd = CreateWindow(Name, Name, WS_OVERLAPPEDWINDOW, (GetSystemMetrics(0) / 2 - 609), (GetSystemMetrics(1) / 2 - 425),
		1218, 850, NULL, NULL, hInst, NULL);
	setMenuButton(wpData->hwnd, IDM_CONNECT, MF_GRAYED);
	setMenuButton(wpData->hwnd, IDM_DISCONNECT, MF_GRAYED);

	prepWindow(hInst);

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
-- DATE: December 5, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: void printToWindowsNew(char* str)
--				
-- RETURNS: void
--
-- NOTES:
-- This function prints the character stored in the str buffer to a particular x and y coordinate of the window.
----------------------------------------------------------------------------------------------------------------------*/

//This takes whole chunks of chars (char*) and appends them to the screen.
void printToWindowsNew(char* str, int window)
{

	char incomingBuffer[1019];
	if (window == 0) {
		for (int i = 2; i < 1018; ++i) {
			incomingBuffer[i] = str[i];
		}

		incomingBuffer[1018] = '\0';
		// get new length to determine buffer size
		int newIn = lstrlen(incomingBuffer);
		int newLength = GetWindowTextLength(textHwnd) + lstrlen(incomingBuffer) + 2;
		// create buffer to hold current and new text
		TCHAR* newBuffer = (TCHAR*)GlobalAlloc(GPTR, newLength * sizeof(TCHAR));
		int newbuf = lstrlen(newBuffer);
		if (!newBuffer) return;

		// get existing text from edit control and put into buffer
		GetWindowText(textHwnd, newBuffer, newLength);
		int size = sizeof(newBuffer);
		// append the newText to the buffer
		_tcscat_s(newBuffer, newLength, incomingBuffer);
		//newBuffer[newLength - 1] = '\0';
		int bufleng = sizeof(newBuffer);
		// Set the text in the edit control
		SetWindowText(textHwnd, newBuffer);
	}
	else {
		for (int i = 2; i < 1018; ++i) {
			incomingBuffer[i] = str[i];
		}

		incomingBuffer[1018] = '\0';
		// get new length to determine buffer size
		int newIn = lstrlen(incomingBuffer);
		int newLength = GetWindowTextLength(textHwndRx) + lstrlen(incomingBuffer) + 2;
		// create buffer to hold current and new text
		TCHAR* newBuffer = (TCHAR*)GlobalAlloc(GPTR, newLength * sizeof(TCHAR));
		int newbuf = lstrlen(newBuffer);
		if (!newBuffer) return;

		// get existing text from edit control and put into buffer
		GetWindowText(textHwndRx, newBuffer, newLength);
		int size = sizeof(newBuffer);
		// append the newText to the buffer
		_tcscat_s(newBuffer, newLength, incomingBuffer);
		//newBuffer[newLength - 1] = '\0';
		int bufleng = sizeof(newBuffer);
		// Set the text in the edit control
		SetWindowText(textHwndRx, newBuffer);
	}

	// free the buffer
	GlobalFree(newBuffer);

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
				wpData->hComm = OpenPort((LPCWSTR)portNumber);
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

			if (wpData->connected == false) {
				wpData->connected = true;
				wpData->status = IDLE;
				wpData->hdc = GetDC(wpData->hwnd);
				if (readThread == NULL) {
					sendThread = CreateThread(NULL, 0, ThreadSendProc, &wpData, 0, &threadId);
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

				}
				else {
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

			//MessageBox(NULL, ofn.lpstrFile, TEXT("File Name"), MB_OK);
			break;

		case IDM_DISCONNECT:
			wpData->connected = false;
			wpData->fileUploaded = false;
			wpData->status = COMMAND_MODE;
			wpData->status = false;
			wpData->receivedREQ = FALSE;
			setMenuButton(wpData->hwnd, IDM_CONNECT, MF_ENABLED);
			setMenuButton(wpData->hwnd, IDM_DISCONNECT, MF_GRAYED);
			break;

		case IDM_HELP:
			//MessageBox(NULL, TEXT("1) Select \"Port Configuration\"\n2) Set your desired settings\n3) Click \"Connect\""),
			//TEXT("Help"), MB_OK);
			//printToWindowsNew((char*)"Two before narrow not relied how except moment myself. Dejection assurance mrs led certainly. So gate at no only none open. Betrayed at properly it of graceful on. Dinner abroad am depart ye turned hearts as me wished. Therefore allowance too perfectly gentleman supposing man his now. Families goodness all eat out bed steepest servants. Explained the incommode sir improving northward immediate eat. Man denoting received you sex possible you. Shew park own loud son door less yet.");
			break;

		case IDM_EXIT:
			if (wpData->hComm) {
				CloseHandle(wpData->hComm);
			}
			PostQuitMessage(0);
		}
		break;

	case WM_PAINT:		// Process a repaint message
		hdc = BeginPaint(hwnd, &paintstruct); // Acquire DC
		TextOut(hdc, 0, 0, str, strlen(str)); // output character
		EndPaint(hwnd, &paintstruct); // Release DC
		break;

	case WM_DESTROY:	// Terminate program

		if (wpData->hComm) {
			CloseHandle(wpData->hComm);
			//delete wpData;
		}
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: InitListViewColumns
--
-- DATE: December 3, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: BOOL InitListViewColumns(HWND hWndListView, HINSTANCE hInst, LVCOLUMN cl, char* colName)
--
-- RETURNS: BOOL
--
-- NOTES:
-- This is the default function that is called when a message is dispatched.
----------------------------------------------------------------------------------------------------------------------*/
BOOL InitListViewColumns(HWND hWndListView, HINSTANCE hInst, LVCOLUMN cl, char* colName)
{
	char szText[256];     // Temporary buffer.
	LVCOLUMN lvc;
	int iCol = col;

	// Initialize the LVCOLUMN structure.
	// The mask specifies that the format, width, text,
	// and subitem members of the structure are valid.
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;


	lvc.iSubItem = col;
	lvc.pszText = (LPSTR)colName;
	lvc.cx = 150;               // Width of column in pixels.
	lvc.fmt = LVCFMT_LEFT;  // center-aligned column.


	// Load the names of the column headings from the string resources.
	LoadString(hInst,
		UINT(colName + iCol),
		szText,
		sizeof(szText) / sizeof(szText[0]));

	// Insert the columns into the list view.
	if (ListView_InsertColumn(hWndListView, iCol, &lvc) == -1)
		return FALSE;

	if (col == 1)
		col = 0;
	else
		col++;

	return TRUE;
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: addColumns
--
-- DATE: December 3, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: void addColumns(HWND hwndLV, LVITEM* lvItem)
--
-- RETURNS: void
--
-- NOTES:
-- This is the default function that is called when a message is dispatched.
----------------------------------------------------------------------------------------------------------------------*/
void addColumns(HWND hwndLV, LVITEM* lvItem) {
	LVITEM lvI;

	// Initialize LVITEM members that are common to all items.
	lvI.pszText = LPSTR_TEXTCALLBACK; // Sends an LVN_GETDISPINFO message.
	lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
	lvI.stateMask = 0;
	lvI.iSubItem = 0;
	lvI.state = 0;

	// Initialize LVITEM members that are different for each item.
	for (int index = 0; index < 3; index++)
	{
		lvI.iItem = index;
		lvI.iImage = index;

		// Insert items into the list.
		ListView_InsertItem(hwndLV, &lvI);
	}

	lvItem = &lvI;
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: prepWindow
--
-- DATE: December 3, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: void prepWindow(HINSTANCE hInst)
--
-- RETURNS: void
--
-- NOTES:
-- This is the default function that is called when a message is dispatched.
----------------------------------------------------------------------------------------------------------------------*/
void prepWindow(HINSTANCE hInst) {
	/*
	Send section
	*/

	HWND textHwndLabel = CreateWindow("STATIC", "Send",
		WS_VISIBLE | WS_CHILD | SS_LEFT | ES_READONLY,
		0, 0, 900, 20, wpData->hwnd, NULL, hInst, NULL);

	textHwnd = CreateWindow("EDIT", "",
		WS_VISIBLE | WS_CHILD | SS_LEFT | ES_MULTILINE | WS_VSCROLL | ES_READONLY,
		0, 20, 900, 370, wpData->hwnd, NULL, hInst, NULL);

	//Send stats table
	hWndListView = CreateWindow(WC_LISTVIEW, (LPCSTR)L"", WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_VISIBLE,
		900, 0, 300, 200, wpData->hwnd, NULL, hInst, NULL);

	InitListViewColumns(hWndListView, hInst, lcl, (LPSTR)"Send Criteria");

	LVITEM* lv = new LVITEM();
	addColumns(hWndListView, lv);

	TCHAR buf[3];
	TCHAR bufACK[3];
	TCHAR bufREQ[3];


	ListView_SetItemText(hWndListView, 0, 0, (LPSTR)"Frames Sent");
	ListView_SetItemText(hWndListView, 1, 0, (LPSTR)"ACKs Sent");
	ListView_SetItemText(hWndListView, 2, 0, (LPSTR)"REQs Sent");

	InitListViewColumns(hWndListView, hInst, lcl, (LPSTR)"Send Statistics");


	ListView_SetItemText(hWndListView, 0, 1, (LPSTR)"0");
	ListView_SetItemText(hWndListView, 1, 1, (LPSTR)"0");
	ListView_SetItemText(hWndListView, 2, 1, (LPSTR)"0");

	/*
	Receive section
	*/

	HWND textHwndRxLabel = CreateWindow("STATIC", "Receive",
		WS_VISIBLE | WS_CHILD | SS_LEFT | ES_READONLY,
		0, 400, 900, 20, wpData->hwnd, NULL, hInst, NULL);

	textHwndRx = CreateWindow("EDIT", "",
		WS_VISIBLE | WS_CHILD | SS_LEFT | ES_MULTILINE | WS_VSCROLL | ES_READONLY,
		0, 420, 900, 370, wpData->hwnd, NULL, hInst, NULL);

	//Receive stats table
	hWndListViewRx = CreateWindow(WC_LISTVIEW, (LPCSTR)L"", WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_VISIBLE,
		900, 400, 300, 200, wpData->hwnd, NULL, hInst, NULL);

	InitListViewColumns(hWndListViewRx, hInst, rcl, (LPSTR)"Receive Criteria");

	addColumns(hWndListViewRx, lv);
	ListView_SetItemText(hWndListViewRx, 0, 0, (LPSTR)"Frames Received");
	ListView_SetItemText(hWndListViewRx, 1, 0, (LPSTR)"ACKs Received");
	ListView_SetItemText(hWndListViewRx, 2, 0, (LPSTR)"REQs Received");

	_stprintf_s(buf, _T("%d"), wpData->countFramesReceive);
	_stprintf_s(bufACK, _T("%d"), wpData->countAckReceive);
	_stprintf_s(bufREQ, _T("%d"), wpData->countReqReceive);
	InitListViewColumns(hWndListViewRx, hInst, rcl, (LPSTR)"Receive Statistics");
	ListView_SetItemText(hWndListViewRx, 0, 1, (LPSTR)"0");
	ListView_SetItemText(hWndListViewRx, 1, 1, (LPSTR)"0");
	ListView_SetItemText(hWndListViewRx, 2, 1, (LPSTR)"0");
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: updateStats
--
-- DATE: December 3, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: void updateStats(LPSTR newValue, int rowPosition)
--
-- RETURNS: void
--
-- NOTES:
-- This is the default function that is called when a message is dispatched.
----------------------------------------------------------------------------------------------------------------------*/
void updateStats(LPSTR newValue, int rowPosition) {
	switch (rowPosition) {
	case (10):
		ListView_SetItemText(hWndListView, 0, 1, (LPSTR)newValue);
		break;
	case (11):
		ListView_SetItemText(hWndListView, 1, 1, (LPSTR)newValue);

		break;
	case (12):
		ListView_SetItemText(hWndListView, 2, 1, (LPSTR)newValue);
		break;
	case (20):
		ListView_SetItemText(hWndListViewRx, 0, 1, (LPSTR)newValue);
		break;
	case (21):
		ListView_SetItemText(hWndListViewRx, 1, 1, (LPSTR)newValue);
		break;
	case (22):
		ListView_SetItemText(hWndListViewRx, 2, 1, (LPSTR)newValue);
		break;
	}
}