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
	char frameENQ[2];
	frameENQ[0] = 0x00;
	frameENQ[1] = ENQ;

	if (wpData->fileUploaded) {
		WriteFile(wpData->hComm, frameENQ, 2, NULL, &o1);
			wpData->sentdEnq = true;
			if (WaitForSingleObject(ackEvent, 1000000) == WAIT_OBJECT_0) {
				ResetEvent(ackEvent);
				OutputDebugString(_T("Setting status to send mode"));
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
	wpData->currentSyncByte = frame[0];
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
	OutputDebugString("We are waiting for ACK");
	 if(WaitForSingleObject(ackEvent, 50000) == WAIT_OBJECT_0) {
		//reset ack event
		 OutputDebugString("Received an ACK after sending a frame!");
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
		
		if (wpData->status == SEND_MODE) {
			bool failedSending = true;
			//framePter = dataLink->uploadedFrames.at(framePointIndex);

			while (failedSending) {
				if (framePointIndex < dataLink->uploadedFrames.size()) {
					if (sendFrame(wpData->hComm, dataLink->uploadedFrames[framePointIndex], 1024)) {
						if (waitACK()) {
							if (wpData->status == SEND_MODE) {
								OutputDebugString("Sendmode");
							}
							else {
								OutputDebugString("not in sendmode");
							}
						
							failedSending =false;
							countErrorAck = 0;
						}
						else {
							//resent frame
							failedSending = true;
							countErrorAck++;
							if (countErrorAck == 3) {
								failedSending = false;
								errorAck = true;
								OutputDebugString("B");
								wpData->status = IDLE;
							}
						}
					}
				}
				else {
					framePointIndex = 0;
					sendFrame(wpData->hComm, frameEOT, sizeof(frameEOT));
					wpData->status = IDLE;
					wpData->fileUploaded = false;
				}
			}

			if (framePointIndex < dataLink->uploadedFrames.size() && errorAck == false) {
				framePointIndex++;
			}
			//else if(framePointIndex == dataLink->uploadedFrames.size() && errorAck == false){
			//	framePointIndex = 0; 
			//	wpData->fileUploaded = false;
			//	sendFrame(wpData->hComm, frameEOT, sizeof(frameEOT));
			//	//WaitForSingleObject(eotEvent, 1000);
			//	OutputDebugString("A");
			//	wpData->status = IDLE;
			//}
		}
		else if(wpData->status == IDLE && wpData->fileUploaded){	
			Bid();
		}
		else if (wpData->status == RECEIVE_MODE) {
			if(WaitForSingleObject(GOOD_FRAME_EVENT, 30000) == WAIT_OBJECT_0) {
				char frameACK[2];
				frameACK[0] = wpData->currentSyncByte;
				frameACK[1] = wpData->fileUploaded ? REQ : ACK;
				sendFrame(wpData->hComm, frameACK, 2);
				OutputDebugString("sent an ACK from receiving!!");
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
			memset(&frameBuffer, 0, sizeof(frameBuffer));
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
						OutputDebugString("Received 2 chars!");
						if (controlBuffer[1] == ENQ) {
							control = controlBuffer[0];
							sendAcknowledgment(control);
							wpData->status = RECEIVE_MODE;
							OutputDebugString("Received ENQ from IDLE state and now I'm receiving");
						}
					}
					else if (fRes == TRUE && result == 2 && wpData->sentdEnq) {
						OutputDebugString("Received 2 chars!");
						if (controlBuffer[1] == ACK) {
							SetEvent(ackEvent);
							OutputDebugString("Received ACK from IDLE state");
						}
					}
					PurgeComm(wpData->hComm, PURGE_RXCLEAR);
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
						OutputDebugString("Received 2 chars!");
						control = controlBuffer[0];
						if (control == wpData->currentSyncByte) {
							if (controlBuffer[1] == ACK || controlBuffer[1] == REQ) {
								SetEvent(ackEvent);
								OutputDebugString("Received an ACK! in Send Mode");
								if (control == REQ) {
									wpData->receivedREQ = true;
								}
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
					if (fRes == TRUE && result == 1024) {
						if (frameBuffer[1] == STX)
						OutputDebugString("Received 1024 chars!");
						// check the frame
						// if good, set the event
						wpData->currentSyncByte = frameBuffer[0];
						SetEvent(GOOD_FRAME_EVENT);
						printToWindow(wpData->hwnd, wpData->hdc, frameBuffer, &x, &y);
					}
					else if (fRes == false && result == 2) {
						if (frameBuffer[1] == EOT) {
							OutputDebugString("received EOT! going back to idle..");
							wpData->status = IDLE;
						}
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
	if (!WriteFile(wpData->hComm, &acknowledge, 2, 0, &ol)) {
		OutputDebugString("Failed to send acknowledgment");
	}
	return 0;
}

int randomizeTimeOut(int range_min, int range_max){
	return (double)rand() / (RAND_MAX + 1) * (range_max - range_min) + range_min;
}
