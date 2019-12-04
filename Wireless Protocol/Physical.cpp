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
HANDLE GOOD_FRAME_EVENT = CreateEvent(NULL, TRUE, FALSE, 0);
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

	char frameENQ[2] = { 0, ENQ };

	if (wpData->fileUploaded) {
		if (WriteFile(wpData->hComm, frameENQ, 2, NULL, &o1)) {
			if (WaitForSingleObject(ackEvent, timeoutToReceive) == WAIT_OBJECT_0) {
				ResetEvent(ackEvent);
				wpData->sentdEnq = true;
				OutputDebugString(_T("Received"));
				wpData->status = SEND_MODE;
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
-- DESIGNER: Jameson Cheong
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

	//running completing asynchronously return false
	WriteFile(hComm, frame, nBytesToRead, 0, &o1);
	OutputDebugString(_T("Send to port."));

	return 1;
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

	if (WaitForSingleObject(ackEvent, 1000) == WAIT_OBJECT_0) {
		//reset ack event
		ResetEvent(ackEvent);
		return 1;
	}
	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: checkREQ
--
-- DATE: November 11, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Jameson Cheong
--
-- PROGRAMMER: Jameson Cheong
--
-- INTERFACE: int checkREQ()
--
--
-- RETURNS: int 1 when receive ACK; int 0 when no ACK
--
-- NOTES: Waits for an ACK using event driven.
--
----------------------------------------------------------------------------------------------------------------------*/
//return 0 no REQ or REQCounter < 3
int checkREQ() {
	char frameEOT[2] = { 255, EOT };
	if (wpData->receivedREQ == TRUE && REQCounter < 3) {
		REQCounter++;
		if (REQCounter == 3) {
			//To do sent EOT .... need packize eot frame
			
			if (sendFrame(wpData->hComm, frameEOT, sizeof(frameEOT))) {
				OutputDebugString(_T("Sent EOT."));
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
	//char frame[1024] = { 'J', 'H', 'e', 'l', 'l', 'o' };
	//char* frame211 = dataLink->uploadedFrames[0];
	char frameEOT[2] = { 0 , EOT };
	char frameREQ[2] = { 0 , REQ };
	//int size = sizeof(frame);
	//test send
	char* framePter;
	int countErrorAck = 0;
	bool errorAck = false;
	//wpData->fileUploaded = false;

	//WriteFile(wpData->hComm, dataLink->uploadedFrames[0], 1024, 0, &o1);
	//sendFrame(wpData->hComm, frameREQ, 1024);
	//sendFrame(wpData->hComm, dataLink->uploadedFrames[0], 1024);

	while (wpData->connected == true) {
		if (countErrorAck == 3) {
			countErrorAck = 0;
			OutputDebugString(_T("error ack"));
			wpData->status = IDLE;
		}
		if (wpData->status == SEND_MODE) {
			//framePter = dataLink->uploadedFrames.at(framePointIndex);
			if (sendFrame(wpData->hComm, dataLink->uploadedFrames[framePointIndex], 1024)){
				if (waitACK() || true) {
					errorAck = false;
					countErrorAck = 0;
					if (checkREQ()) {		//false, receive REQ or REQCounter == 3
						OutputDebugString(_T("Send EOT, go to IDLE"));
					}
					else {
						sendFrame(wpData->hComm, dataLink->uploadedFrames[framePointIndex], 1024);
					}
				}
				else {
					//resent frame
					errorAck = true;
					if (sendFrame(wpData->hComm, dataLink->uploadedFrames[framePointIndex], 1024)) {
						OutputDebugString(_T("Resend Frame"));
					}
					countErrorAck++;
				}
			}
			if (framePointIndex < dataLink->uploadedFrames.size() - 1 && !errorAck) {
				framePointIndex++;
			}
			else if(framePointIndex == dataLink->uploadedFrames.size() - 1){
				framePointIndex = 0; 
				wpData->fileUploaded = false;
				sendFrame(wpData->hComm, frameEOT, sizeof(frameEOT));
				WaitForSingleObject(eotEvent, 1000);
				wpData->status = IDLE;
			}
		}
		else if(wpData->status == IDLE && wpData->fileUploaded){	
			Bid();
		}
		else if (wpData->status == RECEIVE_MODE) {
			if(WaitForSingleObject(GOOD_FRAME_EVENT, 3000) == WAIT_OBJECT_0) {
				char frameACK[2] = { wpData->currentSyncByte , ACK };
				sendFrame(wpData->hComm, frameACK, 2);
				ResetEvent(GOOD_FRAME_EVENT);
			}
			else {
				OutputDebugString("Timeoout!");

				wpData->status = IDLE;
			}

		}
	}
	return 1;
}


int ReadInput(char* buffer) {
	DWORD dwRes;
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

			// object signaled
			if (dwRes == WAIT_OBJECT_0) {	// object was signaled, read completed
				if (tempBuffer[0] == ACK || tempBuffer[0] == ENQ || tempBuffer[0] == REQ) {
					buffer[bufferSize] = tempBuffer[0];
					break;
				}
				else {*/
					buffer[bufferSize] = tempBuffer[0];
					bufferSize++;
				}
			}
			// object wasnt signaled
			else {
				return 0;
			}
		}
		// Read instantly
		else {
			buffer[bufferSize] = tempBuffer[0];
			bufferSize++;
		}
	}
	return 1;
}



DWORD WINAPI ThreadReceiveProc(LPVOID n) {
	unsigned static int x = 0;
	unsigned static int y = 0;
	DWORD CommEvent;
	OVERLAPPED ol;
	char str[3];
	char buffer[1024];
	char control;
	SetCommMask(wpData->hComm, EV_RXCHAR);
	while (wpData->connected == true) {
		if (WaitCommEvent(wpData->hComm, &CommEvent, 0)) {
			if (CommEvent & EV_RXCHAR) {
				OutputDebugString("Received something");
				ReadInput(buffer);
				if (buffer[1] == ENQ && wpData->status == IDLE) {
					OutputDebugString("Received an ENQ");
					control = buffer[0];
					sendAcknowledgment(control);
					wpData->status = RECEIVE_MODE;
				}
				else if (buffer[1] == ACK &&  buffer[0] == wpData->currentSyncByte && wpData->status == SEND_MODE ) {
					SetEvent(ackEvent);
				}
				else if (buffer[1] == STX) {
					dataLink->incomingFrames[0] = buffer;
					if (checkFrame() || true) {
						SetEvent(GOOD_FRAME_EVENT);
						OutputDebugString("Receive frame");
						wpData->currentSyncByte = buffer[0];
					}
				}
			}
		}
	}
	return 1;
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

int sendAcknowledgment(char control) {
	OVERLAPPED ol{ 0 };
	char acknowledge[2];
	acknowledge[0] = control;
	if (wpData->status == RECEIVE_MODE && wpData->fileUploaded == false) {
		acknowledge[1] = ACK;
	}
	else if (wpData->status == RECEIVE_MODE && wpData->fileUploaded == true) {
		acknowledge[1] = REQ;
	}
	else if (wpData->status == IDLE) {
		acknowledge[1] = ACK;
	}
	if (!WriteFile(wpData->hComm, &acknowledge, 2, 0, &ol)) {
		OutputDebugString("Failed to send acknowledgment");
	}
	return 0;
}

int randomizeTimeOut(int range_min, int range_max){
	return (double)rand() / (RAND_MAX + 1) * (range_max - range_min) + range_min;
}

void swapSyncByte() {
	wpData->currentSyncByte = wpData->currentSyncByte == 0x00 ? 0xFF : 0x00;
}

//			//send to datalink to verify checksum for this frame
//			// if good, sendAcknowledgment()
//			// else, dont do anything
//		}
//		// but if we received an EOT, simply set the state to IDLE, so the bidding can happen.
//		// read byte by byte
//		// and then after checking sync byte, send it over to datalink

//	}
//}
////	else {
////		wpData->status = IDLE; // timed out
////	}
////}
////else if (wpData->status == IDLE) {
////	SetCommMask(wpData->hComm, EV_RXCHAR);
////	if (WaitCommEvent(wpData->hComm, &CommEvent, 0)) {
////		if (Read(wpData->hComm, str, 2, NULL, &ol)) {
////			if (str[0] == wpData->currentSyncByte && str[1] == ENQ) {
////				sendAcknowledgment();
////			}
////		}
////		// if i receive ack
////		// if i am in idle and have seomthing to send
////		// set event received ACK to true.
////	}
////}

////// i am in sending mode and i am waiting to receive an ACK.
////else if (wpData->status == SEND_MODE) {
////	SetCommMask(wpData->hComm, EV_RXCHAR);
////	if (WaitCommEvent(wpData->hComm, &CommEvent, 0)) {
////		if (Read(wpData->hComm, str, 2, NULL, &ol)) {
////			if (str[0] == wpData->currentSyncByte && str[1] == REQ ) {
////				wpData->receivedREQ = true;
////				SetEvent(ackEvent);
////			}
////			else if (str[0] == wpData->currentSyncByte && str[1] == ACK) {
////				SetEvent(ackEvent);
////			}
////		}
////	}