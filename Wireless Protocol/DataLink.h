#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <fstream>


#define DWORD_SIZE 1017;
using namespace std;
bool packetizeFile(string filePath);

struct DataLink {
	vector<char*>* uploadedFrames;
};

extern DataLink* dataLink;