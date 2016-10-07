// Andrew Meckling
#pragma once

#include "Entity.hpp"
#include <Odin/EntityId.hpp>

#include <Odin/GraphicalComponent.hpp>
#include <Odin/PhysicalComponent.hpp>
#include <Odin/InputManager.hpp>

#include <Odin/Math.hpp>

#include "Constants.h"

using odin::EntityId;
using odin::GraphicalComponent;
using odin::PhysicalComponent;
using odin::InputManager;
using odin::InputListener;

enum class ComponentType
{
    Graphical, Physical, Input
};

class Game;

// Lazy entity proxy object.
struct EntityView
{
    EntityId eid;
    Game*    pGame;

    EntityView( EntityId eid, Game* game )
        : eid( eid ), pGame( game )
    {
    }

    void attach( GraphicalComponent gfx );
    void attach( PhysicalComponent fsx );
    void attach( InputListener lstn );

    void detach( ComponentType type );

    GraphicalComponent* gfxComponent();
    PhysicalComponent* fsxComponent();
    InputListener* inputListener();
};


class Game
{
public:

    template< typename ValueType >
    using EntityMap = odin::BinarySearchMap< EntityId, ValueType >;


    EntityMap< Entity >             entities;

    GLuint                          program;
    EntityMap< GraphicalComponent > gfxComponents;
    GLint                           uMatrix, uColor, uTexture;

    b2ThreadPool                    b2thd;
    b2World                         b2world = { { 0.f, -9.81f }, &b2thd };
    EntityMap< PhysicalComponent >  fsxComponents;

    InputManager                    inputManager;
    EntityMap< InputListener >      listeners;

    int _width;
    int _height;

    unsigned short _bulletCount = 0;
    bool running = false;


    Game( int width, int height )
        : program( loadShaders( "vertexShader.glsl", "fragmentShader.glsl" ) )
        , uMatrix( glGetUniformLocation( program, "uMatrix" ) ) 
        , uColor( glGetUniformLocation( program, "uColor" ) )
        , uTexture( glGetUniformLocation( program, "uTexture" ) )
        , _width( width )
        , _height( height )
    {
    }

    void draw( const GraphicalComponent& gfx, EntityId eid );

    void update( const PhysicalComponent& fsx, EntityId eid );

    void trigger( const InputListener& lstn, EntityId eid );

    void add( EntityId eid, GraphicalComponent gfx )
    {
        gfxComponents.add( eid, std::move( gfx ) );
    }

    void add( EntityId eid, PhysicalComponent fsx )
    {
        fsxComponents.add( eid, std::move( fsx ) );
    }

    void add( EntityId eid, InputListener lstn )
    {
        listeners.add( eid, std::move( lstn ) );
    }

    EntityView fireBullet( Vec2 position, Vec2 velocity )
    {
        EntityId eid( "bullet", _bulletCount++ );

        if ( !entities.add( eid, Entity( position, 0 ) ) )
            std::cout << "Entity " << eid << " already exists.\n";

        if ( !gfxComponents.add( eid, GraphicalComponent::makeRect( 3.f, .1f ) ) )
            std::cout << "Entity " << eid << " already has a GraphicalComponent.\n";

        b2BodyDef bodyDef;
        bodyDef.position = position;
        bodyDef.linearVelocity = velocity;
        bodyDef.type = b2_dynamicBody;
        bodyDef.bullet = true;

        if ( !fsxComponents.add( eid, PhysicalComponent::makeCircle( .05f, b2world, bodyDef, 0.01 ) ) )
            std::cout << "Entity " << eid << " already has a PhysicalComponent.\n";

        return EntityView( eid, this );
    }

    EntityView addRect( EntityId   eid,
                        Vec2       dimen,
                        Vec2       pos   = { 0, 0 },
                        float      rot   = 0,
                        glm::vec3  color = { 1, 1, 1 },
                        b2BodyType bodyType = b2BodyType::b2_staticBody )
    {
        if ( !entities.add( eid, Entity( pos, rot ) ) )
            std::cout << "Entity " << eid << " already exists.\n";

        if ( !gfxComponents.add( eid,
                GraphicalComponent::makeRect( dimen.x, dimen.y, color ) ) )
            std::cout << "Entity " << eid << " already has a GraphicalComponent.\n";

        b2BodyDef bodyDef;
        bodyDef.position = pos;
        bodyDef.angle = rot;
        bodyDef.type = bodyType;

        if ( !fsxComponents.add( eid,
                PhysicalComponent::makeRect( dimen.x, dimen.y, b2world, bodyDef ) ) )
            std::cout << "Entity " << eid << " already has a PhysicalComponent.\n";

        return EntityView( eid, this );
    }

    EntityView addRightTri( EntityId   eid,
                            Vec2       dimen,
                            Vec2       pos   = { 0, 0 },
                            float      rot   = 0,
                            glm::vec3  color = { 1, 1, 1 },
                            b2BodyType bodyType = b2BodyType::b2_staticBody )
    {
        if ( !entities.add( eid, Entity( pos, rot ) ) )
            std::cout << "Entity " << eid << " already exists.\n";

        if ( !gfxComponents.add( eid,
                GraphicalComponent::makeRightTri( dimen.x, dimen.y, color ) ) )
            std::cout << "Entity " << eid << " already has a GraphicalComponent.\n";

        b2BodyDef bodyDef;
        bodyDef.position = pos;
        bodyDef.angle = rot;
        bodyDef.type = bodyType;

        if ( !fsxComponents.add( eid,
                PhysicalComponent::makeRightTri( dimen.x, dimen.y, b2world, bodyDef ) ) )
            std::cout << "Entity " << eid << " already has a PhysicalComponent.\n";

        return EntityView( eid, this );
    }

    EntityView addEqTri( EntityId   eid,
                         float      length,
                         Vec2       pos   = { 0, 0 },
                         float      rot   = 0,
                         glm::vec3  color = { 1, 1, 1 },
                         b2BodyType bodyType = b2BodyType::b2_staticBody )
    {
        if ( !entities.add( eid, Entity( pos, rot ) ) )
            std::cout << "Entity " << eid << " already exists.\n";

        if ( !gfxComponents.add( eid, GraphicalComponent::makeEqTri( length, color ) ) )
            std::cout << "Entity " << eid << " already has a GraphicalComponent.\n";

        b2BodyDef bodyDef;
        bodyDef.position = pos;
        bodyDef.angle = rot;
        bodyDef.type = bodyType;

        if ( !fsxComponents.add( eid, PhysicalComponent::makeEqTri( length, b2world, bodyDef ) ) )
            std::cout << "Entity " << eid << " already has a PhysicalComponent.\n";

        return EntityView( eid, this );
    }
};


void EntityView::attach( GraphicalComponent gfx )
{
    pGame->gfxComponents[ eid ] = std::move( gfx );
}

void EntityView::attach( PhysicalComponent fsx )
{
    pGame->fsxComponents[ eid ] = std::move( fsx );
}

void EntityView::attach( InputListener lstn )
{
    pGame->listeners[ eid ] = std::move( lstn );
}

void EntityView::detach( ComponentType type )
{
    switch ( type )
    {
    case ComponentType::Graphical :
        pGame->gfxComponents.remove( eid );
        break;
    case ComponentType::Physical :
        pGame->fsxComponents.remove( eid );
        break;
    case ComponentType::Input :
        pGame->listeners.remove( eid );
        break;
    }
}

GraphicalComponent* EntityView::gfxComponent()
{
    auto itr = pGame->gfxComponents.search( eid );
    return itr ? (GraphicalComponent*) itr : nullptr;
}

PhysicalComponent* EntityView::fsxComponent()
{
    auto itr = pGame->fsxComponents.search( eid );
    return itr ? (PhysicalComponent*) itr : nullptr;
}

InputListener* EntityView::inputListener()
{
    auto itr = pGame->listeners.search( eid );
    return itr ? (InputListener*) itr : nullptr;
}

void Game::draw( const GraphicalComponent& gfx, EntityId eid )
{
    using namespace glm;
    Entity& entity = entities[ eid ];

    float zoom = 1.0 / SCALE;
    float aspect = _width / (float) _height;

    mat4 mtx;
    mtx = scale( mtx, vec3( zoom, zoom * aspect, 1 ) );
    mtx = translate( mtx, vec3( entity.position.glmvec2, 0 ) );
    mtx = rotate( mtx, entity.rotation, vec3( 0, 0, 1 ) );

    if ( gfx.programId == 0 )
    {
        glUseProgram( this->program );
        glUniform( uMatrix, mtx );
        glUniform( uColor, gfx.color );
        glUniform( uTexture, gfx.texture );
    }

    glBindVertexArray( gfx.vertexArray );
    glDrawArrays( GL_TRIANGLES, 0, gfx.count );
}

void Game::update( const PhysicalComponent& fsx, EntityId eid )
{
    Entity& entity = entities[ eid ];

    entity.position = fsx.position();

    if ( fsx.pBody->IsBullet() )
    {
        Vec2 vel = fsx.pBody->GetLinearVelocity();

        float angle = std::atan2( vel.y, vel.x );
        fsx.pBody->SetTransform( entity.position, angle );
    }
    
    entity.rotation = fsx.rotation();
}

void Game::trigger( const InputListener& lstn, EntityId eid )
{
    lstn( inputManager, eid );
}