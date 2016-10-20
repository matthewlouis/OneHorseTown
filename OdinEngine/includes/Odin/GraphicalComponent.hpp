#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <utility>

#include "glhelp.h"

namespace odin
{
	enum FacingDirection { RIGHT = 1, LEFT = -1 };

    struct VertexAttribute
    {
        GLuint  index;
        GLenum  type;
        GLint   size;
        GLsizei width;

        VertexAttribute( GLuint  index,
                         GLenum  type,
                         GLint   size,
                         GLsizei width )
            : index( index )
            , type( type )
            , size( size )
            , width( width )
        {
        }
    };

    template< typename Attr >
    VertexAttribute make_vert_attr( GLuint index )
    {
        using value_type = typename Attr::value_type;

        return VertexAttribute( index,
            gl_type_constant< value_type >(),
            sizeof(Attr) / sizeof(value_type),
            sizeof(Attr) );
    }

    struct GraphicalComponent
    {
        void*      pData = nullptr;
        int        count = 0;
        glm::vec4  color = { 1, 1, 1, 1 };

        int        texture = 0;
        GLuint     vertexArray = 0;
        GLuint     vertexBuffer = 0;
        GLuint     programId = 0;

		bool       animated = false;
		int*	   animLengths = nullptr; //array of length values - index == anim loop, val == length of loop in frames if 0, not animated
		int        animState = 0;
		int        currentFrame = 0;
		int        maxFrames = 1; //the length of the longest animation on sprite sheet
		int        totalAnim = 1; //the total number of anim states
		FacingDirection  direction = RIGHT;

        GraphicalComponent() = default;

        template< typename Vertex, typename... Attrs >
		GraphicalComponent(const Vertex* pVertices,
							int           count,
							glm::vec4     color = { 1, 1, 1, 1 },
							bool          animated = false,
							int           totalAnim = 0,
							int*          animLength = 0,
                            Attrs&&...    attrs )
            : pData( new char[ sizeof( Vertex ) * count ] )
            , count( count )
            , color( color )
			, animated ( animated )
			, totalAnim( totalAnim )
			, animLengths ( animLength )
        {
            size_t size = sizeof( Vertex ) * count;
            std::memcpy( pData, pVertices, size );

            glGenVertexArrays( 1, &vertexArray );
            glBindVertexArray( vertexArray );

            glGenBuffers( 1, &vertexBuffer );
            glBindBuffer( GL_ARRAY_BUFFER, vertexBuffer );
            glBufferData( GL_ARRAY_BUFFER, size, pData, GL_STATIC_DRAW );

            int offset = 0;
            for ( VertexAttribute attr : { attrs... } )
            {
                glEnableVertexAttribArray( attr.index );
                glVertexAttribPointer( attr.index, attr.size, attr.type,
                    GL_FALSE, sizeof( Vertex ), (void*) offset );
                offset += attr.width;
            }

			if (!animated) //all code below is for animated sprites only
				return;

			animLengths = new int[totalAnim];

			//calculate longest animation while copying values
			for (int i = 0; i < totalAnim; i++) 
			{
				animLengths[i] = animLength[i];
				if (*(animLength + i) > maxFrames)
				{
					maxFrames = *(animLength + i);
				}
			}
        }

        ~GraphicalComponent()
        {
            glDeleteVertexArrays( 1, &vertexArray );
            glDeleteBuffers( 1, &vertexBuffer );
            delete[] pData;
			delete[] animLengths;
        }

        GraphicalComponent( GraphicalComponent&& move )
            : pData( move.pData )
            , count( move.count )
            , color( move.color )
            , texture( move.texture )
            , vertexArray( move.vertexArray )
            , vertexBuffer( move.vertexBuffer )
            , programId( move.programId )
			, animated (move.animated)
			, animLengths (move.animLengths)
			, animState (move.animState)
			, currentFrame (move.currentFrame)
			, maxFrames (move.maxFrames)
			, totalAnim (move.totalAnim)
			, direction (move.direction)
        {
            move.pData = nullptr;
            move.count = 0;
            move.texture = 0;
            move.vertexArray = 0;
            move.vertexBuffer = 0;
            move.programId = 0;
			move.animated = false;
			move.animLengths = nullptr;
			move.animState = 0;
			move.currentFrame = 0;
			move.maxFrames = 1;
			move.totalAnim = 1;
			move.direction = RIGHT;
        }

        GraphicalComponent& operator =( GraphicalComponent&& move )
        {
            using std::swap;
            swap( pData, move.pData );
            swap( count, move.count );
            color = move.color;
            swap( texture, move.texture );
            swap( vertexArray, move.vertexArray );
            swap( vertexBuffer, move.vertexBuffer );
            swap( programId, move.programId );
			swap(animated, move.animated);
			swap(animLengths, move.animLengths);
			swap(animState, move.animState);
			swap(currentFrame, move.currentFrame);
			swap(maxFrames, move.maxFrames);
			swap(totalAnim, move.totalAnim);
			swap(direction, move.direction);
            return *this;
        }

		//increment current frame to draw
		void incrementFrame() 
		{
			if (!animated) //not animated
				return;

			static int frameDelay = 0; //how many draws before changing animation frame
			if (frameDelay++ < 3)
				return;
			else {
				if (currentFrame < *(animLengths + animState) - 1)
					++currentFrame;
				else
					currentFrame = 0;

				frameDelay = 0;
			}
		}


		static GraphicalComponent makeRect(
			float     width,
			float     height,
			glm::vec3 color = { 1, 1, 1 },
			float     alpha = 1,
			bool      animated = false,
			int       totalAnim = 1,
			int*      animLength = nullptr)
        {
            float vertices[][5] = {
                { -width / 2, -height / 2, 0, 0, 1 },
                { -width / 2, +height / 2, 0, 0, 0 },
                { +width / 2, +height / 2, 0, 1, 0 },

                { +width / 2, +height / 2, 0, 1, 0 },
                { +width / 2, -height / 2, 0, 1, 1 },
                { -width / 2, -height / 2, 0, 0, 1 },
            };
            

			return GraphicalComponent(vertices, 6, glm::vec4{ color, alpha },
				animated, totalAnim, animLength,
				make_vert_attr< glm::vec3 >(0),
				make_vert_attr< glm::vec2 >(1));
        }

        static GraphicalComponent makeRightTri(
            float     width,
            float     height,
            glm::vec3 color = { 1, 1, 1 },
            float     alpha = 1,
			bool      animated = false,
			int       totalAnim = 1,
			int*      animLength = nullptr)
        {
            float vertices[][5] = {
                { -width / 2, -height / 2, 0, 0, 1 },
                { -width / 2, +height / 2, 0, 0, 0 },
                { +width / 2, -height / 2, 0, 1, 1 },
            };

            return GraphicalComponent( vertices, 3, glm::vec4{ color, alpha },
						animated, totalAnim, animLength,
                        make_vert_attr< glm::vec3 >( 0 ),
                        make_vert_attr< glm::vec2 >( 1 ) );
        }

        static GraphicalComponent makeEqTri(
            float     length,
            glm::vec3 color = { 1, 1, 1 },
            float     alpha = 1,
			bool      animated = false,
			int       totalAnim = 1,
			int*      animLength = nullptr)
        {
            const float h = float( 0.86602540378 * length ); // ?3/2 * a

            float vertices[][5] = {
                { 0, 2 * h / 3,        0, 0.5, 0 },
                { +length / 2, -h / 3, 0, 1, 1 },
                { -length / 2, -h / 3, 0, 0, 1 },
            };

			return GraphicalComponent(vertices, 3, glm::vec4{ color, alpha }, 
				animated, totalAnim, animLength,
				make_vert_attr< glm::vec3 >(0),
				make_vert_attr< glm::vec2 >(1));
        }

		void switchAnimState(int state)
		{
			if (animState == state) //if we are already on this state
				return;

			animState = state;
			currentFrame = 0; //start from first frame
		}

    };

} // namespace odin