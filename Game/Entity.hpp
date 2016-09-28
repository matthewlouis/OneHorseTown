#pragma once

#include "Math.hpp"

struct Entity
{
    Vec2        position    = { 0, 0 };
    float       rotation    = 0;
    unsigned    flags       = 0;

    Entity() = default;

    Entity( Vec2 pos, float rot, unsigned flags = 0 )
        : position( pos )
        , rotation( rot )
        , flags( flags )
    {
    }
};