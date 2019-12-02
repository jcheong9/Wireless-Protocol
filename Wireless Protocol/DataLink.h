#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <crc.hpp>
#include <sstream>
using namespace std;
bool packetizeFile(string filePath);
bool checkFrame();
string crc(char* buffer, streamsize buffer_size);
void clearReceivingBuffer();
void storePrintingBuffer(char* dataword);

struct DataLink {
	vector<char*> uploadedFrames;
	vector<char*> incomingFrames;
	vector<string> validDataword;
};

extern DataLink* dataLink;