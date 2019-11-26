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
	BOOL receivedREQ;
	bool connected;
	int status;
};
extern Data * wpData;




LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
void setMenuButton(HWND hwnd, UINT uIDEnableItem, UINT uEnable);
void printToWindow(HWND hwnd, HDC hdc, char* str, unsigned int* x, unsigned int* y);

#define ENQ		0x05;
#define ACK		0x06;
#define REQ		0x11;
#define EOT		0x04;


#endif
