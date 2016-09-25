#include "ImageLoader.h"
#include "IOManager.h"
#include "picoPNG.h"
#include "Errors.h"

namespace OdinEngine {
	GLTexture ImageLoader::loadPNG(std::string filePath) {
		GLTexture texture = {}; //init all to 0


		std::vector<unsigned char> in; //input image
		std::vector<unsigned char> out; //output image

		unsigned long width, height;

		if (!IOManager::readFileToBuffer(filePath, in)) { //if failed to read
			fatalError("Failed to read PNG to buffer: " + filePath);
		}

		int errorCode = decodePNG(out, width, height, &(in[0]), in.size());

		if (errorCode != 0) {
			fatalError("decodePNG failed with error: " + std::to_string(errorCode));
		}

		glGenTextures(1, &(texture.id)); //generate texture
		glBindTexture(GL_TEXTURE_2D, texture.id); //bind texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(out[0])); //send graphics data to VRAM!

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

		//generate mipmaps (may not need this with pixel art)
		glGenerateMipmap(GL_TEXTURE_2D);

		//unbind texture
		glBindTexture(GL_TEXTURE_2D, 0);

		texture.width = width;
		texture.height = height;

		return texture;
	}
}