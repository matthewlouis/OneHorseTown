#pragma once

#include "glhelp.h"

namespace gl
{
    template< typename T >
    constexpr GLenum type_constant( T = {} );

    template<>
    constexpr GLenum type_constant< GLbyte >( GLbyte )
    {
        return GL_BYTE;
    }

    template<>
    constexpr GLenum type_constant< GLubyte >( GLubyte )
    {
        return GL_UNSIGNED_BYTE;
    }

    template<>
    constexpr GLenum type_constant< GLshort >( GLshort )
    {
        return GL_SHORT;
    }

    template<>
    constexpr GLenum type_constant< GLushort >( GLushort )
    {
        return GL_UNSIGNED_SHORT;
    }

    template<>
    constexpr GLenum type_constant< GLint >( GLint )
    {
        return GL_INT;
    }

    template<>
    constexpr GLenum type_constant< GLuint >( GLuint )
    {
        return GL_UNSIGNED_INT;
    }

    template<>
    constexpr GLenum type_constant< GLfloat >( GLfloat )
    {
        return GL_FLOAT;
    }

    template<>
    constexpr GLenum type_constant< GLdouble >( GLdouble )
    {
        return GL_DOUBLE;
    }
}
