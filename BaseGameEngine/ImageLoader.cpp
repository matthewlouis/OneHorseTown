#include "ImageLoader.h"
#include "IOManager.h"
#include "picoPNG.h"
#include "Errors.h"

GLTexture ImageLoader::loadPNG(std::string filePath) {
	GLTexture texture = {}; //init everything to 0
	
	std::vector<unsigned char> inImage;
	std::vector<unsigned char> outImage;
	unsigned long width, height;

	if (IOManager::readFileToBuffer(filePath, inImage) == false) {
		fatalError("Failed to load PNG file to buffer: " + filePath);
	}

	//do actual image decoding
	int errorCode = decodePNG(outImage, width, height, &(inImage[0]), inImage.size());
	if (errorCode != 0) {
		fatalError("decodePNG failed with error: " + errorCode);
	}

	glGenTextures(1, &(texture.id));
	glBindTexture(GL_TEXTURE_2D, texture.id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, GL_FALSE, GL_RGBA, GL_UNSIGNED_BYTE, &(outImage[0])); //fill with pixel data
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //using GL_NEAREST for pixel art
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//for now, no mipmapping
	//glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	texture.width = width;
	texture.height = height;

	return texture;
}