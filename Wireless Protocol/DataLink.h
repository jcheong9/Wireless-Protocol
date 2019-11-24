#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
using namespace std;
bool packetizeFile(string filePath);
struct DataLink {
	vector<char*>* uploadedFrames;
};

extern DataLink* dataLink;