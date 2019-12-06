#include "Physical.h"
/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Physical.c - Contains RS-232 and system level flow control functions
--
--
-- PROGRAM: Wireless Protocol
--
-- FUNCTIONS:
--				int Bid()
--				int waitACK()
--				HANDLE OpenPort(LPCWSTR lpszCommName)
--				int Write(HANDLE hComm, TCHAR character)
--				DWORD WINAPI Read(LPVOID n)
--				int InitializePort(HANDLE hComm, COMMCONFIG cc, DWORD dwSize)
--				int sendFrame(HANDLE hComm, char* frame, DWORD nBytesToRead)
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
TCHAR buf[1024];
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
-- RETURNS: int
--
-- NOTES: Bids for trnsmission if a file is uploaded or awaits an ENQ if no file is uploaded.
--
----------------------------------------------------------------------------------------------------------------------*/

int Bid() {
	OVERLAPPED o1{ 0 };
	char chRead[2];

	DWORD CommEvent{ 0 };
	DWORD timeoutToReceive = 1500;
	DWORD randomizedTO = 0;

	ReceiveModeEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	char frameENQ[2];
	frameENQ[0] = 0x00;
	frameENQ[1] = ENQ;
	OutputDebugString(_T("\n......Bidding.....\n"));

	if (wpData->fileUploaded) {
		OutputDebugString(_T("\n......Sent ENQ.....\n"));
		wpData->sentdEnq = true;
		WriteFile(wpData->hComm, frameENQ, 2, NULL, &o1);

		if (WaitForSingleObject(ackEvent, 2500) == WAIT_OBJECT_0) {
			wpData->status = SEND_MODE;
			ResetEvent(ackEvent);
			OutputDebugString(_T("Setting status to send mode"));
		}
		//timeout
		else {
			wpData->sentdEnq = false;
			randomizedTO = randomizeTimeOut(500, 5000);
			OutputDebugString(_T("Timeout Bidding ENQ"));

			if (WaitForSingleObject(enqEvent, randomizedTO) == WAIT_OBJECT_0) {
				wpData->status = IDLE;
				ResetEvent(enqEvent);
			}
		}
	}

	return 0;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: OpenPort
--
-- DATE: December 5, 2019
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
-- DATE: December 5, 2019
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
-- NOTES: Writes the frame received from datalink. Check end of transmition and set status to IDLE. Display
-- the send frame on the screen.
--
----------------------------------------------------------------------------------------------------------------------*/

int sendFrame(HANDLE hComm, char* frame, DWORD nBytesToRead) {
	DWORD CommEvent{ 0 };
	OVERLAPPED o1{ 0 };


	WriteFile(hComm, frame, nBytesToRead, 0, &o1);
	if (frame[1] == EOT) {
		//WriteFile(hComm, frame, nBytesToRead, 0, &o1);
		wpData->status = IDLE;

		wpData->sentdEnq = false;
		wpData->receivedREQ = false;
		if (WaitForSingleObject(enqEvent, 2000) == WAIT_OBJECT_0) {

			OutputDebugString(_T("GOT AN ENQ WHILE SLEEPING"));
			ResetEvent(enqEvent);
		}
		
		OutputDebugString("Sending an EOT from sendFrame");
		return 1;
	}

	if (frame[1] == STX) {
		printToWindowsNew(frame, 0);
	}

	//running completing asynchronously return false


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
-- DESIGNER: Jameson Cheong
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
	 if(WaitForSingleObject(ackEvent, 4500) == WAIT_OBJECT_0) {
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
-- DESIGNER: Jameson Cheong, Tommy Chang
--
-- PROGRAMMER: Jameson Cheong
--
-- INTERFACE: int checkREQ()
--
--
-- RETURNS: int 1 when receive ACK; int 0 when no ACK
--
-- NOTES: This function check REQ conditions. The function check file is staged and count the frames received then
-- sent EOT. After EOT, the thread sleep for 2 second.
--
----------------------------------------------------------------------------------------------------------------------*/


int checkREQ() {
	char frameEOT[1024];
	memset(&frameEOT, 0, sizeof(EOT));
	frameEOT[0] = '\0';
	frameEOT[1] = EOT;
	if (wpData->receivedREQ == TRUE && REQCounter < 3) {
		REQCounter++;
		if (REQCounter == 3) {
			//To do sent EOT .... need packize eot frame
			REQCounter = 0;
			if (sendFrame(wpData->hComm, frameEOT, sizeof(frameEOT))) {
				OutputDebugString(_T("Sent EOT due to REQCounter."));
				//wpData->fileUploaded = false;
			}

/*			wpData->status = IDLE;

			if (WaitForSingleObject(enqEvent, 4000) == WAIT_OBJECT_0) {
				
				OutputDebugString(_T("GOT AN ENQ WHILE SLEEPING"));
				ResetEvent(enqEvent);
			}	*/		
			return 1;
		}
	}
	return 0;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: ThreadSendProc
--
-- DATE: September 30, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Jameson Cheong, Tommy Chang
--
-- PROGRAMMER: Jameson Cheong
--
-- INTERFACE: DWORD WINAPI ThreadSendProc(LPVOID n)
--
-- RETURNS: DWORD
--
-- NOTES: Function that is executed by a thread created when user enters "connect" mode for the first time.
-- Stays in an infinite loop, and monitors the status. There are three status (IDLE, SEND_MODE, RECEIVE_MODE)
-- this function monitor.
--
----------------------------------------------------------------------------------------------------------------------*/

DWORD WINAPI ThreadSendProc(LPVOID n) {
	PurgeComm(wpData->hComm, PURGE_TXCLEAR);
	OVERLAPPED o1{ 0 };
	DWORD CommEvent{ 0 };
	char frameEOT[1024];
	memset(&frameEOT, 0, sizeof(EOT));
	frameEOT[0] = '\0';
	frameEOT[1] = EOT;
	char frameREQ[2] = { 0 , REQ };
	int countErrorAck = 0;
	int vectorSize = dataLink->uploadedFrames.size();
	bool errorAck = false;
	bool failedSending = true;
	while (wpData->connected == true) {
		PurgeComm(wpData->hComm, PURGE_TXCLEAR);
		if (wpData->status == SEND_MODE && wpData->fileUploaded) {
			//framePter = dataLink->uploadedFrames.at(framePointIndex);
			failedSending = true;
			while (failedSending) {
				if (sendFrame(wpData->hComm, dataLink->uploadedFrames[wpData->framePointIndex], 1024)) {
					++wpData->countFramesSend;
					_stprintf_s(buf, _T("%d"), wpData->countFramesSend);
					updateStats((LPSTR)buf, 10);

					if (waitACK()) {
						failedSending =false;
						countErrorAck = 0;
						wpData->framePointIndex++;
						//checkREQ();
					}
					else {
						//resent frame
						failedSending = true;
						countErrorAck++;
						OutputDebugString("\n......Resent frame.....\n");
						_stprintf_s(buf, _T("%d"), ++wpData->framesResent);
						updateStats((LPSTR)buf, 13);
						if (countErrorAck >= 3) {
							wpData->status = IDLE;
							wpData->receivedREQ = false;
							failedSending = false;
							wpData->sentdEnq = false;
							errorAck = true;
							OutputDebugString("\n......Goes to IDLE when 3 errors.....\n");
						}
					}
				}
			}
			if (wpData->status == SEND_MODE) {
				PurgeComm(wpData->hComm, PURGE_TXCLEAR);
				if (wpData->framePointIndex == dataLink->uploadedFrames.size()) {
					dataLink->uploadedFrames.clear();
					wpData->status = IDLE;
					wpData->fileUploaded = false;
					wpData->sentdEnq = false;
					wpData->framePointIndex = 0;					
					sendFrame(wpData->hComm, frameEOT, 1024);
					OutputDebugString(_T("\n......END frame.....\n"));
				}
			}
		}
		else if(wpData->status == IDLE && wpData->fileUploaded){	
			Bid();
		}
		else if (wpData->status == RECEIVE_MODE) {
			if(WaitForSingleObject(GOOD_FRAME_EVENT, 4500) == WAIT_OBJECT_0) {
				ResetEvent(GOOD_FRAME_EVENT);
				if (wpData->status == RECEIVE_MODE) {
					char frameACK[2];
					frameACK[0] = wpData->currentSyncByte;
					frameACK[1] = wpData->fileUploaded ? REQ : ACK;

					sendFrame(wpData->hComm, frameACK, 2);

					if (frameACK[1] == REQ) {
						OutputDebugString("Sent an REQ from receiving mode!");
						++wpData->countReqSend;
						_stprintf_s(buf, _T("%d"), wpData->countReqSend);
						updateStats((LPSTR)buf, 12);
					}
					else {
						OutputDebugString("Sent an ACK from receiving mode!");
						++wpData->countAckSend;
						_stprintf_s(buf, _T("%d"), wpData->countAckSend);
						updateStats((LPSTR)buf, 11);
					}
				}
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


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: ThreadReceiveProc
--
-- DATE: December 5, 2019
--
-- REVISIONS: none
--
-- DESIGNER:  Tommy Chang
--
-- PROGRAMMER: Tommy Chang
--
-- INTERFACE: DWORD WINAPI ThreadReceiveProc(LPVOID n)
--
-- RETURNS: DWORD
--
-- NOTES: Function that is executed by a thread created when user enters "connect" mode for the first time.
-- Stays in an infinite loop, and monitors the status. There are three status (IDLE, SEND_MODE, RECEIVE_MODE)
-- this function monitor.
-- this functions main purpose is to react to events coming into the input buffer based on state of the program
--
----------------------------------------------------------------------------------------------------------------------*/

DWORD WINAPI ThreadReceiveProc(LPVOID n) {
	unsigned static int x = 0;
	unsigned static int y = 0;
	char control;
	TCHAR buf[1024];
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
		PurgeComm(wpData->hComm, PURGE_RXCLEAR);
		memset(&frameBuffer, 0, sizeof(frameBuffer));
		memset(&controlBuffer, 0, sizeof(controlBuffer));
		//wpData->status = RECEIVE_MODE;
		if (WaitCommEvent(wpData->hComm, &CommEvent, 0)) {
			fRes = FALSE;
			result = -1;
			
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
							SetEvent(enqEvent);
							control = controlBuffer[0];
							sendAcknowledgment(control);
							wpData->status = RECEIVE_MODE;
							OutputDebugString("Received ENQ from IDLE state and now I'm receiving");
						}
					}
					else if (fRes == TRUE && result == 2 && wpData->sentdEnq) {
						OutputDebugString("Received 2 chars in IDLE state BUT I SENT ENQ!");
						//SetEvent(enqEvent);
						if (controlBuffer[1] == ACK || controlBuffer[1] == REQ) {
							wpData->status = SEND_MODE;
							SetEvent(ackEvent);
							OutputDebugString("Received ACK from IDLE state");
							if (controlBuffer[1] == ACK ) {
								_stprintf_s(buf, _T("%d"), ++wpData->countAckReceive);
								updateStats((LPSTR)buf, 21);
							}
							else if (controlBuffer[1] == REQ){
								_stprintf_s(buf, _T("%d"), ++wpData->countReqReceive);
								updateStats((LPSTR)buf, 22);
							}


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
						OutputDebugString(_T("\n Received 2 chars in Send state!\n"));
						control = controlBuffer[1];
							if (controlBuffer[1] == ACK || controlBuffer[1] == REQ ) { /*&& control == wpData->currentSyncByte) {*/
								SetEvent(ackEvent);
								if (control == REQ) {
									wpData->receivedREQ = true;
									checkREQ();
								}
								if (control == ACK) {
									OutputDebugString(_T("\nReceived an ACK from sending mode\n"));
									++wpData->countAckReceive;
									_stprintf_s(buf, _T("%d"), wpData->countAckReceive);
									updateStats((LPSTR)buf, 21);
								}
								else {
									OutputDebugString(_T("Received a REQ from sending mode"));
									++wpData->countReqReceive;
									_stprintf_s(buf, _T("%d"), wpData->countReqReceive);
									updateStats((LPSTR)buf, 22);
								}

							}
					}
					PurgeComm(wpData->hComm, PURGE_RXCLEAR);
					break;
				case RECEIVE_MODE:
					if (!ReadFile(wpData->hComm, frameBuffer, 1024, &result, &ol)) {	
						if (frameBuffer[1] == EOT) {
							wpData->status = IDLE;
							SetEvent(GOOD_FRAME_EVENT);
							OutputDebugString(_T("received EOT, going back to IDLE from receieve"));
							break;
						} else if (GetLastError() != ERROR_IO_PENDING) {
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
						if (frameBuffer[1] == EOT) {
							wpData->status = IDLE;
							SetEvent(GOOD_FRAME_EVENT);
							OutputDebugString(_T("received EOT, going back to IDLE from receieve"));
							break;
						}
						fRes = TRUE;
					}

					if (fRes == TRUE && result == 1024) {

						if (frameBuffer[1] == STX) {
							dataLink->incomingFrames.push_back(frameBuffer);
							if (checkFrame()) {

								OutputDebugString("Received 1024 chars in Receive State!");
								// check the frame
								// if good, set the event
								++wpData->countFramesReceive;
								_stprintf_s(buf, _T("%d"), wpData->countFramesReceive);
								updateStats((LPSTR)buf, 20);
								wpData->currentSyncByte = frameBuffer[0];
								SetEvent(GOOD_FRAME_EVENT);
								printToWindowsNew(frameBuffer, 1);
							}
							else {
								_stprintf_s(buf, _T("%d"), ++wpData->badFrames);
								updateStats((LPSTR)buf, 23);
								OutputDebugString(_T("Bad frame, failed checkframe()"));
							}
						}
					}
					PurgeComm(wpData->hComm, PURGE_RXCLEAR);
					break;
			}
				
		}
	}
	return 1;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendAcknowledgement
--
-- DATE: December 5, 2019
--
-- REVISIONS: none
--
-- DESIGNER:  Tommy Chang
--
-- PROGRAMMER: Tommy Chang
--
-- INTERFACE: int sendAcknowledgment(char control)
--
-- RETURNS: DWORD
--
-- NOTES: Sends acknowledgment when the application changes state from IDLE
--
----------------------------------------------------------------------------------------------------------------------*/



int sendAcknowledgment(char control) {
	OVERLAPPED ol{ 0 };
	char acknowledge[2];
	acknowledge[0] = control;
	if (wpData->status == RECEIVE_MODE && wpData->fileUploaded == false) {
		OutputDebugString("Received mode ACK");
		acknowledge[1] = ACK;
	}
	else if (wpData->status == RECEIVE_MODE && wpData->fileUploaded == true) {
		acknowledge[1] = REQ;
		OutputDebugString("Received mode REQ");

	}
	else if (wpData->status == IDLE) {
		acknowledge[1] = ACK;
		OutputDebugString("IDLE mode ACK");

	}
	WriteFile(wpData->hComm, &acknowledge, 2, 0, &ol);
	OutputDebugString("sent ACK or REQ");
	if (acknowledge[1] == ACK) {
		_stprintf_s(buf, _T("%d"), ++wpData->countAckSend);
		updateStats((LPSTR)buf, 11);
	}
	else if (acknowledge[1] == REQ) {
		_stprintf_s(buf, _T("%d"), ++wpData->countReqSend);
		updateStats((LPSTR)buf, 12);
	}
	return 0;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: randomizeTimeout
--
-- DATE: December 5, 2019
--
-- REVISIONS: none
--
-- DESIGNER:  Tommy Chang
--
-- PROGRAMMER: Tommy Chang
--
-- int randomizeTimeOut(int range_min, int range_max)
--
-- RETURNS: int
--
-- NOTES: returns a random number between an upper bound and a lower band,
-- used to generate a randmoized timeout wait time
--
----------------------------------------------------------------------------------------------------------------------*/ 
int randomizeTimeOut(int range_min, int range_max){
	return (double)rand() / (RAND_MAX + 1) * (range_max - range_min) + range_min;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: swapSyncByte
--
-- DATE: December 5, 2019
--
-- REVISIONS: none
--
-- DESIGNER:  Tommy Chang
--
-- PROGRAMMER: Tommy Chang
--
-- int swapSyncByte()
--
-- RETURNS: void
--
-- NOTES: Unused for now
--
----------------------------------------------------------------------------------------------------------------------*/

void swapSyncByte() {
	wpData->currentSyncByte = wpData->currentSyncByte == 0x00 ? 0xFF : 0x00;
}
