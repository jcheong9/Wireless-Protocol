#include "DataLink.h"
#define DWORD_SIZE 1017
#define FRAME_SIZE 1024


DataLink* dataLink = new DataLink();
bool packetizeFile(string filePath) {
	dataLink->uploadedFrames = new vector<char*>();
	auto frameVec = dataLink->uploadedFrames;
	ifstream file{ filePath };
	char c;
	int charIndex = 0;
	int syncFlag = 0;
	int frameIndex = -1;
	char* dataword = nullptr;
	while (file.get(c)) {
		if (charIndex % (DWORD_SIZE+2) == 0) {
			if (frameIndex != -1) {
				dataword[charIndex++] = 0x04;
				dataword[charIndex++] = 0xC1;
				dataword[charIndex++] = 0x1D;
				dataword[charIndex++] = 0xB7;
				dataword[charIndex] = 0x12;
				syncFlag == 0? syncFlag = 1: syncFlag = 0;
			}
			frameVec->push_back(new char[1024]()); //create new char array
			frameIndex++; //frame index increment
			charIndex = 0; //reset char index
			/*adding SYN and STX*/
			dataword = frameVec->at(frameIndex);
			syncFlag == 0 ? dataword[charIndex++] = (int)0 
				: dataword[charIndex++] = 0xFF;
			dataword[charIndex++] = 0x02;
		}
		dataword[charIndex++] = c;
	}
	//in case that the frame has eof
	if (file.eof()) {
		for (charIndex; charIndex < DWORD_SIZE + 2; charIndex++) {
			dataword[charIndex] = 0x00;
		}
		dataword[charIndex++] = 0x04;
		dataword[charIndex++] = 0xC1;
		dataword[charIndex++] = 0x1D;
		dataword[charIndex++] = 0xB7;
		dataword[charIndex] = 0x12;
	}
	return true;
}



