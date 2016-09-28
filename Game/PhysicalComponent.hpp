#pragma once

#include <Box2D/Box2D.h>

#include "Math.hpp"

inline b2Vec2 getShapePosition( const b2Shape* shape )
{
    if ( shape->m_type == b2Shape::e_circle )
    {
        return static_cast< const b2CircleShape* >( shape )->m_p;
    }
    else if ( shape->m_type == b2Shape::e_polygon )
    {
        return static_cast< const b2PolygonShape* >( shape )->m_centroid;
    }
    return b2Vec2_zero;
}

inline b2Vec2 getFixturePosition( const b2Fixture* fixture )
{
    return getShapePosition( fixture->GetShape() );
}

inline b2Vec2 getFixtureWorldPosition( const b2Fixture* fixture )
{
    return fixture->GetBody()->GetPosition()
        + getShapePosition( fixture->GetShape() );
}

struct PhysicalComponent
{
    b2Body* pBody = nullptr;

    PhysicalComponent() = default;

    PhysicalComponent( b2Body* body )
        : pBody( body )
    {
    }

    PhysicalComponent( PhysicalComponent&& move )
        : pBody( move.pBody )
    {
        move.pBody = nullptr;
    }

    ~PhysicalComponent()
    {
        if ( pBody != nullptr )
            pBody->GetWorld()->DestroyBody( pBody );
    }

    PhysicalComponent& operator =( PhysicalComponent&& move )
    {
        std::swap( pBody, move.pBody );
        return *this;
    }

    Vec2 position() const
    {
        return pBody->GetPosition();
    }

    float rotation() const
    {
        return pBody->GetAngle();
    }

    static PhysicalComponent makeRect(
        float     width,
        float     height,
        b2World&  world,
        b2BodyDef bodyDef = {},
        float     density = 1 )
    {
        b2PolygonShape boxShape;
        boxShape.SetAsBox( width / 2, height / 2 );

        b2Body* pBody = world.CreateBody( &bodyDef );
        pBody->CreateFixture( &boxShape, density );

        return PhysicalComponent( pBody );
    }

    static PhysicalComponent makeCircle(
        float     radius,
        b2World&  world,
        b2BodyDef bodyDef = {},
        float     density = 1 )
    {
        b2CircleShape circleShape;
        circleShape.m_radius = radius;

        b2Body* pBody = world.CreateBody( &bodyDef );
        pBody->CreateFixture( &circleShape, density );

        return PhysicalComponent( pBody );
    }

    static PhysicalComponent makeRightTri(
        float     width,
        float     height,
        b2World&  world,
        b2BodyDef bodyDef = {},
        float     density = 1 )
    {
        b2Vec2 vertices[] = {
            { -width / 2, -height / 2 },
            { -width / 2, +height / 2 },
            { +width / 2, -height / 2 },
        };

        b2PolygonShape triShape;
        triShape.Set( vertices, 3 );

        b2Body* pBody = world.CreateBody( &bodyDef );
        pBody->CreateFixture( &triShape, density );

        return PhysicalComponent( pBody );
    }

    static PhysicalComponent makeEqTri(
        float     length,
        b2World&  world,
        b2BodyDef bodyDef = {},
        float     density = 1 )
    {
        float h = 0.86602540378f * length; // ?3 / 2 * a

        b2Vec2 vertices[] = {
            { 0, 2 * h / 3 },
            { +length / 2, -h / 3 },
            { -length / 2, -h / 3 },
        };

        b2PolygonShape triShape;
        triShape.Set( vertices, 3 );

        b2Body* pBody = world.CreateBody( &bodyDef );
        pBody->CreateFixture( &triShape, density );

        return PhysicalComponent( pBody );
    }

};