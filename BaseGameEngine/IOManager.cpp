#include "IOManager.h"

#include <fstream>

bool IOManager::readFileToBuffer(std::string filePath, std::vector<unsigned char> &buffer){
	std::ifstream file(filePath, std::ios::binary); //read in file as binary
	if (file.fail()) {
		perror(filePath.c_str());
		return false;
	}

	//seek to end of file
	file.seekg(0, std::ios::end);

	//Get the size of the file
	int fileSize = file.tellg();
	file.seekg(0, std::ios::beg); //go back to beginning

	fileSize -= file.tellg(); //get proper filesize minus any header data (if any)

	buffer.resize(fileSize); //now buffer can fit data
	file.read((char *)&buffer[0], fileSize);
	file.close();

	return true; //success!
}
