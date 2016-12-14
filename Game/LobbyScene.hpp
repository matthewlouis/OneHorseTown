// Andrew Meckling
#pragma once

#include <Odin/SceneManager.hpp>
#include <Odin/TextureManager.hpp>

#include "TestScene.hpp"

class Delay
{
public:

    unsigned delay;
    unsigned start;

    explicit Delay( unsigned delay, unsigned start = -1 )
        : delay( delay )
        , start( start )
    {
    }

    void restart( unsigned start = -1 )
    {
        this->start = start;
    }

    void set( unsigned delay, unsigned start )
    {
        this->delay = delay;
        this->start = start;
    }

    bool check( unsigned ticks ) const
    {
        return ticks - start > delay;
    }
};

#define DELAY_TRIGGER( DELAY, TICKS, COND ) \
    if ( !bool( COND ) ) \
        (DELAY).restart( TICKS ); \
    else if ( (DELAY).check( TICKS ) )

class LobbyScene
    : public odin::Scene
{
public:

    template< typename ValueType >
    using EntityMap = odin::BinarySearchMap< EntityId, ValueType >;

    static constexpr size_t COMP_MAX = 50;

    template< typename T >
    using Components = TypedAllocator< T, COMP_MAX >;

    EntityMap< Entity2 > entities;

    Components< GraphicalComponent > graphics;


    std::string         audioBankName;
    AudioEngine*        pAudioEngine;
    InputManager*       pInputManager;
    odin::SceneManager* pSceneManager;

    GLuint program;

    GLint uMatrix, uColor, uTexture, uFacingDirection,
        uCurrentFrame, uCurrentAnim, uMaxFrame, uMaxAnim, uSilhoutte, uInteractive;

    static constexpr int MAX_PLAYERS = odin::ControllerManager::MAX_PLAYERS;

    bool playerConnected[ MAX_PLAYERS ];
    bool playerReady[ MAX_PLAYERS ];
    int  playerSlot[ MAX_PLAYERS ];

    LobbyScene( int width, int height, std::string audioBank = "" )
        : Scene( width, height )

        , audioBankName( std::move( audioBank ) )
        , program( load_shaders( "Shaders/vertexAnim.glsl", "Shaders/fragmentShader.glsl" ) )
        , uMatrix( glGetUniformLocation( program, "uMatrix" ) )
        , uColor( glGetUniformLocation( program, "uColor" ) )
        , uTexture( glGetUniformLocation( program, "uTexture" ) )
        , uFacingDirection( glGetUniformLocation( program, "uFacingDirection" ) )
        , uCurrentFrame( glGetUniformLocation( program, "uCurrentFrame" ) )
        , uCurrentAnim( glGetUniformLocation( program, "uCurrentAnim" ) )
        , uMaxFrame( glGetUniformLocation( program, "uMaxFrames" ) )
        , uMaxAnim( glGetUniformLocation( program, "uTotalAnim" ) )
        , uSilhoutte( glGetUniformLocation( program, "uSilhoutte" ) )
        , uInteractive( glGetUniformLocation( program, "uInteractive" ) )
    {
        for ( int i = 0; i < MAX_PLAYERS; ++i )
            playerSlot[ i ] = i;
    }

    GraphicalComponent* newGraphics( GraphicalComponent gfx )
    {
        return ALLOC( graphics, GraphicalComponent )(std::move( gfx ));
    }

    void deleteComponent( GraphicalComponent* gfx )
    {
        DEALLOC( graphics, gfx );
    }

    void init( unsigned ticks ) override
    {
        Scene::init( ticks );

        if ( audioBankName != "" )
        {
            pAudioEngine->loadBank( audioBankName + ".bank",
                                    FMOD_STUDIO_LOAD_BANK_NORMAL );
            pAudioEngine->loadBank( audioBankName + ".strings.bank",
                                    FMOD_STUDIO_LOAD_BANK_NORMAL );
        }

        auto& p0 = entities[ { "player", 0 } ];
        p0.position = { -150, 0 };
        p0.pDrawable = newGraphics( GraphicalComponent::makeRect( 94, 168 ) );
        p0.pDrawable->texture = PLAYER_0_CARD;

        auto& p1 = entities[ { "player", 1 } ];
        p1.position = { -50, 0 };
        p1.pDrawable = newGraphics( GraphicalComponent::makeRect( 94, 168 ) );
        p1.pDrawable->texture = PLAYER_1_CARD;

        auto& p2 = entities[ { "player", 2 } ];
        p2.position = { 50, 0 };
        p2.pDrawable = newGraphics( GraphicalComponent::makeRect( 94, 168 ) );
        p2.pDrawable->texture = PLAYER_2_CARD;

        auto& p3 = entities[ { "player", 3 } ];
        p3.position = { 150, 0 };
        p3.pDrawable = newGraphics( GraphicalComponent::makeRect( 94, 168 ) );
        p3.pDrawable->texture = PLAYER_3_CARD;


        auto& c0 = entities[ { "gpad", 0 } ];
        c0.position = { -150, -100 };
        c0.pDrawable = newGraphics( GraphicalComponent::makeRect( 32, 32 ) );
        c0.pDrawable->texture = 1;

        auto& c1 = entities[ { "gpad", 1 } ];
        c1.position = { -50, -100 };
        c1.pDrawable = newGraphics( GraphicalComponent::makeRect( 32, 32 ) );
        c1.pDrawable->texture = 2;

        auto& c2 = entities[ { "gpad", 2 } ];
        c2.position = { 50, -100 };
        c2.pDrawable = newGraphics( GraphicalComponent::makeRect( 32, 32 ) );
        c2.pDrawable->texture = 3;

        auto& c3 = entities[ { "gpad", 3 } ];
        c3.position = { 150, -100 };
        c3.pDrawable = newGraphics( GraphicalComponent::makeRect( 32, 32 ) );
        c3.pDrawable->texture = 4;
    }

    void exit( unsigned ticks ) override
    {
        Scene::exit( ticks );
        pAudioEngine->stopAllEvents();
        pAudioEngine->setEventParameter( "event:/Music/EnergeticTheme", "Energy", 0.0 );
        pAudioEngine->setEventParameter( "event:/Music/EnergeticTheme", "GameOver", 0.0 );
    }

	void pause(unsigned ticks) override
	{
		Scene::pause(ticks);
		if(pAudioEngine->isEventPlaying("event:/Music/IntroTheme"))
			pAudioEngine->stopEvent("event:/Music/IntroTheme");
	}

    void resume( unsigned ticks ) override
    {
        Scene::resume( ticks );

        odin::load_texture< GLubyte[4] >( NULL_TEXTURE, 1, 1, { 0xFF, 0xFF, 0xFF, 0xFF } );
        odin::load_texture( PLAYER_0_CARD, "Textures/player0.png" );
        odin::load_texture( PLAYER_1_CARD, "Textures/player1.png" );
        odin::load_texture( PLAYER_2_CARD, "Textures/player2.png" );
        odin::load_texture( PLAYER_3_CARD, "Textures/player3.png" );

        odin::load_texture( 1, "Textures/p1.png" );
        odin::load_texture( 2, "Textures/p2.png" );
        odin::load_texture( 3, "Textures/p3.png" );
        odin::load_texture( 4, "Textures/p4.png" );

		//play music if not already playing
		if (!pAudioEngine->isEventPlaying("event:/Music/IntroTheme"))
			pAudioEngine->playEvent("event:/Music/IntroTheme");
    }

    int findPlayerBySlot( int slot ) const
    {
        for ( int i = 0; i < MAX_PLAYERS; ++i )
            if ( playerSlot[ i ] == slot )
                return i;
        return -1;
    }

    int _cyclePlayer( int pos, int offset = 1 )
    {
        int i = pos;
        do {
            i = playerSlot[ i ] + offset;
            i = findPlayerBySlot( i % MAX_PLAYERS );
        } while ( i != pos && playerReady[ i ] );
        return i;
    }

    Delay _allReadyTimeout { 1000 };

    bool emptyLobby() const
    {
        for ( bool player : playerConnected )
            if ( player )
                return false;
        return true;
    }

    int playerCount() const
    {
        int count = 0;
        for ( bool player : playerConnected )
            count += (int) player;
        return count;
    }

    void update( unsigned ticks ) override
    {
        Scene::update( ticks );

        auto& gamepads = pInputManager->gamepads;

        for ( int i = 0; i < MAX_PLAYERS; ++i )
        {
            if ( (playerConnected[ i ] = !!gamepads.getController( i )) )
            {
                if ( !playerReady[ i ] )
                {
                    if ( gamepads.wasButtonPressed( i, SDL_CONTROLLER_BUTTON_DPAD_LEFT ) )
                    {
                        int j = _cyclePlayer( i, MAX_PLAYERS - 1 );
                        std::swap( playerSlot[ i ], playerSlot[ j ] );
                    }

                    if ( gamepads.wasButtonPressed( i, SDL_CONTROLLER_BUTTON_DPAD_RIGHT ) )
                    {
                        int j = _cyclePlayer( i, 1 );
                        std::swap( playerSlot[ i ], playerSlot[ j ] );
                    }
                }

				if (gamepads.wasButtonPressed(i, SDL_CONTROLLER_BUTTON_START)) {
					playerReady[i] = !playerReady[i];

					switch (i) {
					case 0:
						pAudioEngine->playEvent("event:/Desperado/Select_p1");
						break;
					case 1:
						pAudioEngine->playEvent("event:/Desperado/Select_p2");
						break;
					case 2:
						pAudioEngine->playEvent("event:/Desperado/Select_p3");
						break;
					case 3:
						pAudioEngine->playEvent("event:/Desperado/Select_p4");
						break;
					default:
						break;
					}
				}
					
            }
            else
            {
                playerReady[ i ] = false;
            }

            float alpha = playerConnected[ i ] ? playerReady[ i ] ? 1.0 : 0.5 : 0.2;

            uint16_t j = playerSlot[ i ];
            entities[ { "player", j } ].pDrawable->color.a = alpha;
            entities[ { "gpad", j } ].pDrawable->color.a = alpha;
        }

        int validPlayers = 0;
        for ( int i = 0; i < MAX_PLAYERS; ++i )
            if ( playerConnected[ i ] == playerReady[ i ] )
                validPlayers++;

        DELAY_TRIGGER( _allReadyTimeout, ticks, validPlayers == MAX_PLAYERS && playerCount() > 1 )
        {
            std::array< int, MAX_PLAYERS > playerDat;
            playerDat.fill( -1 );

            for ( int i = 0; i < MAX_PLAYERS; ++i )
                if ( playerReady[ i ] )
                    playerDat[ playerSlot[ i ] ] = i;

            auto level = new TestScene( width, height, playerDat );
            level->pInputManager = pInputManager;
            level->pAudioEngine = pAudioEngine;
            level->pSceneManager = pSceneManager;
            pSceneManager->pushScene( level );

            for ( int i = 0; i < MAX_PLAYERS; ++i )
                playerReady[ i ] = false;
        }
    }

    void draw() override
    {
        using namespace glm;
        Scene::draw();

        float zoom = 1.0f / SCALE;
        float aspect = width / (float)height;
        mat4 cameraMatrix = scale({}, vec3(zoom, zoom * aspect, 1));

        glUseProgram( program );

        for ( auto x : entities )
        {
            Entity2& ntt = x.value;

            if ( auto drawable = ntt.pDrawable )
            {
                if ( !drawable->visible )
                    continue;

                mat4 mtx = translate( cameraMatrix, vec3( ntt.position, 0 ) );
                mtx = rotate( mtx, ntt.rotation, vec3( 0, 0, 1 ) );

                glUniform( uMatrix, mtx );
                glUniform( uColor, drawable->color );
                glUniform( uTexture, drawable->texture );
                glUniform( uFacingDirection, drawable->direction );

                glUniform( uCurrentAnim, 0.f );
                glUniform( uCurrentFrame, 0.f );
                glUniform( uMaxFrame, 1.f );
                glUniform( uMaxAnim, 1.f );

                glBindVertexArray( drawable->vertexArray );
                glDrawArrays( GL_TRIANGLES, 0, drawable->count );
            }
        }
    }

};
