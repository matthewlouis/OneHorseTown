#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <utility>

#include "glhelp.h"

namespace odin
{

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

        GraphicalComponent() = default;

        template< typename Vertex, typename... Attrs >
        GraphicalComponent( const Vertex* pVertices,
                            int           count,
                            glm::vec4     color = { 1, 1, 1, 1 },
                            Attrs&&...    attrs )
            : pData( new char[ sizeof( Vertex ) * count ] )
            , count( count )
            , color( color )
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
        }

        ~GraphicalComponent()
        {
            glDeleteVertexArrays( 1, &vertexArray );
            glDeleteBuffers( 1, &vertexBuffer );
            delete[] pData;
        }

        GraphicalComponent( GraphicalComponent&& move )
            : pData( move.pData )
            , count( move.count )
            , color( move.color )
            , texture( move.texture )
            , vertexArray( move.vertexArray )
            , vertexBuffer( move.vertexBuffer )
            , programId( move.programId )
        {
            move.pData = nullptr;
            move.count = 0;
            move.texture = 0;
            move.vertexArray = 0;
            move.vertexBuffer = 0;
            move.programId = 0;
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
            return *this;
        }


        static GraphicalComponent makeRect(
            float     width,
            float     height,
            glm::vec3 color = { 1, 1, 1 },
            float     alpha = 1 )
        {
            float vertices[][5] = {
                { -width / 2, -height / 2, 0, 0, 1 },
                { -width / 2, +height / 2, 0, 0, 0 },
                { +width / 2, +height / 2, 0, 1, 0 },

                { +width / 2, +height / 2, 0, 1, 0 },
                { +width / 2, -height / 2, 0, 1, 1 },
                { -width / 2, -height / 2, 0, 0, 1 },
            };
            ;
            return GraphicalComponent( vertices, 6, glm::vec4{ color, alpha },
                        make_vert_attr< glm::vec3 >( 0 ),
                        make_vert_attr< glm::vec2 >( 1 ) );
        }

        static GraphicalComponent makeRightTri(
            float     width,
            float     height,
            glm::vec3 color = { 1, 1, 1 },
            float     alpha = 1 )
        {
            float vertices[][5] = {
                { -width / 2, -height / 2, 0, 0, 1 },
                { -width / 2, +height / 2, 0, 0, 0 },
                { +width / 2, -height / 2, 0, 1, 1 },
            };

            return GraphicalComponent( vertices, 3, glm::vec4{ color, alpha },
                        make_vert_attr< glm::vec3 >( 0 ),
                        make_vert_attr< glm::vec2 >( 1 ) );
        }

        static GraphicalComponent makeEqTri(
            float     length,
            glm::vec3 color = { 1, 1, 1 },
            float     alpha = 1 )
        {
            const float h = 0.86602540378 * length; // ?3/2 * a

            float vertices[][5] = {
                { 0, 2 * h / 3,        0, 0.5, 0 },
                { +length / 2, -h / 3, 0, 1, 1 },
                { -length / 2, -h / 3, 0, 0, 1 },
            };

            return GraphicalComponent( vertices, 3, glm::vec4{ color, alpha },
                       make_vert_attr< glm::vec3 >( 0 ),
                       make_vert_attr< glm::vec2 >( 1 ) );
        }

    };

} // namespace odin