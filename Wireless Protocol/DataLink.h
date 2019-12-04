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
void initialize_frame();
struct DataLink {
	vector<char*> uploadedFrames;
	vector<char*> incomingFrames;
	vector<string> validDataword;
	int charIndex = 0;
	int syncFlag = 0;
	int frameIndex = -1;
	char* dataword = nullptr;
};

extern DataLink* dataLink;