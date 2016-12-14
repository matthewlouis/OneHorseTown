// Andrew Meckling
#pragma once

//#include <Odin/Scene.h>
#include <Odin/SceneManager.hpp>
#include <Odin/AudioEngine.h>
#include <Odin/ThreadedAudio.h>
#include <Odin/TextureManager.hpp>
#include <Odin/Camera.h>


#include "Constants.h"
#include "EntityFactory.h"
#include "Player.h"

#include <tuple>
#include <array>
#include <bitset>
#include <memory>

//for clocking assembly
#include <tchar.h>
#include <windows.h>

#include "ContextAllocator.hpp"
#include "TypedAllocator.hpp"


// Macros to speed up rendering
#define PI 3.1415926f
#define SIN45 0.7071f
#define BULLET_PADDING 17
#define BULLET_PADDING_ANGLED 12

using odin::Entity;
using odin::EntityId;
using odin::GraphicalComponent;
using odin::PhysicalComponent;
using odin::AnimatorComponent;
using odin::InputManager;
using odin::InputListener;
using odin::ComponentType;
using odin::AudioEngine;

struct EntityView;
class Player;


// Defines a function which returns the component map for the 
// particular type of component. (This is a 'variadic macro')
#define OHT_DEFINE_COMPONENTS( ... )                            \
template< typename T >                                          \
auto& components()                                              \
{                                                               \
    return std::get< odin::BinarySearchMap< EntityId, T >& >(   \
        std::tie( __VA_ARGS__ ) );                              \
}

template< typename T, typename Sc >
auto& get_components(Sc* pScene)
{
    return pScene->components< T >();
}

struct Entity2;

class EntityBase
{
public:
    const char* name = "<no name>";

    virtual void onDestroy( Entity2& ) {}

    virtual void onEvent( int ) {}

} g_DEFAULT_ENTITY_BASE;

class EntityBullet
	: public EntityBase
{
public:

	Player *player;

	EntityBullet(Player* player)
		: EntityBase()
		, player(player)
	{
	}
};

class EntityPlayer
	: public EntityBase
{
public:

	int playerIndex;
	Player *player;

	EntityPlayer( int index = -1, Player* player = NULL )
		: EntityBase()
		, playerIndex( index )
		, player( player )
	{
	}

	void onDestroy( Entity2& ntt ) override
	{
		std::cout << "Destroying player " << playerIndex << std::endl;
	}

    void onEvent( int e ) override
    {
        if ( e == 1 )
        {
			// Grant immunity while respawning
			if (player->respawning > 0) {
				return;
			}
			player->alive = false;
			Player::deadPlayers++;
            std::cout << "Hit player " << playerIndex << std::endl;

			player->soundEvent = { true, "event:/Desperado/Die" };

			//changes the category of the physics component
			//matt: don't know enough about box2d to know if there's an easier way to do this
			//but only happens 3 times, so shouldn't be too much overhead.
			b2Filter filter = b2Filter();
			filter.categoryBits = DEAD_ENTITY;
			filter.maskBits = PLATFORM;
			player->psx->GetFixtureList()->SetFilterData(filter);

			if (Player::deadPlayers >= Player::totalPlayers - 1 && Player::lastShooterPoints == 2) { //if last player
				player->focus = true;
			}
        }
    }
};

class MyContactListener : public b2ContactListener
{
public:

    std::vector< EntityBase* > deadEntities;

    void BeginContact(b2Contact* contact) {
		
        auto bodyA = contact->GetFixtureA()->GetBody();
        auto bodyB = contact->GetFixtureB()->GetBody();


		if (bodyA->IsBullet())
        {
			/*
			EntityBullet * eb = (EntityBullet *)bodyA->GetUserData();
			eb->player->killCount++;

			if (bodyB->GetUserData()) {
				EntityPlayer* ep = (EntityPlayer *)bodyB->GetUserData();

				ep->player->alive = false;
				//deadEntities.push_back((EntityBase*)bodyB->GetUserData());

				EntityBullet * eb = (EntityBullet *)bodyA->GetUserData();
				eb->player->countKill();
				std::printf("\n*kill bodyA bullet*\n");

				eb->player->soundEvent = { true, "event:/Desperado/Die" };
				
				if (Player::deadPlayers >= 3) //if last player
					eb->player->focus = true;
			}*/
			deadEntities.push_back((EntityBase*)bodyA->GetUserData());
		}

		if (bodyB->IsBullet())
        {
			/*EntityBullet * eb = (EntityBullet *)bodyB->GetUserData();

			eb->player->killCount++;

			if (bodyA->GetUserData()) {
				auto ep = (EntityPlayer *)bodyA->GetUserData();
				ep->player->alive = false;
				//deadEntities.push_back((EntityBase*)bodyA->GetUserData());

				EntityBullet * eb = (EntityBullet *)bodyB->GetUserData();
				eb->player->countKill();

				eb->player->soundEvent = { true, "event:/Desperado/Die" };

				if (Player::deadPlayers >= 3) //if last player
					eb->player->focus = true;
			}*/
			deadEntities.push_back((EntityBase*)bodyB->GetUserData());
		}
	}

    void EndContact(b2Contact* contact) {

    }
};

struct Entity2
{
    friend class LevelScene;
    
    using BasePointer = std::unique_ptr< EntityBase >;

    glm::vec2 position = { 0, 0 };
    float     rotation = 0;
	unsigned  flags = 0;

	b2Body*             pBody = nullptr;
    GraphicalComponent* pDrawable = nullptr;
    AnimatorComponent*  pAnimator = nullptr;

    Entity2() = default;

    Entity2( Vec2 pos, float rot, unsigned fl = 0 )
        : position( pos )
        , rotation( rot )
        , flags( fl )
    {
    }

    Entity2( Entity2&& move )
        : position( move.position )
        , rotation( move.rotation )
        , flags( move.flags )
        , pBody( move.pBody )
        , pDrawable( move.pDrawable )
        , pAnimator( move.pAnimator )
        , _base( std::move( move._base ) )
    {
		move.pBody = nullptr;
        move.pDrawable = nullptr;
        move.pAnimator = nullptr;
    }

    ~Entity2() = default;

    Entity2& operator =( Entity2&& move )
    {
        position = move.position;
        rotation = move.rotation;
        flags = move.flags;
        std::swap( pBody, move.pBody );
        std::swap( pDrawable, move.pDrawable);
        std::swap( pAnimator, move.pAnimator );
        std::swap( _base, move._base );
        return *this;
    }

    void setBase( std::nullptr_t )
    {
        _base = nullptr;
    }

    void setBase( BasePointer&& base )
    {
        _base = std::move( base );
    }

    template< typename EntityClass >
    void setBase( EntityClass self )
    {
        _base = std::make_unique< EntityClass >( std::move( self ) );
    }

    void reset()
    {
        _base = nullptr;
        pBody = nullptr;
        pDrawable = nullptr;
        pAnimator = nullptr;
    }

    void kill()
    {
        (*this)->onDestroy( *this );
        flags |= 1;
    }

    EntityBase* base()
    {
        return _base.get();
    }

    EntityBase* operator ->()
    {
        return _base ? _base.get() : &g_DEFAULT_ENTITY_BASE;
    }

private:

    BasePointer _base = nullptr;
};




class LevelScene
    : public odin::Scene
{
public:

    using EntityPlayerType = EntityPlayer;

    template< typename ValueType >
    using EntityMap = odin::BinarySearchMap< EntityId, ValueType >;

    static constexpr size_t COMP_MAX = 500;

    template< typename T >
    using Components = TypedAllocator< T, COMP_MAX >;

    EntityMap< Entity2 > entities;

    Components< GraphicalComponent > graphics;
    Components< AnimatorComponent >  animations;

    GraphicalComponent* newGraphics( GraphicalComponent gfx )
    {
        return ALLOC( graphics, GraphicalComponent )( std::move( gfx ) );
    }

    AnimatorComponent* newAnimator( AnimatorComponent anim )
    {
        return ALLOC( animations, AnimatorComponent )( std::move( anim ) );
    }

    b2Body* newBody( const b2BodyDef& bodyDef )
    {
        return b2world.CreateBody( &bodyDef );
    }

    void deleteComponent( GraphicalComponent* gfx )
    {
        if ( gfx ) gfx->~GraphicalComponent();
        DEALLOC( graphics, gfx );
    }

    void deleteComponent( AnimatorComponent* anim )
    {
        if ( anim ) anim->~AnimatorComponent();
        DEALLOC( animations, anim );
    }

    void deleteComponent( b2Body* body )
    {
        if ( body ) b2world.DestroyBody( body );
    }


    //EntityMap< Entity >             entities;

    //EntityMap< GraphicalComponent > gfxComponents;
    //EntityMap< AnimatorComponent >  animComponents;

    b2ThreadPool                    b2thd;
    b2World                         b2world = { { 0.f, -9.81f } };
    //EntityMap< PhysicalComponent >  fsxComponents;

    //OHT_DEFINE_COMPONENTS( entities, gfxComponents, animComponents, fsxComponents );

    InputManager*                   pInputManager;
    std::vector< InputListener >    listeners;

    std::string                     audioBankName;
    AudioEngine*                    pAudioEngine;

    odin::SceneManager* pSceneManager;

	odin::Camera camera;

    //SDL_Renderer* renderer;

    GLuint program;
    GLint uMatrix, uColor, uTexture, uFacingDirection,
        uCurrentFrame, uCurrentAnim, uMaxFrame, uMaxAnim, uSilhoutte, uInteractive;

    //for simulating energy - alpha presentation
    float energyLevel = 0;
    unsigned short _bulletCount = 0;

	// Range of the bullets, set to diagonal screen distance by default
	float bulletRange;

    MyContactListener _contactListener;

    /*PolymorphicAllocator<
        ThresholdAllocator< 4,
            BitsetAllocator< 1024 * 2, 4 >,
            ThresholdAllocator< 128,
                BitsetAllocator< 1024 * 1024, 8 >,
                Mallocator
            >
        >
    > _localAllocator;*/

	int			numberPlayers;
	Player      players[MAX_PLAYERS];
	Player		*winningPlayer, *lastToDiePlayer;
	int			timeOnDeadPlayer;
	int			maxPoints = 3;

	bool		gameOver = false;
	Uint32		gameOverStartTicks;
	bool	    startingGame = true;
	Uint32		startingGameStartTicks;
	float	    silhouette; //for adjusting character colors
	bool	    usingASM = true;

	LevelScene(int width, int height, std::string audioBank = "", int numberPlayers = MAX_PLAYERS)
		: Scene(width, height)
		, audioBankName(audioBank)
		, numberPlayers(numberPlayers)

		, program(load_shaders("Shaders/vertexAnim.glsl", "Shaders/fragmentShader.glsl"))
		, uMatrix(glGetUniformLocation(program, "uMatrix"))
		, uColor(glGetUniformLocation(program, "uColor"))
		, uTexture(glGetUniformLocation(program, "uTexture"))
		, uFacingDirection(glGetUniformLocation(program, "uFacingDirection"))
		, uCurrentFrame(glGetUniformLocation(program, "uCurrentFrame"))
		, uCurrentAnim(glGetUniformLocation(program, "uCurrentAnim"))
		, uMaxFrame(glGetUniformLocation(program, "uMaxFrames"))
		, uMaxAnim(glGetUniformLocation(program, "uTotalAnim"))
		, uSilhoutte(glGetUniformLocation(program, "uSilhoutte"))
		, uInteractive(glGetUniformLocation(program, "uInteractive"))
    {
    }

	void init(unsigned ticks)
    {
		Scene::init(ticks);

		bulletRange = sqrt(width * width + height * height);
		camera.init(width, height);

		b2world.SetContactListener(&_contactListener);

		if (audioBankName != "")
        {
			pAudioEngine->loadBank(audioBankName + ".bank",
				FMOD_STUDIO_LOAD_BANK_NORMAL);
			pAudioEngine->loadBank(audioBankName + ".strings.bank",
				FMOD_STUDIO_LOAD_BANK_NORMAL);
        }

		/*dim screen - may not need now that background animates
		EntityId dimScreenid("wreadd", 0);
		decltype(auto) dimScreen = entities[dimScreenid];
		dimScreen.position = Vec2(0, 0);
		dimScreen.pDrawable = newGraphics(GraphicalComponent::makeRect(VIRTUAL_WIDTH, VIRTUAL_HEIGHT, { 255.f, 255.f, 255.f }, 0.5f));
		dimScreen.pDrawable->texture = BLACK;
		*/

		//ready text creation
		EntityId readyid("wready", 0);
		decltype(auto) ready = entities[readyid];
		ready.position = Vec2(0, 0);
		ready.pDrawable = newGraphics(GraphicalComponent::makeRect(256, 64, { 255.f, 255.f, 255.f }, 1.0f));
		ready.pDrawable->interactive = false;
		ready.pDrawable->texture = READY_TEXTURE;
		ready.pAnimator = newAnimator(AnimatorComponent({ 16, 16 }));
		ready.pAnimator->loop = false;

		//win screen creation (starts with alpha = 0)
		EntityId wineid("wintex", 0);
		decltype(auto) wintex = entities[wineid];
		wintex.position = Vec2(0, 0);
		wintex.pDrawable = newGraphics(GraphicalComponent::makeRect(64, 64, { 255.f, 255.f, 255.f }, 0.0f));
		wintex.pDrawable->texture = WIN_TEXTURE;
		wintex.pAnimator = newAnimator(AnimatorComponent({ 1, 1 }));

		listeners.push_back([this](const InputManager& inmn) {
			if (inmn.wasKeyPressed(SDLK_g)) {
				entities["wintex"].position.x -= 10;
    }
		});
		listeners.push_back([this](const InputManager& inmn) {
			if (inmn.wasKeyPressed(SDLK_h)) {
				entities["wintex"].position.x += 10;
			}
		});
		listeners.push_back([this](const InputManager& inmn) {
			if (inmn.wasKeyPressed(SDLK_1)) {
				usingASM = !usingASM;
				printf("\nNow using: %s", usingASM ? "ASM" : "C++");
			}
		});
	}

	void resume(unsigned ticks)
    {
		startingGameStartTicks = ticks;
		Scene::resume(ticks);
        //context_allocator::push( _localAllocator );
    }

	void pause(unsigned ticks)
    {
		Scene::pause(ticks);
        //context_allocator::pop();
    }

	void exit(unsigned ticks)
    {
		Scene::exit(ticks);
		pAudioEngine->stopAllEvents();
		pAudioEngine->setEventParameter("event:/Music/EnergeticTheme", "Energy", 0.0);
		pAudioEngine->setEventParameter("event:/Desperado/Die", "lastkill", 0.0);
		pAudioEngine->setEventParameter("event:/Music/EnergeticTheme", "GameOver", 0.0);
		energyLevel = 0;
		Player::deadPlayers = 0;

		/*Using 1 bank for all scene now so do NOT unload
		if (audioBankName != "")
        {
			pAudioEngine->unloadBank(audioBankName + ".bank");
			pAudioEngine->unloadBank(audioBankName + ".strings.bank");
        }*/
    }

	void _destroy(Entity2& ntt)
    {
        ntt.kill();
        deleteComponent( ntt.pBody );
        deleteComponent( ntt.pDrawable );
        deleteComponent( ntt.pAnimator );
        ntt.reset();
    }

	void update(unsigned ticks)
	{
		Scene::update(ticks);

		//play any sound events players have triggered
		for (Player& p : players) {
			p.update();
			if (p.soundEvent.playEvent) { //if there is a sound to play
				pAudioEngine->playEvent(p.soundEvent.event); //play it
				p.soundEvent = {}; //reset soundevent
			}
		}


		//game over zoom in and win text display
		if (gameOver) {
			glm::vec2 cameraPos = camera.getPosition();
			float scale = camera.getScale();
			float maxscale = 5.0f;
			int delayAmount = 200;

			pAudioEngine->setEventParameter("event:/Desperado/Die", "lastkill", 1.0);


			if (gameOverStartTicks == 0) {
				if (pAudioEngine->isEventPlaying("event:/Music/EnergeticTheme"))
					pAudioEngine->stopEvent("event:/Music/EnergeticTheme"); //mute music

				lastToDiePlayer->anim->frameDelay = 0;
				lastToDiePlayer->anim->incrementFrame();

				scale = maxscale;
				cameraPos = { lastToDiePlayer->psx->GetPosition().x * 10 * scale, lastToDiePlayer->psx->GetPosition().y * 10 * scale };

				//if player is dead on ground, and blood pool animation is playing
				//then reduce slowdown
				if (lastToDiePlayer->currentState == PlayerState::DEAD) {
					if (lastToDiePlayer->anim->currentFrame >= 5) {
						gameOverStartTicks = SDL_GetTicks();
					}
				}
			}
			else if (ticks - gameOverStartTicks > 1750) {
				if (!pAudioEngine->isEventPlaying("event:/Music/EnergeticTheme"))
					pAudioEngine->playEvent("event:/Music/EnergeticTheme"); //play music if not already playing

				delayAmount = 0;

				entities["wintex"].pDrawable->color.a = 1.0f;
				entities["wintex"].pAnimator->switchAnimState(0);

				scale = camera.getScale();
				if (scale < maxscale) {
					scale += 0.1f;
				}

				glm::vec2 focusPos = { winningPlayer->psx->GetPosition().x * 10 * maxscale, winningPlayer->psx->GetPosition().y * 10 * maxscale };
				cameraPos = camera.getPosition();

				if (focusPos.x < cameraPos.x - CAMERA_MOVE_AMOUNT) {
					cameraPos.x -= CAMERA_MOVE_AMOUNT;
				}
				else if (focusPos.x > cameraPos.x + CAMERA_MOVE_AMOUNT) {
					cameraPos.x += CAMERA_MOVE_AMOUNT;
				}

				if (focusPos.y < cameraPos.y - CAMERA_MOVE_AMOUNT) {
					cameraPos.y -= CAMERA_MOVE_AMOUNT;
				}
				else if (focusPos.y > cameraPos.y + CAMERA_MOVE_AMOUNT) {
					cameraPos.y += CAMERA_MOVE_AMOUNT;
				}
			}

			if (ticks - gameOverStartTicks > 2000)
				entities["wintex"].pAnimator->switchAnimState(1);

			camera.setPosition(cameraPos, true);
			camera.setScale(scale);

			entities["wintex"].position = glm::vec2(cameraPos.x / scale, cameraPos.y / scale);

			SDL_Delay(delayAmount);
		}

		//setting player position so players appear in corners on first draw
		for (auto x : entities)
		{
			Entity2& ntt = x.value;
			if (auto body = ntt.pBody)
			{
				Vec2 pos = body->GetPosition();
				ntt.position = glm::round(glm::vec2(pos) * 10.0f);
				ntt.rotation = body->GetAngle();
			}
		}


		//if starting game, don't go any further: we want to halt gameplay
		//READY DRAW section 
		if (startingGame) {
			AnimatorComponent *ac = entities["wready"].pAnimator;
			ac->incrementFrame();

			if (ac->currentFrame % 3 == 1)
				entities[EntityId(0)].pAnimator->incrementFrame(); //next background frame

			if (ac->animState == 0 && ac->currentFrame >= ac->maxFrames - 1) { //if READY displayed and animation finished
				ac->switchAnimState(1); //change to DRAW animation state
			}
			else if (ac->animState == 1) {
				if (ac->currentFrame >= 6)
					entities[EntityId(0)].pAnimator->incrementFrame(); //next background frame

				if (ac->currentFrame == 6) {//DRAW has appeared
					pAudioEngine->playEvent("event:/Desperado/Draw");
				}
				else if (ac->currentFrame >= ac->maxFrames - 1) {
					startingGame = false;
					entities["wready"].pDrawable->visible = false;
				}
			}

			silhouette = (float)(entities[EntityId(0)].pAnimator->currentFrame) / (entities[EntityId(0)].pAnimator->maxFrames - 1);
			glUniform(uSilhoutte, 1.0f);

			return;
		}


		//poll input events
		for (auto& lstn : listeners)
			lstn(*pInputManager);

        float timeStep = Scene::ticksDiff / 1000.f;
		b2world.Step(timeStep, 8, 3);


        std::vector< EntityId > deadEntities;
		deadEntities.reserve(_contactListener.deadEntities.size());

		for (auto x : entities)
        {
            Entity2& ntt = x.value;
            if ( auto animator = ntt.pAnimator )
            {
                animator->incrementFrame();

				switch (animator->type)
                {
                    // Handle fading animation
                case odin::AnimationType::FADEOUT:
					if (animator->currentFrame == animator->maxFrames - 1)
                    {
						_destroy(ntt);
						deadEntities.push_back(x.key);
                    }
                    else {
                        ntt.pDrawable->color.a = 1.f - (float)animator->currentFrame / (float)animator->maxFrames;
                    }
                default:
                    break;
                }
            }
        }

        for ( auto x : entities )
        {
            if ( EntityBase* pbase = x.value.base() )
            {
                for ( EntityBase* ebase : _contactListener.deadEntities )
                {
                    if ( ebase == pbase )
                    {
                        _destroy( x.value );
                        deadEntities.push_back( x.key );
                        break;
                    }
                }
            }
        }

        if ( !_contactListener.deadEntities.empty() )
            _contactListener.deadEntities.clear();

		for (EntityId eid : deadEntities)
			entities.remove(eid);


		if (!gameOver && Player::deadPlayers >= numberPlayers - 1) {
			for (int i = 0; i < numberPlayers; ++i)
			{
				Player& p = players[i];
				if (p.alive) {
					//printf("\n%d: %f", i, p.points);
					if (p.awardPoint(i) == maxPoints) {
						gameOverSequence();
						break;
					}
				}
			}
			if (!gameOver) {
				

				for each (Player& p in players) {
					if (!p.alive) {
						p.respawn();
					}
				}
			}
		}

		if (drawDetected()) {
			printf("IT'S A DRAW... MULLIGAN!");
			pAudioEngine->playEvent("event:/Desperado/Draw");
			for each (Player& p in players) {
				if(p.active)
					p.respawn();
			}
		}

		int max = 0;
		for each (Player& p in players)
			if (p.points > max)
				max = p.points;

		energyLevel = max * 0.4f;
		energyLevel = energyLevel > 1.0f ? 1.0f : energyLevel;
		pAudioEngine->setEventParameter("event:/Music/EnergeticTheme", "Energy", energyLevel);

    }
	
	bool drawDetected() {
		int totalBullets = 0;

		for (int i = 0; i < numberPlayers; ++i)
			if (players[i].alive)
				totalBullets += players[i].bulletCount;

		return totalBullets == 0 && Player::deadPlayers < numberPlayers - 1;
	}

	void gameOverSequence() {
		gameOver = true;

		pAudioEngine->setEventParameter("event:/Music/EnergeticTheme", "GameOver", 1.0f);
		timeOnDeadPlayer = SDL_GetTicks();
		//entities["wintex"].pDrawable->color = glm::vec4(255, 255, 255, 1);

		for (int i = 0; i < numberPlayers; i++) {
			if (players[i].focus) //last player
				lastToDiePlayer = &players[i];

			if (players[i].alive) {
				winningPlayer = &players[i];
				printf("\n****GAME OVER and Player %d won!", i + 1);
			}
		}
	}

	//silhouettes a sprite using c++
	glm::vec4 inline silhouetteCPP(const glm::vec4* color, bool interactive, float* silhouette) {
		static long printCount = 0;
		if (!interactive)
			return *color;

		return glm::vec4(color->r * *silhouette, color->g * *silhouette, color->b * *silhouette, color->a);
	}

	//silhouettes a sprite using ASM
	glm::vec4 inline silhouetteASM(const glm::vec4* color, bool interactive, float* silhouette) {
		if (!interactive)
			return *color;

		glm::vec4 silCol;
		__m128 col = _mm_set_ps(color->a, *silhouette, *silhouette, *silhouette);  //silhouette color (keeps alpha value)
		
		__asm
		{
			mov eax, color			  //loads the color vector into CPU register

			movups xmm0, [eax]        //move to xmm0 so we have sse access   
			mulps xmm0, col           // multiply the two vectors getting the new silhouetted color
			movups [silCol], xmm0     //store new color
		}

		return silCol; //return the processed color

	}


    void draw()
    {
        using namespace glm;
        Scene::draw();

        //float zoom = 1.0f / SCALE;
        //float aspect = width / (float) height;
        //const mat4 base = scale( {}, vec3( zoom, zoom * aspect, 1 ) );
		//update camera matrix
		camera.update();
		glm::mat4 cameraMatrix = camera.getCameraMatrix();

        glUseProgram( program );

        for ( auto x : entities )
        {
            Entity2& ntt = x.value;
            if ( auto drawable = ntt.pDrawable )
            {
                if ( !drawable->visible )
                    continue;

				mat4 mtx = cameraMatrix * translate( {}, vec3( ntt.position, 0 ) );
                mtx = rotate( mtx, ntt.rotation, vec3( 0, 0, 1 ) );

                glUniform( uMatrix, mtx );

                glUniform( uColor, usingASM? silhouetteASM(&drawable->color, drawable->interactive, &silhouette) : silhouetteCPP(&drawable->color, drawable->interactive, &silhouette));
				

                glUniform( uTexture, drawable->texture );
                glUniform( uFacingDirection, drawable->direction );
				glUniform( uInteractive, drawable->interactive );

                if ( auto anim = ntt.pAnimator )
                {
                    glUniform( uCurrentAnim, (float) anim->animState );
                    glUniform( uCurrentFrame, (float) anim->currentFrame );
                    glUniform( uMaxFrame, (float) anim->maxFrames );
                    glUniform( uMaxAnim, (float) anim->totalAnim );
                }
                else
                {
                    glUniform( uCurrentAnim, 0.f );
                    glUniform( uCurrentFrame, 0.f );
                    glUniform( uMaxFrame, 1.f );
                    glUniform( uMaxAnim, 1.f );
                }

                glBindVertexArray( drawable->vertexArray );
                glDrawArrays( GL_TRIANGLES, 0, drawable->count );
            }
				}
        }

	void add(EntityId eid, GraphicalComponent gfx)
    {
        //gfxComponents.add( eid, std::move( gfx ) );
    }

	void add(EntityId eid, PhysicalComponent fsx)
    {
        //fsxComponents.add( eid, std::move( fsx ) );
    }

	void add(InputListener lstn)
    {
		listeners.push_back(std::move(lstn));
    }

	void player_input(const InputManager& mngr, EntityId eid, int pindex);

    // Using bullet start position, the velocity  direction, and default facing direction.

	void fireBullet(Vec2 position, odin::Direction8Way direction, int pIndex);

	// Casts a ray from position in the direction given.
	// returns eid, normal to the collision, and distance of collision
	std::tuple<EntityBase*, Vec2, float> resolveBulletCollision(Vec2 position, Vec2 direction);
};


/*struct EntityView
{
    EntityId    eid;
    LevelScene* pScene;

    EntityView( EntityId eid, LevelScene* game )
        : eid( eid )
        , pScene( game )
    {
    }

    void attach( GraphicalComponent gfx )
    {
        pScene->gfxComponents[ eid ] = std::move( gfx );
    }

    void attach( PhysicalComponent fsx )
    {
        pScene->fsxComponents[ eid ] = std::move( fsx );
    }

    void attach( AnimatorComponent fsx )
    {
        pScene->animComponents[ eid ] = std::move( fsx );
    }

    void detach( ComponentType type )
    {
        switch ( type )
        {
        case ComponentType::Graphical:
            pScene->gfxComponents.remove( eid );
            break;
        case ComponentType::Physical:
            pScene->fsxComponents.remove( eid );
            break;
        case ComponentType::Animator:
            pScene->animComponents.remove( eid );
            break;
        }
    }

    GraphicalComponent* gfxComponent()
    {
        auto itr = pScene->gfxComponents.search( eid );
        return itr ? (GraphicalComponent*) itr : nullptr;
    }

    PhysicalComponent* fsxComponent()
    {
        auto itr = pScene->fsxComponents.search( eid );
        return itr ? (PhysicalComponent*) itr : nullptr;
    }

    AnimatorComponent* animComponent()
    {
        auto itr = pScene->animComponents.search( eid );
        return itr ? (AnimatorComponent*) itr : nullptr;
    }

};*/

inline void LevelScene::player_input(const InputManager& mngr, EntityId eid, int pindex)
{
	if (!entities.search(eid))
        return;

    Entity2& ntt = entities[ eid ];

	if (!players[pindex].alive)
		return;

	//arm
    Entity2& arm_ntt = entities[ { "playes", (uint16_t)pindex } ];
	GraphicalComponent& arm_gfx = *arm_ntt.pDrawable;
	AnimatorComponent& arm_anim = *arm_ntt.pAnimator;

    b2Body& body = *ntt.pBody;
    GraphicalComponent& gfx = *ntt.pDrawable;
    AnimatorComponent& anim = *ntt.pAnimator;
    
	if (!players[pindex].active)
		players[pindex].init(&gfx, &anim, &body, &arm_gfx, &arm_anim, arm_ntt.pBody);

    Vec2 vel = body.GetLinearVelocity();
    float maxSpeed = 7.5f;
    float actionLeft = mngr.isKeyDown(SDLK_LEFT) ? 1.f : 0.f;
    float actionRight = mngr.isKeyDown(SDLK_RIGHT) ? 1.f : 0.f;

    //adjust facing direction FOR KEYBOARD ONLY
	if (actionLeft) {
		gfx.direction = odin::LEFT;
		arm_gfx.direction = odin::LEFT;
	}
	if (actionRight) {
		gfx.direction = odin::RIGHT;
		arm_gfx.direction = odin::RIGHT;
	}

    Vec2 aimDir = mngr.gamepads.leftStick( pindex );
    aimDir.y = -aimDir.y;

    if ( glm::length( aimDir.glmvec2 ) < 0.25f )
        aimDir = {0, 0};

	//aim the arm graphics
	odin::Direction8Way aimDirection = players[pindex].aimArm(aimDir);

	if (actionLeft == 0 && actionRight == 0 && aimDir.x == 0)
    {
        //pFixt->SetFriction( 2 );
        vel.x = tween<float>(vel.x, 0, 16 * (1 / 60.0f));
    }
    else if ( actionLeft || actionRight || glm::length( aimDir.glmvec2 ) > 0.55f )
    {
        //pFixt->SetFriction( 0 );
        vel.x += aimDir.x * (20 + 5) * (1 / 60.0); // for use w/gamepad

        vel.x -= actionLeft * (20 + 5) * (1 / 60.0f);
        vel.x += actionRight * (20 + 5) * (1 / 60.0f);
        vel.x = glm::clamp(vel.x, -maxSpeed, +maxSpeed);
    }

	if (mngr.wasKeyPressed(SDLK_UP)) {
		vel.y = 12;
	}

	if (mngr.wasKeyReleased(SDLK_UP) && vel.y > 0) {
		vel.y *= 0.6f;
	}

	if (mngr.gamepads.wasButtonPressed(pindex, SDL_CONTROLLER_BUTTON_A)) {
		if (players[pindex].doubleJump < 2)
		{
		vel.y = 12;
			players[pindex].doubleJump++;
	}
	}

	if (mngr.gamepads.wasButtonReleased(pindex, SDL_CONTROLLER_BUTTON_A) && vel.y > 0)
		vel.y *= 0.6f;

    // Handle Duck input on button X
    if (mngr.gamepads.wasButtonPressed(pindex, SDL_CONTROLLER_BUTTON_X))
    {

    }
    if (mngr.gamepads.wasButtonReleased(pindex, SDL_CONTROLLER_BUTTON_X))
    {

    }

    //float rTrigger = mngr.gamepads.rightTrigger( pindex );

    // Handle Shoot input on button B
    if ( mngr.gamepads.didRightTriggerCross( pindex, 0.85 ) || mngr.gamepads.wasButtonPressed(pindex, SDL_CONTROLLER_BUTTON_B) && players[pindex].respawning <= 0)
    {
		//arm_anim.play = true;
		//arm_anim.currentFrame = 1;
		
		fireBullet(ntt.position, aimDirection, pindex);
    }
    if (mngr.gamepads.wasButtonReleased(pindex, SDL_CONTROLLER_BUTTON_B))
    {

    }

	if (mngr.gamepads.wasButtonPressed(pindex, SDL_CONTROLLER_BUTTON_START))
	{
		if (gameOver) {
			this->expired = true;
		}
		
	}
	
    //for testing audio
	if ( pindex == 0 && mngr.wasKeyPressed(SDLK_SPACE)) {
		arm_anim.play = true;
		arm_anim.currentFrame = 1;
		fireBullet(ntt.position, aimDirection, pindex);
	}
    if (mngr.wasKeyPressed(SDLK_1))
        pAudioEngine->setEventParameter("event:/Music/EnergeticTheme", "Energy", 0.0); //low energy test
    if (mngr.wasKeyPressed(SDLK_2))
        pAudioEngine->setEventParameter("event:/Music/EnergeticTheme", "Energy", 1.0); //high energy test

	if (mngr.wasKeyPressed(SDLK_KP_8))
		pAudioEngine->changeMasterVolume(0.1);
	if (mngr.wasKeyPressed(SDLK_KP_2))
		pAudioEngine->changeMasterVolume(-0.1);

    body.SetLinearVelocity(vel);
}

// Casts ray from position in specified direction
// returns eid, normal to the collision, and distance from position
std::tuple<EntityBase*, Vec2, float> LevelScene::resolveBulletCollision(Vec2 position, Vec2 direction) {
	// buffer value
	float delta = 0.001;
	
	//set up input
	b2RayCastInput input;
	input.p1 = position;
    input.p2 = Vec2( position.glmvec2 + glm::normalize( direction.glmvec2 ) * bulletRange );
    //{
    //    position.x + direction.x * bulletRange, position.y + direction.y * bulletRange
    //};
	input.maxFraction = 1;

	//check every fixture of every body to find closest
	float closestFraction = 1; //start with end of line as p2
	b2Vec2 intersectionNormal(0, 0);

    EntityBase* pEntityBase = nullptr;

    for ( b2Body* body = b2world.GetBodyList(); body; body = body->GetNext() )
    {
		for ( b2Fixture* f = body->GetFixtureList(); f; f = f->GetNext() )
        {
			b2RayCastOutput output;
			if ( !f->RayCast( &output, input, 0 ) )
				continue;
			
            if ( !(f->GetFilterData().maskBits & BULLET) )
				continue;

			if (output.fraction < closestFraction && output.fraction > delta)
            {
				closestFraction = output.fraction;
				intersectionNormal = output.normal;
                pEntityBase = (EntityBase*) body->GetUserData();
				//eid = x.key;
			}
		}
		
	}
	return std::make_tuple( pEntityBase, intersectionNormal, (closestFraction * bulletRange) * 10 );

}

inline void LevelScene::fireBullet(Vec2 position, odin::Direction8Way direction, int pIndex)
{
	if (players[pIndex].bulletCount <= 0) {
		pAudioEngine->playEvent("event:/Desperado/Empty");
		return;
	}

	//animate arm
	players[pIndex].arm_anim->switchAnimState(1);
	players[pIndex].arm_anim->loop = false;
	players[pIndex].aiming = true;
	players[pIndex].delay = 15;

	Player::lastShooterPoints = players[pIndex].points;

	players[pIndex].bulletCount--;

	float length = 100.f;
	float rotation = 0;
	Vec2 offset = { 0,0 };

	//play sound using multithreaded audio buffer copying
	if(!pAudioEngine->getMute()) //if not muted
		playSound("Audio/FX/Shot.wav", 127);
	//play sound using FMOD
    //pAudioEngine->playEvent("event:/Desperado/Shoot");

	// id of the entity hit, normal to the collision, distance to target
	std::tuple<EntityBase*, Vec2, float> collisionData;
	
	// Determine the offset and rotation of the bullet graphic based on the aim direction
    glm::vec2 pos = position;
    pos /= 10.0f;
	switch (direction) {
	case odin::NORTH_WEST:
		collisionData = resolveBulletCollision(pos, { -1,1 });
		length = std::get<2>(collisionData) - 20;
		rotation = -PI / 4;
		offset = { SIN45 * -length / 2 - BULLET_PADDING_ANGLED, SIN45 * length / 2 + BULLET_PADDING_ANGLED };
		break;
	case odin::NORTH_EAST:
		collisionData = resolveBulletCollision(pos, { 1,1 });
		length = std::get<2>(collisionData) - 20;
		rotation = PI / 4;
		offset = { SIN45 * length / 2 + BULLET_PADDING_ANGLED, SIN45 * length / 2 + BULLET_PADDING_ANGLED };
		break;
	case odin::SOUTH_WEST:
		collisionData = resolveBulletCollision(pos, { -1,-1 });
		length = std::get<2>(collisionData) - 20;
		rotation = PI / 4;
		offset = { SIN45 * -length / 2 - BULLET_PADDING_ANGLED, SIN45 * -length / 2 - BULLET_PADDING_ANGLED  + 7};
		break;
	case odin::SOUTH_EAST:
		collisionData = resolveBulletCollision(pos, { 1,-1 });
		length = std::get<2>(collisionData) - 20;
		rotation = -PI / 4;
		offset = { SIN45 * length / 2 + BULLET_PADDING_ANGLED, SIN45 * -length / 2 - BULLET_PADDING_ANGLED + 7};
		break;
	case odin::NORTH:
		collisionData = resolveBulletCollision(pos, { 0,1 });
		length = std::get<2>(collisionData) - 20;
		rotation = PI / 2;
		offset = { 4, length / 2 + BULLET_PADDING};
		break;
	case odin::SOUTH:
		collisionData = resolveBulletCollision(pos, { 0, -1 });
		length = std::get<2>(collisionData) - 20;
		rotation = -PI / 2;
		offset = { 4, -length / 2 - BULLET_PADDING };
		break;
	case odin::WEST:
		collisionData = resolveBulletCollision(pos, { -1,0 });
		length = std::get<2>(collisionData) - 20;
		offset = { -length / 2 - BULLET_PADDING, 4 };
		break;
	case odin::EAST:
		collisionData = resolveBulletCollision(pos, { 1,0 });
		length = std::get<2>(collisionData) - 20;
		offset = { length / 2 + BULLET_PADDING, 4 };
		break;
	default:
		break;
	}

    auto nttBase = std::get<0>( collisionData );

    if ( nttBase != nullptr )
        nttBase->onEvent( 1 );

    EntityId bid("bulleq", _bulletCount);

    Entity2& bullet = entities[ bid ];
    bullet.position = position;
	bullet.setBase(EntityBullet{ &players[pIndex] });

    //bullet.pDrawable = newGraphics( GraphicalComponent::makeRect( 1, 1, { 0, 0, 0 } ) );

    glm::vec2 off = offset;

    b2BodyDef bodyDef;
    bodyDef.position = Vec2( (position.glmvec2 / 10.f) + glm::normalize( off ) * 2.0f );
    bodyDef.linearVelocity = Vec2( glm::normalize( off ) * 1000.0f );
    bodyDef.type = b2_dynamicBody;
    bodyDef.gravityScale = 0;
    bodyDef.bullet = true;
    bodyDef.userData = bullet.base();


    //bullet.pBody = newBody( bodyDef );
    auto circ = PhysicalComponent::makeCircle( 0.05, b2world, bodyDef );
    bullet.pBody = circ.pBody;
    circ.pBody = nullptr;

    EntityId eid("bullet", _bulletCount);

    Entity2& bullet2 = entities[ eid ];
    bullet2.position = Vec2( position + offset );
    bullet2.rotation = rotation;

    bullet2.pDrawable = newGraphics( GraphicalComponent::makeRect(
        length, 8.0f, { 1, 1, 1 }, 1, { length / bulletRange, 1 } ));
    bullet2.pDrawable->texture = BULLET_TEXTURE;

    bullet2.pAnimator = newAnimator( AnimatorComponent( { 8 }, odin::FADEOUT ) );

    ++_bulletCount;

	if(!gameOver)
		camera.shake();

    //return EntityView(bid, this);

	}
