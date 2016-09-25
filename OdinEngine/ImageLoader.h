#pragma once
#include "GLTexture.h"
#include <string>

namespace OdinEngine {
	class ImageLoader
	{
	public:
		static GLTexture loadPNG(std::string filePath);
	};
}
