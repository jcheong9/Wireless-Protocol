#include "DataLink.h"
#define DWORD_SIZE 1017
//#define FRAME_SIZE 1024
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
		if (charIndex % (FRAME_SIZE-5) == 0) {
			if (frameIndex != -1) {
				string crc_result = crc(dataword);
				//fill checksum bytes
				for (int i = 0; i < 4; i+=2) {
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
			}
			dataLink->uploadedFrames.push_back(new char[FRAME_SIZE]()); //create a new char array(a frame)
			frameIndex++; //frame index increment
			charIndex = 0; //reset char index
			/*adding SYN and STX*/
			dataword = dataLink->uploadedFrames.at(frameIndex);
			dataword[charIndex++] = 0x00;
			dataword[charIndex++] = 0x02;
			//syncFlag == 0 ? dataword[charIndex++] = (int)0 : dataword[charIndex++] = 0xFF;
			//dataword[charIndex++] = 0x02;
		}
		dataword[charIndex] = c;
		charIndex++;
	}
	//in case that the frame has end of file
	if (file.eof()) {
		for (charIndex; charIndex < FRAME_SIZE - 5; charIndex++) {
			dataword[charIndex] = 0x00;
		}
		string crc_result = crc(dataword);
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



string crc(char* buffer) {
	std::streamsize const buffer_size = FRAME_SIZE-5;
	boost::crc_32_type  result;
	result.process_bytes(buffer, buffer_size);
	unsigned int crc = result.checksum();
	std::stringstream stream;
	stream << std::hex << crc;
	std::string result_in_hex(stream.str());
	return result_in_hex;
}