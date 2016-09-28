#pragma once

#include <Box2D/Common/b2Math.h>
#include <glm/vec2.hpp>
#include <cmath>

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
};