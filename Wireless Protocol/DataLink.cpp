#include "DataLink.h"
#define DWORD_SIZE 1017
#define FRAME_SIZE 1024
/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: DataLink.cpp - This application processes incomming and outgoing frames using the provided functions.
--
--
-- PROGRAM: Wireless Protocol
--
-- FUNCTIONS:
--				bool packetizeFile(string filePath)
--				void initialize_frame()
--				string crc(char* buffer, streamsize buffer_size)
--				bool checkFrame()
--				void storePrintingBuffer(char* dataword)
--				void clearReceivingBuffer()
--
-- DATE: September 30, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- NOTES:
-- This file has functionalities that are related to frames.
-- It does packitize textfiles into multiple frame, and check incoming frames with CRC error detection.
----------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: packetizeFile
--
-- DATE: Demember 3, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: bool packetizeFile(string filePath)
--
--
-- RETURNS: bool
--
-- NOTES:
-- It splits an uploaded text file into multiple frames of which the size is 1024 with header and checksum values
----------------------------------------------------------------------------------------------------------------------*/
DataLink * dataLink = new DataLink();
bool packetizeFile(string filePath) {
	ifstream file{ filePath };
	char c;
	dataLink->charIndex = 0;
	dataLink->syncFlag = 0;
	dataLink->frameIndex = -1;
	dataLink->dataword = nullptr;

	initialize_frame();

	//filling characters 
	while (file.get(c)) {
		//start of the frame & end of dataword
		if (dataLink->charIndex % (FRAME_SIZE - 5) == 0) {
			//attach checksum result and EOF to the end of dataword
			if (dataLink->frameIndex != -1) {
				string crc_result = crc(dataLink->dataword, FRAME_SIZE - 5);
				//fill checksum bytes
				for (int i = 0; i < 8; i += 2) {
					stringstream str;
					string temp = crc_result.substr(i, 2);
					str << temp;
					unsigned int tempInt;
					str >> std::hex >> tempInt;
					dataLink->dataword[dataLink->charIndex++] = tempInt;
				}
				//insert EOF
				dataLink->dataword[dataLink->charIndex] = 0x12;
				dataLink->syncFlag == 0 ? dataLink->syncFlag = 1 : dataLink->syncFlag = 0;
				dataLink->charIndex = 0; //reset char index
			}
			initialize_frame();
		}
		dataLink->dataword[dataLink->charIndex++] = c;
	}
	//in case that the frame has end of file
	if (file.eof()) {
		for (dataLink->charIndex; dataLink->charIndex < FRAME_SIZE - 5; dataLink->charIndex++) {

			dataLink->dataword[dataLink->charIndex] = 0x00;
		}
		string crc_result = crc(dataLink->dataword, FRAME_SIZE - 5);
		//fill checksum bytes
		for (int i = 0; i < 8; i += 2) {
			stringstream str;
			string temp = crc_result.substr(i, 2);
			str << temp;
			int tempInt;
			str >> std::hex >> tempInt;
			dataLink->dataword[dataLink->charIndex++] = tempInt;
			char character = *(dataLink->dataword + dataLink->charIndex - 1);
		}

		dataLink->dataword[dataLink->charIndex] = 0x12;
	}
	return true;
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: initialize_frame
--
-- DATE: Demember 3, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: void initialize_frame()
--
--
-- RETURNS: void
--
-- NOTES:
-- It creates a new frame to store 1017 size of data word from an uploaded file
----------------------------------------------------------------------------------------------------------------------*/
void initialize_frame() {
	//initialize a frame(char array)
	dataLink->uploadedFrames.push_back(new char[FRAME_SIZE]());
	//frame index increment
	dataLink->frameIndex++;
	//adding SYN and STX//
	dataLink->dataword = dataLink->uploadedFrames.at(dataLink->frameIndex);
	dataLink->syncFlag == 0 ? dataLink->dataword[dataLink->charIndex++] = 0x00 : dataLink->dataword[dataLink->charIndex++] = 0xFF;
	dataLink->dataword[dataLink->charIndex++] = 0x02;
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: crc
--
-- DATE: Demember 3, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: string crc(char* buffer, streamsize buffer_size)
--
--
-- RETURNS: string
--
-- NOTES:
-- This function takes chars and run CRC_32 checksum and return the hex value in string format
----------------------------------------------------------------------------------------------------------------------*/
string crc(char* buffer, streamsize buffer_size) {
	boost::crc_32_type  result;
	result.process_bytes(buffer, buffer_size);
	unsigned int crc = result.checksum();
	stringstream stream;
	stream << std::hex << crc;
	string result_in_hex(stream.str());
	return result_in_hex;
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: checkFrame
--
-- DATE: Demember 3, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: bool checkFrame()
--
--
-- RETURNS: bool
--
-- NOTES:
-- This function checks a frame and run CRC with dataword and the first two bytes and compare the result with 
-- the received frame's CRC value
----------------------------------------------------------------------------------------------------------------------*/
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
			crc_result[i / 2] = tempInt;
		}
		for (int i = 0; i < 4; i++) {
			char incoming = crc_result[i];
			char existing = frames.at(0)[FRAME_SIZE - 5 + i];
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
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: storePrintingBuffer
--
-- DATE: Demember 3, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: void storePrintingBuffer(char* dataword)
--
--
-- RETURNS: void
--
-- NOTES:
-- It stores valid dadtaword into header file's string for application layer to print out
----------------------------------------------------------------------------------------------------------------------*/
void storePrintingBuffer(char* dataword) {
	string dataword_str;
	for (int i = 0; i < FRAME_SIZE - 7; i++) {
		if (dataword[i] != 0) dataword_str += dataword[i];
	}
	dataLink->validDataword = dataword_str;
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: clearReceivingBuffer
--
-- DATE: Demember 3, 2019
--
-- REVISIONS: none
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: void clearReceivingBuffer()
--
--
-- RETURNS: void
--
-- NOTES:
-- It clears buffer when a corrupted frame has been received.
----------------------------------------------------------------------------------------------------------------------*/
void clearReceivingBuffer() {
	dataLink->incomingFrames.erase(dataLink->incomingFrames.begin());
}