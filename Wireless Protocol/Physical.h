#pragma once
#ifndef PHYSICAL_H
#define PHYSICAL_H
#include "Application.h"
//#include <windows.h>

#define SYN0		0x00
#define SYN1		0xFF
#define STX			0x02
#define EndOF		0x12
#define ENQ			0x05
#define ACK			0x06
#define REQ			0x11
#define EOT			0x04



HANDLE OpenPort(LPCWSTR lpszCommName);
int Write(HANDLE hComm, TCHAR character);
DWORD WINAPI ThreadSendProc(LPVOID n);
DWORD WINAPI ThreadReceiveProc(LPVOID n);
int Read(HANDLE hComm, char* str, DWORD nNumberofBytesToRead, LPDWORD lpNumberofBytesRead, LPOVERLAPPED o1);
int InitializePort(HANDLE hComm, COMMCONFIG cc, DWORD dwSize);
int randomizeTimeOut(int range_min, int range_max);
int Bid();
#endif
