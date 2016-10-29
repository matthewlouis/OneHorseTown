#pragma once

#include "Scene.h"
#include "AudioEngine.h"

namespace odin
{
	class SceneManager
    {
	public:

		std::vector< Scene* > scenes;
        std::vector< Scene* > pendingScenes;

        Scene* topScene()
        {
            return scenes.empty() ? nullptr : scenes.back();
        }

        size_t count() const
        {
            return scenes.size();
        }

        void pushScene( Scene* scene )
        {
			scene->sceneManager = this;
            pendingScenes.push_back( scene );
        }

        void _pushScene( Scene* scene, unsigned ticks )
        {
            if ( Scene* top = topScene() )
                top->pause( ticks );

            scene->init( ticks );
            scenes.push_back( scene );
            scene->resume( ticks );
        }

        void popScene()
        {
            pendingScenes.push_back( nullptr );
        }

        void _popScene( unsigned ticks )
        {
            Scene* top = scenes.back();
            top->exit( ticks );
            scenes.pop_back();
            delete top;

            if ( top = topScene() )
                top->resume( ticks );
        }

        void popScenes( size_t n )
        {
            while ( n-- > 0 )
                popScene();
        }

        void clearScenes()
        {
            int count = this->count();
            for ( Scene* scene : pendingScenes )
                count += scene ? 1 : -1;

            for ( int i = 0; i < count; ++i )
                popScene();
        }

        void update( unsigned ticks )
        {
            //for ( auto itr = scenes.rbegin(); itr != scenes.rend(); ++itr )
            //    if ( (*itr)->expired )
            //        popScene();
            //    else
            //        break;

            std::vector< Scene* > tmpPendingScenes = pendingScenes;
            pendingScenes.clear();

            for ( Scene* scene : tmpPendingScenes )
                if ( scene == nullptr )
                    _popScene( ticks );
                else
                    _pushScene( scene, ticks );

            Scene* top;
            while ( (top = topScene()) && top->expired )
                _popScene( ticks );

            if ( Scene* top = topScene() )
                top->update( ticks );
        }

        void render()
        {
            if ( Scene* top = topScene() )
                top->draw();
        }

		~SceneManager()
        {
            /*for ( Scene* s : pendingScenes )
                delete s;
            for ( Scene* s : scenes )
                delete s;*/
		}

	};

}