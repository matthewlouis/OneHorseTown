#include "SpriteBatch.h"

#include <algorithm>

namespace OdinEngine {
	SpriteBatch::SpriteBatch() : _vbo(0), _vao(0)
	{
	}


	SpriteBatch::~SpriteBatch()
	{
	}

	void SpriteBatch::init() {
		createVertexArray();
	}

	//set up state to begin rendering
	void SpriteBatch::begin(GlyphSortType sortType /* = GlyphSortType::TEXTURE*/) {
		_sortType = sortType;
		_renderBatches.clear();

		for (int i = 0; i < _glyphs.size(); ++i) {
			delete _glyphs[i];
		}
		_glyphs.clear();
	}


	void SpriteBatch::end() {
		sortGlyphs();
		createRenderBatches();
	}


	void SpriteBatch::draw(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint texture, float depth, const Color& color) {
		Glyph* newGlyph = new Glyph;
		newGlyph->texture = texture;
		newGlyph->depth = depth;

		newGlyph->topLeft.color = color;
		newGlyph->topLeft.setPosition(destRect.x, destRect.y + destRect.w); //destRect w/z are width/height
		newGlyph->topLeft.setUV(uvRect.x, uvRect.y + uvRect.w);

		newGlyph->bottomLeft.color = color;
		newGlyph->bottomLeft.setPosition(destRect.x, destRect.y);
		newGlyph->bottomLeft.setUV(uvRect.x, uvRect.y);

		newGlyph->bottomRight.color = color;
		newGlyph->bottomRight.setPosition(destRect.x + destRect.z, destRect.y);
		newGlyph->bottomRight.setUV(uvRect.x + uvRect.z, uvRect.y);

		newGlyph->topRight.color = color;
		newGlyph->topRight.setPosition(destRect.x + destRect.z, destRect.y + destRect.w);
		newGlyph->topRight.setUV(uvRect.x + uvRect.z, uvRect.y + uvRect.w);

		_glyphs.push_back(newGlyph);
	}

	//loop through all render batches and draw them
	void SpriteBatch::renderBatch() {
		glBindVertexArray(_vao);

		for (int i = 0; i < _renderBatches.size(); ++i) {
			glBindTexture(GL_TEXTURE_2D, _renderBatches[i].texture);

			glDrawArrays(GL_TRIANGLES, _renderBatches[i].offset, _renderBatches[i].numVertices);
		}

		glBindVertexArray(0);
	}


	//go through all glyphs, each glyph with a different texture gets a new batch
	void SpriteBatch::createRenderBatches() {

		std::vector<Vertex> vertices;
		vertices.resize(_glyphs.size() * 6); //allocate storage space for number of vertices required

		if (_glyphs.empty()) { //if no glyphs, simply return
			return;
		}
	
		//add all the vertices required
		int offset = 0;
		int currentVertex = 0;
		_renderBatches.emplace_back(offset, 6, _glyphs[0]->texture); //emplace_back allows creation of a renderbatch without additional copying

		vertices[currentVertex++] = _glyphs[0]->topLeft;
		vertices[currentVertex++] = _glyphs[0]->bottomLeft;
		vertices[currentVertex++] = _glyphs[0]->bottomRight;
		vertices[currentVertex++] = _glyphs[0]->bottomRight;
		vertices[currentVertex++] = _glyphs[0]->topRight;
		vertices[currentVertex++] = _glyphs[0]->topLeft;
		offset += 6;

		for (int currentGlyph = 1; currentGlyph < _glyphs.size(); currentGlyph++) {

			//if texture is different, then emplace new renderBatch
			if (_glyphs[currentGlyph]->texture != _glyphs[currentGlyph - 1]->texture) {
				_renderBatches.emplace_back(offset, 6, _glyphs[currentGlyph]->texture);
				offset += 6;
			}
			else { //else increase the size of current renderBatch to include duplicate texture
				_renderBatches.back().numVertices += 6;
			}

			vertices[currentVertex++] = _glyphs[currentGlyph]->topLeft;
			vertices[currentVertex++] = _glyphs[currentGlyph]->bottomLeft;
			vertices[currentVertex++] = _glyphs[currentGlyph]->bottomRight;
			vertices[currentVertex++] = _glyphs[currentGlyph]->bottomRight;
			vertices[currentVertex++] = _glyphs[currentGlyph]->topRight;
			vertices[currentVertex++] = _glyphs[currentGlyph]->topLeft;
		}

		glBindBuffer(GL_ARRAY_BUFFER, _vbo);

		//orphan the buffer: fast than passing the vertex data here
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
		//upload the data
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());

		glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind buffer
	}

	void SpriteBatch::createVertexArray() {

		if (_vao == 0) {
			glGenVertexArrays(1, &_vao);
		}
		glBindVertexArray(_vao);

		if (_vbo == 0) {
			glGenBuffers(1, &_vbo);
		}
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		//Position attribute pointer
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));
		//Color attribute pointer
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void *)offsetof(Vertex, color)); //GL_TRUE normalizes 0-255 to 0-1
		//UV attribute pointer
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));

		glBindVertexArray(0);
	}

	void SpriteBatch::sortGlyphs() {
		switch (_sortType) {
		case GlyphSortType::BACK_TO_FRONT:
			std::stable_sort(_glyphs.begin(), _glyphs.end(), compareBackToFront);
			break;
		case GlyphSortType::FRONT_TO_BACK:
			std::stable_sort(_glyphs.begin(), _glyphs.end(), compareFrontToBack);
			break;
		case GlyphSortType::TEXTURE:
			std::stable_sort(_glyphs.begin(), _glyphs.end(), compareTexture);
			break;
		}		
	}

	bool SpriteBatch::compareFrontToBack(Glyph* a, Glyph* b) {
		return (a->depth < b->depth);
	}
	bool SpriteBatch::compareBackToFront(Glyph* a, Glyph* b) {
		return (a->depth > b->depth);
	}
	bool SpriteBatch::compareTexture(Glyph* a, Glyph* b) {
		 return (a->texture < b->texture); //smaller textur ids get drawn first
	}
}