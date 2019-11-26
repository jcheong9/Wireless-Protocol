#pragma once
#ifndef PHYSICAL_H
#define PHYSICAL_H
#include "Application.h"
//#include <windows.h>

HANDLE OpenPort(LPCWSTR lpszCommName);
//int Write(HANDLE hComm, TCHAR character);
DWORD WINAPI ThreadSendProc(LPVOID n);
DWORD WINAPI ThreadReceiveProc(LPVOID n);
int Read(HANDLE hComm, char* str, DWORD nNumberofBytesToRead, LPDWORD lpNumberofBytesRead, LPOVERLAPPED o1);
int InitializePort(HANDLE hComm, COMMCONFIG cc, DWORD dwSize);
int sendFrame(HANDLE hComm, vector<char[1024]>* frame);
int waitAck();

struct PhysicalData {
	;
};

extern PhysicalData* physicalData;
#endif
