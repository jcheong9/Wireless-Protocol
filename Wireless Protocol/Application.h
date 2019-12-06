#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H
#include <windows.h>
#include <CommCtrl.h>
#include "DumbMenu.h"
#include "Session.h"
//#include "Physical.h"
#include "DataLink.h"
#include <tchar.h>
#define IDLE			50
#define RECEIVE_MODE	51
#define SEND_MODE		52
#define COMMAND_MODE	53

struct Data {
	HANDLE hComm;
	HDC hdc;
	HWND hwnd;
	BOOL receivedREQ;
	bool connected;
	bool fileUploaded;
	int status = COMMAND_MODE;
	bool sentdEnq;
	char currentSyncByte = 0;
	int framePointIndex = 0;
	int countAckReceive = 0;
	int countFramesReceive = 0;
	int countReqReceive = 0;
	int countAckSend = 0;
	int countFramesSend = 0;
	int countReqSend = 0;

	HWND labels;
};
extern Data* wpData;



LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
void setMenuButton(HWND hwnd, UINT uIDEnableItem, UINT uEnable);
void printToWindow(HWND hwnd, HDC hdc, char* str, unsigned int* x, unsigned int* y);
void printToWindowsNew(char str[]);
BOOL InitListViewColumns(HWND hWndListView, HINSTANCE hInst, LVCOLUMN cl, char* colName);
void addColumns(HWND hwndLV, LVITEM* lvItem);
void updateStats(LPSTR newValue, int rowPosition);

void prepWindow(HINSTANCE hInst);
void updateStats(LPSTR newValue, int rowPosition);




#endif
