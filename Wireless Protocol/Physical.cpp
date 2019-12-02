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

HANDLE ReceiveModeEvent = CreateEvent(NULL, TRUE, FALSE, 0);
HANDLE responseWaitEvent = CreateEvent(NULL, TRUE, TRUE, (LPTSTR)_T("ACK"));
HANDLE ackEvent = CreateEvent(NULL, TRUE, FALSE, 0);
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
	strncpy(frame11, frame, FRAME_SIZE);
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
	char frameEOT[2] = { 0 , 4 };
	int size = sizeof(frame);
	//test send
	char* framePter;
	int countErrorAck = 0;
	
	sendFrame(wpData->hComm, dataLink->uploadedFrames->at(framePointIndex), sizeof(frame));
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

// 
//
//
//
//
//
int ReadInput(char* buffer) {
	DWORD dwRes{ 0 };
	DWORD dwRead{ 0 };
	OVERLAPPED osReader = { 0 };
	osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	int maxSize = 1024;
	int bufferSize = 0;
	char tempBuffer[2];
	tempBuffer[1] = '\0';
	while (bufferSize < maxSize) {
		if (!ReadFile(wpData->hComm, tempBuffer, 1, &dwRead, &osReader)) {
			if (GetLastError() != ERROR_IO_PENDING) { // something occured other than waiting for read to complete
				return 0;
			}
			else {
				dwRes = WaitForSingleObject(osReader.hEvent, 100);  // wait for the read to complete
			}
			if (dwRes == WAIT_OBJECT_0) {	// object was signaled, read completed
				if (tempBuffer[0] == SYN0 || SYN1) {
					if (tempBuffer[0] != wpData->currentSyncByte) {
						return 0;
					}
				}
				else {
					return 0;
				}
				buffer[bufferSize] = tempBuffer[0];
				if (buffer[bufferSize] == EOT) {
					wpData->status == IDLE; // Received end of transmission
					return 1;
				}
			}
			return 1;
		}
	}
}



DWORD WINAPI ThreadReceiveProc(LPVOID n) {
	DWORD CommEvent;
	OVERLAPPED ol;
	char str[3];
	char buffer[1024];
	while (wpData->connected == true) {
		if (wpData->status == RECEIVE_MODE) {
			if (WaitInput(3000) == 1) { // wait 3 seconds for something to arrive in my buffer
				if (ReadInput(buffer)) {
					/*if (checkFrame(buffer) == 1) {
						sendAcknowledgment();
					}*/
					//send to datalink to verify checksum for this frame
					// if good, sendAcknowledgment()
					// else, dont do anything
				}
				// but if we received an EOT, simply set the state to IDLE, so the bidding can happen.
				// read byte by byte
				// and then after checking sync byte, send it over to datalink

			}
			else {
				wpData->status = IDLE; // timed out
			}
		}
		else if (wpData->status == IDLE) {
			SetCommMask(wpData->hComm, EV_RXCHAR);
			if (WaitCommEvent(wpData->hComm, &CommEvent, 0)) {
				if (Read(wpData->hComm, str, 2, NULL, &ol)) {
					if (str[0] == wpData->currentSyncByte && str[1] == ENQ) {
						sendAcknowledgment();
					}
				}
				// if i receive ack
				// if i am in idle and have seomthing to send
				// set event received ACK to true.
			}
		}

		// i am in sending mode and i am waiting to receive an ACK.
		else if (wpData->status == SEND_MODE) {
			SetCommMask(wpData->hComm, EV_RXCHAR);
			if (WaitCommEvent(wpData->hComm, &CommEvent, 0)) {
				if (Read(wpData->hComm, str, 2, NULL, &ol)) {
					if (str[0] == wpData->currentSyncByte && str[1] == REQ ) {
						wpData->receivedREQ = true;
						SetEvent(ackEvent);
					}
					else if (str[0] == wpData->currentSyncByte && str[1] == ACK) {
						SetEvent(ackEvent);
					}
				}
			}
			return 1;
		}
	}
}

int WaitInput(DWORD secs) {
	COMSTAT cs;
	DWORD dwEvtMask{ 0 };
	SetCommMask(wpData->hComm, EV_RXCHAR);
	OVERLAPPED ol;
	ol.hEvent = CreateEvent(NULL, TRUE, FALSE, 0);
	if (!WaitCommEvent(wpData->hComm, &dwEvtMask, &ol))
	{
		if (GetLastError() == ERROR_IO_PENDING)
		{
			if (WaitForSingleObject(ol.hEvent, secs) != WAIT_OBJECT_0)
				return 0;
		}
	}
	CloseHandle(ol.hEvent);
	return 1;
}

int sendAcknowledgment() {
	OVERLAPPED ol{ 0 };
	char acknowledge[2];
	acknowledge[0] = wpData->currentSyncByte; //SYN0 or SYN1
	if (wpData->status == RECEIVE_MODE && wpData->fileUploaded == false) {
		acknowledge[1] = ACK;
	}
	else if (wpData->status == RECEIVE_MODE && wpData->fileUploaded == true) {
		acknowledge[1] = REQ;
	}
	else if (wpData->status == IDLE) {
		acknowledge[1] = ACK;
		wpData->status = RECEIVE_MODE;
	}
	if (!WriteFile(wpData->hComm, &acknowledge, 2, 0, &ol)) {
		OutputDebugString("Failed to send acknowledgment");
	}

}

int randomizeTimeOut(int range_min, int range_max){
	return (double)rand() / (RAND_MAX + 1) * (range_max - range_min) + range_min;
}

void swapSyncByte() {
	wpData->currentSyncByte = wpData->currentSyncByte == 0x00 ? 0xFF : 0x00;
}