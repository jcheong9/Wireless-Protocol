#pragma once

#include <windows.h>
#include <tchar.h>

#ifndef SESSION_H
#define SESSION_H



LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
int ConfigurePort(HWND hwnd, HANDLE hComm, LPCWSTR lpszCommName);
#endif
