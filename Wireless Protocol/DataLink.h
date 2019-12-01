#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <crc.hpp>
#include <sstream>
using namespace std;
bool packetizeFile(string filePath);
bool checkFrame();
string crc(char* buffer);

struct DataLink {
	vector<char*> uploadedFrames;
	vector<char*> incomingFrames;
	vector<char*> receivedDataword;
};

extern DataLink* dataLink;