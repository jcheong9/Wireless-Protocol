#include "Physical.h"
/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Physical.c - Contains RS-232 and system level flow control functions
--
--
-- PROGRAM: Dumb Terminal
--
-- FUNCTIONS:
--				HANDLE OpenPort(LPCWSTR lpszCommName)
--				int InitializePort(HANDLE hComm, COMMCONFIG cc, DWORD dwSize)
--				int Write(HANDLE hComm, TCHAR character)
--				DWORD WINAPI Read(LPVOID n)
--
-- DATE: September 30, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Tommy Chang
--
-- PROGRAMMER: Tommy Chang
--
-- NOTES: Provides access to the physical communications link,by initializing the port, and handling receiving and writing characters
----------------------------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: Bid
--
-- DATE: November29, 2019
--
-- REVISIONS: 
--
-- DESIGNER: Amir Kbah
--
-- PROGRAMMER: Amir Kbah
--
-- INTERFACE: HANDLE OpenPort(LPCWSTR lpszCommName)
--			  LPCWSTR lpszCommName: the name of the Com port to open
--
-- RETURNS: int
--
-- NOTES: Bids for trnsmission if a file is uploaded or awaits an ENQ if no file is uploaded.
--
----------------------------------------------------------------------------------------------------------------------*/

int Bid() {
	OVERLAPPED o1{ 0 };
	char str[2];
	str[1] = '\0';
	DWORD dwCommEvent;
	DWORD dwRead;
	char chRead[2];
	COMMTIMEOUTS CommTimeouts;
	DWORD CommEvent{ 0 };

	memset(&CommTimeouts, 0, sizeof(CommTimeouts));
	GetCommTimeouts(wpData->hComm, &CommTimeouts);
	CommTimeouts.ReadTotalTimeoutMultiplier = 5000;
	CommTimeouts.ReadTotalTimeoutConstant = 10000;
	SetCommTimeouts(wpData->hComm, &CommTimeouts);


	//while (wpData->status = IDLE) {
		OutputDebugString(_T("Bidding"));
		if (wpData->fileUploaded) {
			if (WriteFile(wpData->hComm, (LPCVOID)(SYN1 + ENQ), 2, NULL, &o1)) {
			}
			else {
			
			}
		}
		SetCommMask(wpData->hComm, EV_RXCHAR);
		if (WaitCommEvent(wpData->hComm, &CommEvent, 0)) {
			if (ReadFile(wpData->hComm, &chRead, 1, &dwRead, 0)) {
				OutputDebugString(_T("Received1"));

				//Timeout
				if (dwRead < 1)
				{
					//Timedout randomize timeout and go back
					CommTimeouts.ReadIntervalTimeout = randomizeTimeOut(500, 1000);
					OutputDebugString(_T("TO"));

					//continue;
				}
				//Received something here
				if (dwRead > 0) {
					OutputDebugString(_T("Received"));

					//check second byte in received frame if ENQ ignoring sync byte
					if (chRead[1] == ENQ) {
						if (WriteFile(wpData->hComm, (LPCVOID)(chRead[0] + ACK), 2, NULL, 0)) {
							wpData->status = RECEIVE_MODE;
						}
					}
					//check second byte in received frame if ACK ignoring sync byte
					else if (chRead[1] == ACK) {
						//TODO call send frame
						wpData->status = SEND_MODE;
					}
				}
			}
			else {
				OutputDebugString(_T("NOPE"));

			}
		}
	//}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: OpenPort
--
-- DATE: September 30, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Tommy Chang
--
-- PROGRAMMER: Tommy Chang
--
-- INTERFACE: HANDLE OpenPort(LPCWSTR lpszCommName)
--					LPCWSTR lpszCommName: the name of the Com port to open
--
-- RETURNS: HANDLE
--
-- NOTES: Opens the serial port "lpszCommName" and returns a handle to the port.
--
----------------------------------------------------------------------------------------------------------------------*/


HANDLE OpenPort(LPCWSTR lpszCommName) {
	HANDLE hComm;
	hComm = CreateFile((LPCSTR) lpszCommName, GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (hComm == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, TEXT("Error opening COM port, or COM port is already open"), TEXT(""), MB_OK);
	}
	return hComm;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: InitializePort
--
-- DATE: September 30, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Tommy Chang
--
-- PROGRAMMER: Tommy Chang
--
-- INTERFACE: InitializePort(HANDLE hComm, COMMCONFIG cc, DWORD dwSize)
--					HANDLE hComm: handle to the COM port to set the configurations
--					COMMCONFIG cc: COMMCONFIG object which is storing the user's parameters
--					DWORD dwSize: size of the COMMCONFIG object
--
-- RETURNS: int
--
-- NOTES: Initializes the port's, baud rate, parity, stop bits, and flow control with the user's parameters
--
----------------------------------------------------------------------------------------------------------------------*/

int InitializePort(HANDLE hComm, COMMCONFIG cc, DWORD dwSize) {
	if (!SetCommConfig(hComm, &cc, cc.dwSize)) {
		MessageBox(NULL, TEXT("There was an error setting your configurations"), TEXT(""), MB_OK);
		return 0;
	}
	return 1;
}



/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: Write
--
-- DATE: September 30, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Tommy Chang
--
-- PROGRAMMER: Tommy Chang
--
-- INTERFACE: int Write(HANDLE hComm, TCHAR character) 
--				HANDLE hComm: handle to the port to write
--				TCHAR character: character to write 
--
-- RETURNS: int
--
-- NOTES: Writes the WM_CHAR received from WndProc to the handle 
--
----------------------------------------------------------------------------------------------------------------------*/

int Write(HANDLE hComm, TCHAR character) {
	OVERLAPPED o1{ 0 };
	if (WriteFile(hComm, &character, 1, 0, &o1))
	{
		return 1;
	}
	return 0;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: Read
--
-- DATE: September 30, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Tommy Chang
--
-- PROGRAMMER: Tommy Chang
--
-- INTERFACE: Read(HANDLE hComm, char *str, DWORD nNumberofBytesToRead, LPDWORD lpNumberofBytesRead, LPOVERLAPPED o1)
--
--					HANDLE hComm: handle to the port to read from
--					char *str: buffer to store the character
--					DWORD nNumberofBytesToRead: number of bytes to read
--					LPDWORD lpNumberofBytesRead: number of bytes actually read (NULL)
--					LPOVERLAPPED o1: overlapped structure
--
-- RETURNS: int
--
-- NOTES: Reads nNumberofBytestoRead (currently 1 byte, for one character) from the handle, and stores it in the str buffer.
--
----------------------------------------------------------------------------------------------------------------------*/

int Read(HANDLE hComm, char* str, DWORD nNumberofBytesToRead, LPDWORD lpNumberofBytesRead, LPOVERLAPPED o1) {
	if (ReadFile(hComm, str, nNumberofBytesToRead, NULL, o1)) {
		return 1;
	}
	return 0;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: ReadFunc
--
-- DATE: September 30, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Tommy Chang
--
-- PROGRAMMER: Tommy Chang
--
-- INTERFACE: DWORD WINAPI ReadFunc(LPVOID n)
--				LPVOID n: structure passed in to the thread
--
-- RETURNS: DWORD
--
-- NOTES: Function that is executed by a thread created when user enters "connect" mode for the first time.
-- Stays in an infinite loop, and monitors if a character was received and placed in the input buffer.
-- If there is a character in the input buffer, receives the character into the str buffer by calling Read 
----------------------------------------------------------------------------------------------------------------------*/

DWORD WINAPI ThreadSendProc(LPVOID n) {
	OVERLAPPED o1{ 0 };
	char str[2];
	str[1] = '\0';
	DWORD CommEvent{ 0 };
	static unsigned x = 0;
	static unsigned y = 0;
	SetCommMask(wpData->hComm, EV_RXCHAR); // event-driven
	while (wpData->hComm != NULL) {
		if (WaitCommEvent(wpData->hComm, &CommEvent, 0)) { 
			if (Read(wpData->hComm, str, 1, NULL, &o1)) { 
				wpData->hdc = GetDC(wpData->hwnd); 
				printToWindow(wpData->hwnd, wpData->hdc, str, &x, &y); // print character
			}
		}
	}
	return 1;
}

DWORD WINAPI ThreadReceiveProc(LPVOID n) {
	OVERLAPPED o1{ 0 };
	char str[2];
	str[1] = '\0';
	DWORD CommEvent{ 0 };
	static unsigned x = 0;
	static unsigned y = 0;
	SetCommMask(wpData->hComm, EV_RXCHAR); // event-driven
	while (wpData->hComm != NULL) {
		if (WaitCommEvent(wpData->hComm, &CommEvent, 0)) {
			if (Read(wpData->hComm, str, 1, NULL, &o1)) {
				wpData->hdc = GetDC(wpData->hwnd);
				printToWindow(wpData->hwnd, wpData->hdc, str, &x, &y); // print character
			}
		}
	}
	return 1;
}

int randomizeTimeOut(int range_min, int range_max){
	return (double)rand() / (RAND_MAX + 1) * (range_max - range_min) + range_min;
}