#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include "Vertex.h"

//Spritebatch based on XNA Spritebatch

namespace OdinEngine {

	enum class GlyphSortType{ NONE, FRONT_TO_BACK, BACK_TO_FRONT, TEXTURE };

	//single sprite
	struct Glyph {
		Glyph() {};
		Glyph(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint texture, float depth, const Color& color) :
		texture(texture), depth(depth){
			topLeft.color = color;
			topLeft.setPosition(destRect.x, destRect.y + destRect.w); //destRect w/z are width/height
			topLeft.setUV(uvRect.x, uvRect.y + uvRect.w);

			bottomLeft.color = color;
			bottomLeft.setPosition(destRect.x, destRect.y);
			bottomLeft.setUV(uvRect.x, uvRect.y);

			bottomRight.color = color;
			bottomRight.setPosition(destRect.x + destRect.z, destRect.y);
			bottomRight.setUV(uvRect.x + uvRect.z, uvRect.y);

			topRight.color = color;
			topRight.setPosition(destRect.x + destRect.z, destRect.y + destRect.w);
			topRight.setUV(uvRect.x + uvRect.z, uvRect.y + uvRect.w);

		};

		GLuint texture;
		float  depth; //for rendering sprites behind or in front of eachother

		Vertex topLeft;
		Vertex bottomLeft;
		Vertex topRight;
		Vertex bottomRight;
	};

	class RenderBatch {
	public:
		RenderBatch(GLuint Offset, GLuint NumVertices, GLuint Texture) : offset(Offset),
			numVertices(NumVertices), texture(Texture) {};
		GLuint offset;
		GLuint numVertices;
		GLuint texture;
	};

	class SpriteBatch
	{
	public:
		SpriteBatch();
		~SpriteBatch();

		void init(); //init spritebach

		void begin(GlyphSortType sortType = GlyphSortType::TEXTURE); //prepare for drawing
		void end(); //post-processing

		void draw(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint texture, float depth, const Color& color); //adds sprites to batch

		void renderBatch(); //render all sprites to screen

	private:
		void createRenderBatches();
		void createVertexArray();
		void sortGlyphs();

		static bool compareFrontToBack(Glyph* a, Glyph* b);
		static bool compareBackToFront(Glyph* a, Glyph* b);
		static bool compareTexture(Glyph* a, Glyph* b);

		GLuint _vbo;
		GLuint _vao;
		
		GlyphSortType _sortType;

		std::vector<Glyph> _glyphs;			//actual glyphs
		std::vector<Glyph*> _glyphPointers; //used for sorting

		std::vector<RenderBatch> _renderBatches;
	};

}
