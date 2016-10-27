// Andrew Meckling
#include "TextureManager.hpp"

#include <lodepng.h>

// Global array of gl texture objects created by odin::load_texture(...).
// Calls to odin::load_texture(N, ...) will delete any existing texture unit
// at index N before creating a new texture.
// DO NOT WRITE TO THIS ARRAY.
GLuint odin::texture_units[ odin::NUM_TEX_UNITS ];

GLuint odin::load_texture( int index, const char* filename )
{
    std::vector< GLubyte > image;
    unsigned width, height;

    if ( unsigned error = lodepng::decode( image, width, height, filename ) )
    {
        printf( "Error %u: %s\n", error, lodepng_error_text( error ) );
        return 0;
    }

    return load_texture( index, width, height, (void*) std::data( image ) );
}

GLuint odin::load_texture( int index, int width, int height, void* data )
{
    if ( index < 0 || index >= NUM_TEX_UNITS )
    {
        printf( "Texture unit index out of bounds (%i)\n", index );
        return 0;
    }

    if ( width <= 0 || height <= 0 )
    {
        printf( "Invalid texture dimensions %ix%i\n", width, height );
        return 0;
    }

    glDeleteTextures( 1, &texture_units[ index ] );
    glGenTextures( 1, &texture_units[ index ] );

    // bind texture to the texture unit
    glActiveTexture( GL_TEXTURE0 + index );
    glBindTexture( GL_TEXTURE_2D, texture_units[ index ] );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                  0, GL_RGBA, GL_UNSIGNED_BYTE, data );

    // possibly don't need this.
    //glGenerateMipmap( GL_TEXTURE_2D );

    return texture_units[ index ];
}

auto odin::make_framebuffer( int width, int height, Framebuffer::Attachments attachments )
    -> Framebuffer
{
    Framebuffer buffer = { 0, 0, 0 };

    glGenFramebuffers( 1, &buffer.frame );
    glBindFramebuffer( GL_FRAMEBUFFER, buffer.frame );

    if ( attachments & Framebuffer::COLOR )
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
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        // Set "renderedTexture" as our colour attachement #0
        glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, buffer.color, 0 );
    }

    if ( attachments & Framebuffer::DEPTH )
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
        // Undo any successful operations
        buffer.glDelete();
        printf( "Framebuffer error\n" );
    }

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    return buffer;
}
