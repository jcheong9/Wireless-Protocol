#include "DataLink.h"
#define DWORD_SIZE 1017
#define FRAME_SIZE 1024

DataLink* dataLink = new DataLink();
bool packetizeFile(string filePath) {
	ifstream file{ filePath };
	char c;
	int charIndex = 0;
	int syncFlag = 0;
	int frameIndex = -1;
	char* dataword = nullptr;
	while (file.get(c)) {
		//start of the frame & end of dataword
		if (charIndex % (FRAME_SIZE-5) == 0) {
			//attach checksum result and EOF to the end of dataword
			if (frameIndex != -1) {
				string crc_result = crc(dataword, FRAME_SIZE-5);
				//fill checksum bytes
				for (int i = 0; i < 8; i+=2) {
					stringstream str;
					string temp = crc_result.substr(i, 2);
					str << temp;
					unsigned int tempInt;
					str >> std::hex >> tempInt;
					dataword[charIndex++] = tempInt;
				}
				//insert EOF
				dataword[charIndex] = 0x12;
				syncFlag == 0? syncFlag = 1: syncFlag = 0;
				charIndex = 0; //reset char index
			}
			//initialize a frame(char array)
			dataLink->uploadedFrames.push_back(new char[FRAME_SIZE]());
			//frame index increment
			frameIndex++; 
			//adding SYN and STX//
			dataword = dataLink->uploadedFrames.at(frameIndex);
			syncFlag == 0 ? dataword[charIndex++] = 0x05 : dataword[charIndex++] = 0xFF;
			dataword[charIndex++] = 0x02;
		}
		dataword[charIndex++] = c;
	}
	//in case that the frame has end of file
	if (file.eof()) {
		for (charIndex; charIndex < FRAME_SIZE - 5; charIndex++) {
			dataword[charIndex] = 0x00;
		}
		string crc_result = crc(dataword, FRAME_SIZE - 5);
		//fill checksum bytes
		for (int i = 0; i < 8; i += 2) {
			stringstream str;
			string temp = crc_result.substr(i, 2);
			str << temp;
			unsigned int tempInt;
			str >> std::hex >> tempInt;
			dataword[charIndex++] = tempInt;
		}
		dataword[charIndex] = 0x12;
	}
	return true;
}


string crc(char* buffer, streamsize buffer_size) {
	boost::crc_32_type  result;
	result.process_bytes(buffer, buffer_size);
	unsigned int crc = result.checksum();
	stringstream stream;
	stream << std::hex << crc;
	string result_in_hex(stream.str());
	return result_in_hex;
}

bool checkFrame() {
	vector<char*> frames = dataLink->incomingFrames;
	if (frames.size() != 0) {
		char* targetFrame = frames.at(0);
		string result_str = crc(targetFrame, FRAME_SIZE - 5);
		char crc_result[4];
		for (int i = 0; i < 8; i += 2) {
			stringstream str;
			string temp = result_str.substr(i, 2);
			str << temp;
			unsigned int tempInt;
			str >> std::hex >> tempInt;
			crc_result[i/2] = tempInt;
		}
		for (int i = 0; i < 4; i++) {
			char incoming = crc_result[i];
			char existing = frames.at(0)[FRAME_SIZE - 5+i];
			if (incoming != existing) {
				clearReceivingBuffer();
				return false;
			}
		}
		storePrintingBuffer(&frames.at(0)[2]);
		clearReceivingBuffer();
		return true;
	}
	return false;
}

void storePrintingBuffer(char* dataword){
	string dataword_str;
	for (int i = 0; i < FRAME_SIZE - 7; i++) {
		if (dataword[i] != 0) dataword_str += dataword[i];
	}
	dataLink->validDataword.push_back(dataword_str);
}

void clearReceivingBuffer() {
	dataLink->incomingFrames.erase(dataLink->incomingFrames.begin());
}