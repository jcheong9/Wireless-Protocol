#pragma once
#ifndef PHYSICAL_H
#define PHYSICAL_H
#include <Windows.h>
HANDLE OpenPort(LPCWSTR lpszCommName);
int Write(HANDLE hComm, TCHAR character);
DWORD WINAPI ReadFunc(LPVOID n);
int Read(HANDLE hComm, char* str, DWORD nNumberofBytesToRead, LPDWORD lpNumberofBytesRead, LPOVERLAPPED o1);
int InitializePort(HANDLE hComm, COMMCONFIG cc, DWORD dwSize);
#endif
