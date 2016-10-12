// Andrew Meckling
#pragma once

#include <GL/glew.h>
#include <lodepng.h>

namespace odin
{
    constexpr int NUM_TEX_UNITS = GL_MAX_TEXTURE_UNITS - GL_TEXTURE0;

    GLuint texture_units[ NUM_TEX_UNITS ];

    template< typename Array >
    GLuint load_texture( int index, int width, int height, const Array& data )
    {
        if ( index < 0 || index >= NUM_TEX_UNITS )
            return printf( "Texture unit index out of bounds (%i)\n", index ) , 0; // <- comma operator

        glDeleteTextures( 1, &texture_units[ index ] );
        glGenTextures( 1, &texture_units[ index ] );

        // bind texture to the texture unit
        glActiveTexture( GL_TEXTURE0 + index );
        glBindTexture( GL_TEXTURE_2D, texture_units[ index ] );
        
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, (void*) std::data( data ) );

        // possibly don't need this.
        glGenerateMipmap( GL_TEXTURE_2D );

        return texture_units[ index ];
    }

    GLuint load_texture( int index, const char* filename )
    {
        std::vector< GLubyte > image;
        unsigned width, height;

        if ( unsigned error = lodepng::decode( image, width, height, filename ) )
            printf( "Error %u: %s\n", error, lodepng_error_text( error ) );

        return load_texture( index, width, height, image );
    }

    struct Framebuffer
    {
        GLuint frame; // GL_FRAMEBUFFER
        GLuint color; // GL_TEXTURE_2D
        GLuint depth; // GL_RENDERBUFFER

        enum Mode
        {
            NONE  = 0b00,
            COLOR = 0b01,
            DEPTH = 0b10,
            BOTH  = 0b11,
        };
    };


    Framebuffer make_framebuffer( int width, int height, Framebuffer::Mode mode )
    {
        Framebuffer buffer = { 0, 0, 0 };

        glGenFramebuffers( 1, &buffer.frame );
        glBindFramebuffer( GL_FRAMEBUFFER, buffer.frame );

        if ( mode & Framebuffer::COLOR )
        {
            // The texture we're going to render to
            glGenTextures( 1, &buffer.color );

            // "Bind" the newly created texture : all future texture functions 
            // will modify this texture
            glBindTexture( GL_TEXTURE_2D, buffer.color );

            // Give an empty image to OpenGL
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                          GL_RGB, GL_UNSIGNED_BYTE, nullptr );

            // Poor filtering. Needed !
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

            // Set "renderedTexture" as our colour attachement #0
            glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, buffer.color, 0 );
        }

        if ( mode & Framebuffer::DEPTH )
        {
            // The depth buffer
            glGenRenderbuffers( 1, &buffer.depth );
            glBindRenderbuffer( GL_RENDERBUFFER, buffer.depth );
            glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height );
            glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                       GL_RENDERBUFFER, buffer.depth );
        }

        // Set the list of draw buffers.
        //GLenum DrawBuffers[ 1 ] = { GL_COLOR_ATTACHMENT0 };
        //glDrawBuffers( 1, DrawBuffers ); // "1" is the size of DrawBuffers

        // Always check that our framebuffer is ok
        if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
        {
            glDeleteRenderbuffers( 1, &buffer.depth );
            glDeleteTextures( 1, &buffer.color );
            glDeleteFramebuffers( 1, &buffer.frame );

            printf( "Framebuffer error\n" );
        }

        glBindFramebuffer( GL_FRAMEBUFFER, 0 );

        return buffer;
    }

} // namespace odin