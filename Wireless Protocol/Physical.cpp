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
HANDLE enqEvent = CreateEvent(NULL, TRUE, FALSE, 0);
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
	char frameENQ[2];
	frameENQ[0] = 0x00;
	frameENQ[1] = ENQ;
	OutputDebugString(_T("\n......Bidding.....\n"));
	if (wpData->fileUploaded) {
		WriteFile(wpData->hComm, frameENQ, 2, NULL, &o1);
		wpData->sentdEnq = true;
		if (WaitForSingleObject(ackEvent, 4000) == WAIT_OBJECT_0) {
			ResetEvent(ackEvent);
			OutputDebugString(_T("Setting status to send mode"));
			wpData->status = SEND_MODE;
		}
		//timeout
		else {
			wpData->sentdEnq = false;
			randomizedTO = randomizeTimeOut(500, 3000);
			OutputDebugString(_T("Timeout2"));
			WaitForSingleObject(enqEvent, randomizedTO);
			ResetEvent(enqEvent);
		}
	}
	wpData->sentdEnq = false;
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
	OutputDebugString(_T("\nSend to port.\n"));

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
	OutputDebugString("We are waiting for ACK");
	 if(WaitForSingleObject(ackEvent, 5000) == WAIT_OBJECT_0) {
		//reset ack event
		ResetEvent(ackEvent);

		OutputDebugString("\n.........Recieved ACK TEST........\n");
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
			wpData->receivedREQ = FALSE;
			
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
	DWORD CommEvent{ 0 };
	int framePointIndexlocal = wpData->framePointIndex;
	char frameEOT[2] = { wpData->currentSyncByte , EOT };
	char frameREQ[2] = { 0 , REQ };
	int countErrorAck = 0;
	int vectorSize = dataLink->uploadedFrames.size();
	bool errorAck = false;
	bool failedSending = true;
	while (wpData->connected == true) {

		if (wpData->status == SEND_MODE && wpData->fileUploaded) {
			//framePter = dataLink->uploadedFrames.at(framePointIndex);
			failedSending = true;
			while (failedSending) {
				if (sendFrame(wpData->hComm, dataLink->uploadedFrames[framePointIndexlocal], 1024)) {
					if (waitACK()) {
						failedSending =false;
						countErrorAck = 0;
						framePointIndexlocal++;
						checkREQ();
					}
					else {
						//resent frame
						failedSending = true;
						countErrorAck++;
						OutputDebugString("\n......Resent frame.....\n");
						if (countErrorAck >= 3) {
							failedSending = false;
							errorAck = true;
							OutputDebugString("\n......Resent frame.....\n");
							wpData->status = IDLE;
						}
					}
				}
			}
			if (framePointIndexlocal == dataLink->uploadedFrames.size()) {
				framePointIndexlocal = 0;
				wpData->fileUploaded = false;
				sendFrame(wpData->hComm, frameEOT, sizeof(frameEOT));
				WaitForSingleObject(eotEvent, 2000);
				OutputDebugString(_T("\n......END frame.....\n"));
				wpData->status = IDLE;
			}
			/*if (framePointIndex < dataLink->uploadedFrames.size() -1) {
			}*/

		}
		else if(wpData->status == IDLE && wpData->fileUploaded){	
			Bid();
		}
		else if (wpData->status == RECEIVE_MODE) {
			if(WaitForSingleObject(GOOD_FRAME_EVENT, 5000) == WAIT_OBJECT_0) {
				char frameACK[2];
				frameACK[0] = wpData->currentSyncByte;
				frameACK[1] = wpData->fileUploaded ? REQ : ACK;

				sendFrame(wpData->hComm, frameACK, 2);
				if (frameACK[1] == REQ) {
					OutputDebugString("Sent an REQ from receiving mode!");
				}
				else {
					OutputDebugString("Sent an ACK from receiving mode!");
				}
				ResetEvent(GOOD_FRAME_EVENT);
			}
			else {
				OutputDebugString("Timeoout!");
				wpData->status = IDLE;
			}

		}
		//OutputDebugString(_T("\n......Running Send thread.....\n"));

	}
	return 1;
}




DWORD WINAPI ThreadReceiveProc(LPVOID n) {
	unsigned static int x = 0;
	unsigned static int y = 0;
	char control;
	BOOL fRes;
	DWORD CommEvent{ 0 };
	DWORD result{ 0 };
	OVERLAPPED ol{ 0 };
	ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (ol.hEvent == NULL) {
		OutputDebugString("Couldn't create the event");
	}
	char controlBuffer[2];
	char frameBuffer[1024];
	SetCommMask(wpData->hComm, EV_RXCHAR);
	while (wpData->connected == true) {
		if (WaitCommEvent(wpData->hComm, &CommEvent, 0)) {
			fRes = FALSE;
			result = -1;
			
			memset(&controlBuffer, 0, sizeof(controlBuffer));
			int sizeofstuff = sizeof(frameBuffer);
			switch (wpData->status) {
				case IDLE:
					if (!ReadFile(wpData->hComm, controlBuffer, 2, &result, &ol)) {
						if (GetLastError() != ERROR_IO_PENDING) {
							fRes = FALSE;
							PurgeComm(wpData->hComm, PURGE_RXCLEAR);
						}
						else {
							if (!GetOverlappedResult(wpData->hComm, &ol, &result, TRUE)) {
								fRes = FALSE;
							}
							else {
								fRes = TRUE;
							}
						}
					}
					else {
						fRes = TRUE;
					}
					if (fRes == TRUE && result == 2 && !wpData->sentdEnq) {
						OutputDebugString("Received 2 chars in IDLE state!");
						if (controlBuffer[1] == ENQ) {
							control = controlBuffer[0];
							sendAcknowledgment(control);
							wpData->status = RECEIVE_MODE;
							SetEvent(enqEvent);
							OutputDebugString("Received ENQ from IDLE state and now I'm receiving");
						}
					}
					else if (fRes == TRUE && result == 2 && wpData->sentdEnq) {
						OutputDebugString("Received 2 chars in IDLE state!");
						if (controlBuffer[1] == ACK) {
							SetEvent(ackEvent);
							wpData->status = SEND_MODE;
							OutputDebugString("Received ACK from IDLE state");
						}
					}
					else {
						PurgeComm(wpData->hComm, PURGE_RXCLEAR);
					}
					break;

				case SEND_MODE:
					if (!ReadFile(wpData->hComm, controlBuffer, 2, &result, &ol)) {
						if (GetLastError() != ERROR_IO_PENDING) {
							fRes = FALSE;
							PurgeComm(wpData->hComm, PURGE_RXCLEAR);
						}
						else {
							if (!GetOverlappedResult(wpData->hComm, &ol, &result, TRUE)) {
								fRes = FALSE;
							}
							else {
								fRes = TRUE;
							}
						}
					}
					else {
						fRes = TRUE;
					}
					if (fRes == TRUE && result == 2) {
						OutputDebugString("Received 2 chars in Send state!");
						control = controlBuffer[0];
							if (controlBuffer[1] == ACK || controlBuffer[1] == REQ && control == wpData->currentSyncByte) {
								SetEvent(ackEvent);
								OutputDebugString("Received an ACK in Send state!");
								if (control == REQ) {
									wpData->receivedREQ = true;
								}
							}
						else {
							PurgeComm(wpData->hComm, PURGE_RXCLEAR);
						}
					}
					else {
						PurgeComm(wpData->hComm, PURGE_RXCLEAR);
					}
					break;
				case RECEIVE_MODE:
					if (!ReadFile(wpData->hComm, frameBuffer, 1024, &result, &ol)) {
						if (GetLastError() != ERROR_IO_PENDING) {
							fRes = FALSE;
							PurgeComm(wpData->hComm, PURGE_RXCLEAR);
						}
						else {
							if (!GetOverlappedResult(wpData->hComm, &ol, &result, TRUE)) {
								fRes = FALSE;
							}
							else {
								fRes = TRUE;
							}
						}
					}
					else {
						fRes = TRUE;
					}
					//if (fRes == FALSE && result == 2) {
						if (frameBuffer[1] == EOT) {
							wpData->status = IDLE;
							OutputDebugString("received EOT, going back to IDLE from receieve");
						}
					//}
					else if (fRes == TRUE && result == 1024) {
						if (frameBuffer[1] == STX) {
							dataLink->incomingFrames.push_back(frameBuffer);
							if (checkFrame()) {
								OutputDebugString("Received 1024 chars in Receive State!");
								// check the frame
								// if good, set the event
								wpData->currentSyncByte = frameBuffer[0];
								SetEvent(GOOD_FRAME_EVENT);

								printToWindowsNew(frameBuffer);
							}
						}
					}
					else {
						PurgeComm(wpData->hComm, PURGE_RXCLEAR);
					}
					PurgeComm(wpData->hComm, PURGE_RXCLEAR);
					break;
			}
				
		}
	}
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
	WriteFile(wpData->hComm, &acknowledge, 2, 0, &ol);
	return 0;
}

int randomizeTimeOut(int range_min, int range_max){
	return (double)rand() / (RAND_MAX + 1) * (range_max - range_min) + range_min;
}

void swapSyncByte() {
	wpData->currentSyncByte = wpData->currentSyncByte == 0x00 ? 0xFF : 0x00;
}
