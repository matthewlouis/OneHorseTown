#include "TextureCache.h"
#include "ImageLoader.h"

#include <iostream>


TextureCache::TextureCache()
{
}


TextureCache::~TextureCache()
{
}

//Returns specified texture. If not in the map, adds it.
//Prevents multiple copies of the same texture from being loaded into memory
GLTexture TextureCache::getTexture(std::string texturePath) {

	//traverse map to locate texture
	auto mapIterator = _textureMap.find(texturePath);

	if (mapIterator == _textureMap.end()) { //if texture not in map

		//load texture
		GLTexture newTexture = ImageLoader::loadPNG(texturePath);

		//insert texture into map
		_textureMap.insert(make_pair(texturePath, newTexture));

		std::cout << "Loaded AND Cached texture.\n"; //debug code
		return newTexture;
	}

	std::cout << "Loaded texture from cache.\n"; //debug code
	return mapIterator->second;
}
