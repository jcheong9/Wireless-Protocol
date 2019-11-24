#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H
#include <windows.h>
#include <tchar.h>
#include "DumbMenu.h"

struct Data {
	HANDLE hComm;
	HDC hdc;
	HWND hwnd;
	bool connected;
};

extern Data * wpData;

void setMenuButton(HWND hwnd, UINT uIDEnableItem, UINT uEnable);
void printToWindow(HWND hwnd, HDC hdc, char* str, unsigned int* x, unsigned int* y);

#endif
