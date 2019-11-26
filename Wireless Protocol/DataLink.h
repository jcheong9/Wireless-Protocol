#pragma once
#include <string>
#include <vector>
#include <fstream>

#define DWORD_SIZE 1017;
using namespace std;
int packetizeFile(string filePath);

struct DataLink {
	vector<char [1024]>* uploadedFrames;
};

extern DataLink* dataLink;