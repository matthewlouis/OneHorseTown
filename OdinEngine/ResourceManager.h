#pragma once

#include "TextureCache.h"

namespace OdinEngine {
	class ResourceManager
	{
	public:
		static GLTexture getTexture(std::string texturePath);
	private:
		static TextureCache _textureCache;
	};
}
