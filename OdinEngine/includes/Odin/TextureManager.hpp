// Andrew Meckling
#pragma once

#include <GL/glew.h>
#include <lodepng.h>

namespace odin
{

    template< typename Array >
    GLuint load_texture( int index, int width, int height, const Array& data )
    {
        GLuint tex;
        glGenTextures( 1, &tex );

        glActiveTexture( GL_TEXTURE0 + index );
        glBindTexture( GL_TEXTURE_2D, tex );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                      GL_RGBA, GL_UNSIGNED_BYTE, (void*) std::data( data ) );

        glGenerateMipmap( GL_TEXTURE_2D );

        return tex;
    }

    GLuint load_texture( int index, const char* filename )
    {
        std::vector< GLubyte > image;
        unsigned width, height;

        if ( unsigned error = lodepng::decode( image, width, height, filename ) )
            printf( "Error %u: %s\n", error, lodepng_error_text( error ) );

        return load_texture( index, width, height, image );
    }


} // namespace odin