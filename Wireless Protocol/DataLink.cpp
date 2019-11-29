#include "DataLink.h"
#define DWORD_SIZE 1017
#define FRAME_SIZE 1024


DataLink* dataLink = new DataLink();
bool packetizeFile(string filePath) {
	//dataLink->uploadedFrames = new vector<char*>();

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
			dataLink->uploadedFrames.push_back(new char[1024]()); //create a new char array(a frame)
			frameIndex++; //frame index increment
			charIndex = 0; //reset char index
			/*adding SYN and STX*/
			dataword = dataLink->uploadedFrames.at(frameIndex);
			syncFlag == 0 ? dataword[charIndex++] = (int)0 : dataword[charIndex++] = 0xFF;
			dataword[charIndex++] = 0x02;
		}
		dataword[charIndex] = c;
		charIndex++;
	}
	//in case that the frame has eof
	if (file.eof()) {
		/*for (charIndex; charIndex < DWORD_SIZE + 2; charIndex++) {
			dataword[charIndex] = 0x00;
		}*/
		dataword[1019] = 0x04;
		dataword[1020] = 0xC1;
		dataword[1021] = 0x1D;
		dataword[1022] = 0xB7;
		dataword[1023] = 0x12;
	}
	return true;
}



void CRC() {
	char* text[1];
	char wow[4] = { 't', 'e', 's', 't' };
	std::streamsize const buffer_size = 4;
	try
	{
		boost::crc_32_type  result;

		for (int i = 0; i < 1; ++i)
		{
			std::ifstream  ifs(text[i], std::ios_base::binary);

			if (ifs)
			{
				do
				{
					char  buffer[buffer_size];

					ifs.read(buffer, buffer_size);
					result.process_bytes(buffer, ifs.gcount());
				} while (ifs);
			}
			else
			{
				text[i];
				int a = 3;
			}
		}

		int crcresult = result.checksum();
		int aa = 4;
	}
	catch (std::exception & e)
	{
		return;
	}
	catch (...)
	{
		return;
	}
}