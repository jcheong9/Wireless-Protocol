#pragma once
#ifndef SESSION_H
#define SESSION_H
#include <Windows.h>
#include <tchar.h>
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
int ConfigurePort(HWND hwnd, HANDLE hComm, LPCWSTR lpszCommName);
#endif
