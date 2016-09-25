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
		
		std::vector<Glyph*> _glyphs;
		GlyphSortType _sortType;

		std::vector<RenderBatch> _renderBatches;
	};

}
