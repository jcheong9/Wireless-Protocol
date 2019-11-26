#pragma once
#ifndef SESSION_H
#define SESSION_H
#include "Application.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
int ConfigPort(HWND hwnd, HANDLE hComm, LPCSTR lpszCommName);
boolean addFile();
#endif
