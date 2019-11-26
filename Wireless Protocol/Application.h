#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H
#include <windows.h>
#include "DumbMenu.h"
#include "Session.h"
//#include "Physical.h"
#include "DataLink.h"
#include <tchar.h>

struct Data {
	HANDLE hComm;
	HDC hdc;
	HWND hwnd;
	bool connected;
};


extern Data * wpData;
//__declspec(selectany) unsigned int xC = 0;
//__declspec(selectany) unsigned int yC = 0;
//__declspec(selectany) char* s = (char*)"H";



LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
void setMenuButton(HWND hwnd, UINT uIDEnableItem, UINT uEnable);
void printToWindow(HWND hwnd, HDC hdc, char* str, unsigned int* x, unsigned int* y);

#endif
