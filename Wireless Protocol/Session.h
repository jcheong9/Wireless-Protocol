#pragma once
#ifndef SESSION_H
#define SESSION_H
#include "Application.h"
//#include <windows.h>
#include "Physical.h"

int ConfigPort(HWND hwnd, HANDLE hComm, LPCSTR lpszCommName);
boolean addFile(OPENFILENAME &ofn);
void prepareTransmission();
#endif
