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

HANDLE ReceiveModeEvent;
HANDLE responseWaitEvent = CreateEvent(NULL, TRUE, TRUE, (LPTSTR)_T("ACK"));
HANDLE ackEvent;
HANDLE eotEvent;
int REQCounter = 0;

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
	char chRead[2];

	DWORD CommEvent{ 0 };
	DWORD timeoutToReceive = 500;
	DWORD randomizedTO = 0;

	ReceiveModeEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	HANDLE dummy = CreateEvent(NULL, TRUE, FALSE, NULL);


	OutputDebugString(_T("Bidding"));
	if (wpData->fileUploaded && wpData->status == IDLE) {
		if (WriteFile(wpData->hComm, (LPCVOID)(SYN0 + ENQ), 2, NULL, &o1)) {
			if (WaitForSingleObject(ReceiveModeEvent, timeoutToReceive) == WAIT_OBJECT_0) {
				wpData->sentdEnq = true;
				OutputDebugString(_T("Received"));
				wpData->status = SEND_MODE;
				//continue;
			}
			//timeout
			else {
				wpData->sentdEnq = false;
				randomizedTO = randomizeTimeOut(500, 1000);
				OutputDebugString(_T("Timeout2"));
				WaitForSingleObject(dummy, randomizedTO) == WAIT_OBJECT_0;
			}
		}
	}


	return 1;
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
-- FUNCTION: sendFrame
--
-- DATE: November 11, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Tommy Chang
--
-- PROGRAMMER: Jameson Cheong
--
-- INTERFACE: int sendFrame(HANDLE hComm, char* frame, DWORD nBytesToRead)
--
--
-- RETURNS: int 1 successfully sent; int 0 failed to sent.
--
-- NOTES: Writes the frame received from datalink. Check end of transmition and set status to IDLE
--
----------------------------------------------------------------------------------------------------------------------*/

int sendFrame(HANDLE hComm, char* frame, DWORD nBytesToRead) {
	DWORD CommEvent{ 0 };
	OVERLAPPED o1{ 0 };

	char frame11[FRAME_SIZE];
	strncpy_s(frame11, frame, FRAME_SIZE);
	//running completing asynchronously return false
	if (!WriteFile(hComm, &frame11, nBytesToRead, 0, &o1))
	{
		if (frame11[1] == EOT) {
			wpData->status = IDLE;
		}
		OutputDebugString(_T("Send to port."));
		return 1;
	}

	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: waitACK
--
-- DATE: November 11, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Tommy Chang
--
-- PROGRAMMER: Jameson Cheong
--
-- INTERFACE: int waitACK()
-- 
--
-- RETURNS: int 1 when receive ACK; int 0 when no ACK
--
-- NOTES: Waits for an ACK using event driven. 
--
----------------------------------------------------------------------------------------------------------------------*/
int waitACK() {
	DWORD CommEvent{ 0 };
	SetCommMask(wpData->hComm, EV_RXCHAR); // event-driven
	if (!WaitCommEvent(wpData->hComm, &CommEvent, 0)) {
		return 1;
	}
	return 0;
}
//return 0 no REQ or REQCounter < 3
int checkREQ() {
	char frameEOT[2] = { 255, EOT };
	if (wpData->receivedREQ == TRUE && REQCounter < 3) {
		REQCounter++;
		if (REQCounter == 3) {
			//To do sent EOF .... need packize eot frame
			
			if (!sendFrame(wpData->hComm, frameEOT, sizeof(frameEOT))) {

			}
			WaitForSingleObject(eotEvent, 1000);
			wpData->status = IDLE;
			
			return 1;
		}

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

	int framePointIndex = 0;
	OutputDebugString(_T("Start Thread SEND"));
	//test frames
	char frame[1024] = { 'J', 'H', 'e', 'l', 'l', 'o' };
	char* frame211 = dataLink->uploadedFrames[0];
	char frameEOT[2] = { 0 , 4 };
	int size = sizeof(frame);
	//test send
	char* framePter;
	int countErrorAck = 0;

	WriteFile(wpData->hComm, dataLink->uploadedFrames[0], 1024, 0, &o1);
	//sendFrame(wpData->hComm, frame, sizeof(frame));
	while (wpData->connected == true) {
		if (countErrorAck == 3) {
			wpData->status = IDLE;
		}
		if (wpData->status == SEND_MODE) {

			framePter = dataLink->uploadedFrames.at(framePointIndex);
			if (sendFrame(wpData->hComm, framePter, sizeof(framePter))){
				if (waitACK()) {
					countErrorAck = 0;
					if (checkREQ()) {		//false, receive REQ or REQCounter == 3
						OutputDebugString(_T("Send EOT, go to IDLE"));
					}
					else {
						sendFrame(wpData->hComm, framePter, sizeof(framePter));
					}
				}
				else {
					//resent frame
					if (sendFrame(wpData->hComm, framePter, sizeof(framePter))) {
						OutputDebugString(_T("Resend Frame"));
					}
					countErrorAck++;
				}
			}
			framePointIndex++;
		}
		else {
			framePointIndex = 0;
			//bid();
		}
	}
	/*
	dataLink->uploadedFrames->at(framePointIndex);
	SetCommMask(wpData->hComm, EV_RXCHAR); // event-driven
	while (wpData->hComm != NULL) {
		if (WaitCommEvent(wpData->hComm, &CommEvent, 0)) { 
			if (Read(wpData->hComm, str, 1, NULL, &o1)) { 
				wpData->hdc = GetDC(wpData->hwnd); 
				printToWindow(wpData->hwnd, wpData->hdc, str, &x, &y); // print character
			}
		}
	}
	*/
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

