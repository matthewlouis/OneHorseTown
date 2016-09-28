#pragma once

#include "Game.h"

enum class ComponentType
{
    Graphical, Physical
};

struct EntityView
{
    EntityId eid;
    Game*    pGame;

    EntityView( EntityId eid,
                Game*    game )
        : eid( eid )
        , pGame( game )
    {
        pGame->entities[ eid ]; // Ensure entity exists
    }

    void attach( GraphicalComponent gfx )
    {
        pGame->gfxComponents[ eid ] = std::move( gfx );
    }

    void attach( PhysicalComponent fsx )
    {
        pGame->fsxComponents[ eid ] = std::move( fsx );
    }

    void detach( ComponentType type )
    {
        switch ( type )
        {
        case ComponentType::Graphical :
            pGame->gfxComponents.remove( eid );
            break;
        case ComponentType::Physical :
            pGame->fsxComponents.remove( eid );
            break;
        }
    }

    GraphicalComponent* gfxComponent()
    {
        auto itr = pGame->gfxComponents.search( eid );
        return itr ? (GraphicalComponent*) itr : nullptr;
    }

    PhysicalComponent* fsxComponent()
    {
        auto itr = pGame->fsxComponents.search( eid );
        return itr ? (PhysicalComponent*) itr : nullptr;
    }
};