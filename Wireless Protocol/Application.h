#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H
#include <windows.h>
#include "DumbMenu.h"
#include "Session.h"
#include "Physical.h"
#include "DataLink.h"
#include <tchar.h>

struct Data {
	HANDLE hComm;
	HDC hdc;
	HWND hwnd;
	bool connected;
};

__declspec(selectany) LPCSTR portNumber = (LPCSTR)"COM1";
extern Data * wpData;
__declspec(selectany) unsigned int xC = 0;
__declspec(selectany) unsigned int yC = 0;
__declspec(selectany) char* s = (char*)"H";
__declspec(selectany) HANDLE readThread = NULL;
__declspec(selectany) DWORD threadId;
__declspec(selectany) OPENFILENAME ofn;
__declspec(selectany) char szFile[1000];


void setMenuButton(HWND hwnd, UINT uIDEnableItem, UINT uEnable);
void printToWindow(HWND hwnd, HDC hdc, char* str, unsigned int* x, unsigned int* y);

#endif
