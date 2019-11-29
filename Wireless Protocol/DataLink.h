#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <crc.hpp>
#include <sstream>
using namespace std;
bool packetizeFile(string filePath);
string crc(char* buffer);
struct DataLink {
	vector<char*> uploadedFrames;
};

extern DataLink* dataLink;