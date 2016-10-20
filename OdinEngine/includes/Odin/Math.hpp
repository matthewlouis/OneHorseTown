// Andrew Meckling
#pragma once

#include <Box2D/Common/b2Math.h>

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>

#include <cmath>

// Simple vector2 object which allows automatic conversion between
// glm::vec2 and b2Vec2.
union Vec2
{
    glm::vec2 glmvec2;
    b2Vec2    b2vec2;
    struct {
        float x, y;
    };

    constexpr Vec2() : x( 0 ), y( 0 )
    {
    }

    constexpr Vec2( float x, float y )
        : x( x ), y( y )
    {
    }

    constexpr Vec2( const glm::vec2& vec2 )
        : glmvec2( vec2 )
    {
    }

    constexpr Vec2( const b2Vec2& vec2 )
        : b2vec2( vec2 )
    {
    }

    operator glm::vec2() const
    {
        return glmvec2;
    }

    operator b2Vec2() const
    {
        return b2vec2;
    }

    float& operator []( size_t i )
    {
        return *(&x + i);
    }

    const float& operator []( size_t i ) const
    {
        return *(&x + i);
    }
};


template< typename T >
inline T tween( T f0, T f1, T step )
{
    if ( f0 > f1 )
        return f0 - std::min( f0 - f1, step );

    if ( f0 < f1 )
        return f0 + std::min( f1 - f0, step );

    return f0;
}