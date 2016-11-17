#pragma once

#include <SDL/SDL.h>
#include <glm/glm.hpp>

#include <bitset>
#include <array>
#include <functional>

#include "BinarySearchMap.hpp"
#include "EntityId.hpp"
#include "Util.h"

namespace odin
{

	enum Direction8Way {
		NORTH = 0,
		NORTH_EAST,
		EAST,
		SOUTH_EAST,
		SOUTH,
		SOUTH_WEST,
		WEST,
		NORTH_WEST,
	};

    // Manages gamepads connected to the device. Does not handle its
    // own input detection, merely provides convenient access to the 
    // connected controllers and their states.
    class ControllerManager
    {
    public:

        static constexpr int MAX_PLAYERS = 4;

        using PlayerIndex = int;
        using ControllerMap = BinarySearchMap< SDL_JoystickID, SDL_GameController* >;
        using PlayerList = std::array< SDL_JoystickID, MAX_PLAYERS >;

        using ButtonStates = std::array< std::bitset< SDL_CONTROLLER_BUTTON_MAX >, MAX_PLAYERS >;
        using AxisStates = std::array< glm::vec2, MAX_PLAYERS >;
        //using TiggerStates = std::array< float, MAX_PLAYERS >;
		
		ControllerMap controllers; // Maps joystick ids to game controllers.
        PlayerList    players;     // Stores the joystick id of each player.

        ButtonStates  currButtons; // Represents the current down state of each button on each controller.
        ButtonStates  prevButtons; // Represents the previous down state of each button on each controller.
		AxisStates	  leftAxis;
        AxisStates    rightAxis;
        AxisStates    triggerAxis;
        //AxisStates    prevLeftAxis;
        //AxisStates    prevRightAxis;
        AxisStates    prevTriggerAxis;

        ControllerManager()
            : controllers( { {-1, nullptr} }, MAX_PLAYERS )
            , currButtons()
            , prevButtons()
        {
            players.fill( -1 );
        }

        // Gets the player index that a given joystick id is mapped to.
        // Returns -1 if the joystick id isn't mapped to a player index.
        PlayerIndex findPlayerIndex( SDL_JoystickID id ) const
        {
            for ( PlayerIndex i = 0; i < MAX_PLAYERS; ++i )
                if ( players[ i ] == id )
                    return i;
            return -1;
        }

        // Gets the controller mapped to a given player index.
        // Returns nullptr if no controller is mapped to the player index.
        SDL_GameController* getController( PlayerIndex index ) const
        {
            return controllers[ players[ index ] ];
        }

        // Activates a controller at a specific joystick index.
        // Returns the player index [0, MAX_PLAYERS) of the added
        // controller on success. Returns -1 on failure.
        PlayerIndex addController( int joyDeviceIndex )
        {
            auto p_controller = SDL_GameControllerOpen( joyDeviceIndex );
            if ( p_controller == nullptr )
                return -1; // controller could not be opened.

            // get the joystick id of the newly opened game controller.
            SDL_Joystick* p_joystick = SDL_GameControllerGetJoystick( p_controller );
            SDL_JoystickID joy_id = SDL_JoystickInstanceID( p_joystick );

            // map the joystick id to the controller.
            if ( controllers.add( joy_id, p_controller ) )
            {
                // map the joystick id to the first empty player slot.
                for ( PlayerIndex i = 0; i < MAX_PLAYERS; ++i )
                {
                    if ( players[ i ] == -1 )
                    {
                        players[ i ] = joy_id;
                        return i;
                    }
                }
                // not enough controller slots
                controllers.remove( joy_id );
            }
            SDL_GameControllerClose( p_controller );
            return -1; // controller could not be added.
        }

        // Deactivates a controller with a specific joystick id.
        // Returns the player index [0, MAX_PLAYERS) of the controller 
        // that was removed on success. Returns -1 on failure.
        PlayerIndex removeController( SDL_JoystickID joy_id )
        {
            SDL_GameControllerClose( controllers[ joy_id ] );
            if ( controllers.remove( joy_id ) )
            {
                // unmap the joystick id from its player slot.
                for ( PlayerIndex i = 0; i < MAX_PLAYERS; ++i )
                {
                    if ( players[ i ] == joy_id )
                    {
                        players[ i ] = -1;
                        return i;
                    }
                }
            }
            return -1; // controller wasn't previously connected.
        }

		// Returns true if ANYONE pressed a given button
		bool didAnyonePressButton(SDL_GameControllerButton button) const{
			bool pressed = false;
			for (int i = 0; i < MAX_PLAYERS; ++i) {
				pressed | currButtons[i][button];
			}
			return pressed;
		}

        // Returns true if a specific button on a specific controller just
        // changed from being unpressed to pressed.
        bool wasButtonPressed( PlayerIndex idx, SDL_GameControllerButton button ) const
        {
            return currButtons[ idx ][ button ] && !prevButtons[ idx ][ button ];
        }

        // Returns true if a specific button on a specific controller just
        // changed from being pressed to unpressed.
        bool wasButtonReleased( PlayerIndex idx, SDL_GameControllerButton button ) const
        {
            return !currButtons[ idx ][ button ] && prevButtons[ idx ][ button ];
        }

        // Returns true if a specific button on a specific controller is pressed.
        bool isButtonDown( PlayerIndex idx, SDL_GameControllerButton button ) const
        {
            return currButtons[ idx ][ button ];
        }

		// Returns the x axis for the current controller based on the player index
		float joystickAxisX( PlayerIndex idx ) const
		{
			return leftAxis[ idx ].x;
		}

		// Returns the y axis for the current controller based on the player index
		float joystickAxisY( PlayerIndex idx ) const
		{
			return leftAxis[ idx ].y;
		}

		glm::vec2 joystickDir( PlayerIndex idx ) const
		{
			return leftAxis[ idx ];
		}

        glm::vec2 leftAxisXY( PlayerIndex idx ) const
        {
            return leftAxis[ idx ];
        }

        glm::vec2 rightAxisXY( PlayerIndex idx ) const
        {
            return rightAxis[ idx ];
        }

        float leftTrigger( PlayerIndex idx ) const
        {
            return triggerAxis[ idx ].x;
        }

        float rightTrigger( PlayerIndex idx ) const
        {
            return triggerAxis[ idx ].y;
        }

        bool didLeftTriggerCross( PlayerIndex idx, float threshold ) const
        {
            float thr = std::abs( threshold );

            if ( threshold > 0 )
                return (triggerAxis[ idx ].x >= thr && prevTriggerAxis[ idx ].x < thr);

            return (triggerAxis[ idx ].x <= thr && prevTriggerAxis[ idx ].x > thr);
        }

        bool didRightTriggerCross( PlayerIndex idx, float threshold ) const
        {
            float thr = std::abs( threshold );

            if ( threshold > 0 )
                return (triggerAxis[ idx ].y >= thr && prevTriggerAxis[ idx ].y < thr);

            return (triggerAxis[ idx ].y <= thr && prevTriggerAxis[ idx ].y > thr);
        }

    };

    class InputManager;

    using InputListener = std::function< void( const InputManager& ) >;

    // Sorted array of SDL_Keycode mappings KEYS[ n ] -> SDL_Keycode
    constexpr SDL_Keycode KEYS[] = {
        SDLK_UNKNOWN,
        SDLK_BACKSPACE,
        SDLK_TAB,
        SDLK_RETURN,
        SDLK_ESCAPE,
        SDLK_SPACE,
        SDLK_EXCLAIM,
        SDLK_QUOTEDBL,
        SDLK_HASH,
        SDLK_DOLLAR,
        SDLK_PERCENT,
        SDLK_AMPERSAND,
        SDLK_QUOTE,
        SDLK_LEFTPAREN,
        SDLK_RIGHTPAREN,
        SDLK_ASTERISK,
        SDLK_PLUS,
        SDLK_COMMA,
        SDLK_MINUS,
        SDLK_PERIOD,
        SDLK_SLASH,
        SDLK_0,
        SDLK_1,
        SDLK_2,
        SDLK_3,
        SDLK_4,
        SDLK_5,
        SDLK_6,
        SDLK_7,
        SDLK_8,
        SDLK_9,
        SDLK_COLON,
        SDLK_SEMICOLON,
        SDLK_LESS,
        SDLK_EQUALS,
        SDLK_GREATER,
        SDLK_QUESTION,
        SDLK_AT,
        SDLK_LEFTBRACKET,
        SDLK_BACKSLASH,
        SDLK_RIGHTBRACKET,
        SDLK_CARET,
        SDLK_UNDERSCORE,
        SDLK_BACKQUOTE,
        SDLK_a,
        SDLK_b,
        SDLK_c,
        SDLK_d,
        SDLK_e,
        SDLK_f,
        SDLK_g,
        SDLK_h,
        SDLK_i,
        SDLK_j,
        SDLK_k,
        SDLK_l,
        SDLK_m,
        SDLK_n,
        SDLK_o,
        SDLK_p,
        SDLK_q,
        SDLK_r,
        SDLK_s,
        SDLK_t,
        SDLK_u,
        SDLK_v,
        SDLK_w,
        SDLK_x,
        SDLK_y,
        SDLK_z,
        SDLK_CAPSLOCK,
        SDLK_F1,
        SDLK_F2,
        SDLK_F3,
        SDLK_F4,
        SDLK_F5,
        SDLK_F6,
        SDLK_F7,
        SDLK_F8,
        SDLK_F9,
        SDLK_F10,
        SDLK_F11,
        SDLK_F12,
        SDLK_PRINTSCREEN,
        SDLK_SCROLLLOCK,
        SDLK_PAUSE,
        SDLK_INSERT,
        SDLK_HOME,
        SDLK_PAGEUP,
        SDLK_DELETE,
        SDLK_END,
        SDLK_PAGEDOWN,
        SDLK_RIGHT,
        SDLK_LEFT,
        SDLK_DOWN,
        SDLK_UP,
        SDLK_NUMLOCKCLEAR,
        SDLK_KP_DIVIDE,
        SDLK_KP_MULTIPLY,
        SDLK_KP_MINUS,
        SDLK_KP_PLUS,
        SDLK_KP_ENTER, 
        SDLK_KP_1,
        SDLK_KP_2,
        SDLK_KP_3,
        SDLK_KP_4,
        SDLK_KP_5,
        SDLK_KP_6,
        SDLK_KP_7,
        SDLK_KP_8,
        SDLK_KP_9,
        SDLK_KP_0,
        SDLK_KP_PERIOD,
        SDLK_LCTRL,
        SDLK_LSHIFT,
        SDLK_LALT,
        SDLK_LGUI,
        SDLK_RCTRL,
        SDLK_RSHIFT,
        SDLK_RALT,
        SDLK_RGUI,
    };


	static constexpr int JOYSTICK_DEAD_ZONE = 10200;
    // Manages input events. Call pollEvents to update the stored state 
    // of the connected input devices (keyboard, controllers, etc.).
    class InputManager
    {
    public:

		//constants to divide the joystick into 8 directions
		static const double RAD_22_5;
		static const double RAD_67_5;
		static const double RAD_112_5;
		static const double RAD_157_5;

        static constexpr size_t NUM_KEYS = sizeof( KEYS ) / sizeof( SDL_Keycode );

        std::bitset< NUM_KEYS > _currKeys;
        std::bitset< NUM_KEYS > _prevKeys;

        ControllerManager       gamepads;

        // Maps an sdl keycode to an index in the range [0, NUM_KEYS).
        // Returns 0 if the keycode is not recognized.
        static size_t key_index( SDL_Keycode key )
        {
            auto itr = binary_search( KEYS, key );

            return itr == std::end( KEYS ) ? 0
                : std::distance( std::begin( KEYS ), itr );
        }

        // Calls SDL_PollEvent repeatedly to empty the event queue.
        // Updates the stored state of the connected input devices.
        void pollEvents()
        {
            return pollEvents( [](){} );
        }

        // Calls SDL_PollEvent repeatedly to empty the event queue.
        // Updates the stored state of the connected input devices.
        // Invokes the callback object if the SDL_QUIT event is detected.
        template< typename Callback >
        void pollEvents( Callback callback )
        {
            _prevKeys = _currKeys;
            std::copy( gamepads.currButtons.begin(),
                       gamepads.currButtons.end(),
                       gamepads.prevButtons.begin() );

            //gamepads.prevLeftAxis = gamepads.leftAxis;
            //gamepads.prevRightAxis = gamepads.rightAxis;
            gamepads.prevTriggerAxis = gamepads.triggerAxis;

            SDL_Event event;
            while ( SDL_PollEvent( &event ) )
            {
                switch ( event.type )
                {
                case SDL_CONTROLLERDEVICEADDED:
                {
                    int joy_index = event.cdevice.which;
                    int player_index = gamepads.addController( joy_index );

                    //#ifdef _DEBUG
                    if ( player_index == -1 )
                        printf( "Failed to register joystick location %i\n", joy_index );
                    else
                        printf( "Registered joystick location %i to player %i\n", joy_index, player_index );
                    //#endif
                    break;
                }
                case SDL_CONTROLLERDEVICEREMOVED:
                {
                    SDL_JoystickID joy_id = event.cdevice.which;
                    int player_index = gamepads.removeController( joy_id );

                    //#ifdef _DEBUG
                    if ( player_index == -1 )
                        printf( "Failed to unregister joystick id %i\n", joy_id );
                    else
                        printf( "Unregistered joystick id %i from player %i\n", joy_id, player_index );
                    //#endif
                    break;
                }
                case SDL_CONTROLLERAXISMOTION:
				{
					SDL_ControllerAxisEvent& caxis = event.caxis;
                    SDL_JoyAxisEvent& jaxis = event.jaxis;

					int playerNo = gamepads.findPlayerIndex( caxis.which );

                    if ( jaxis.axis < 2 )
                    {
                        gamepads.leftAxis[ playerNo ][ jaxis.axis ] = jaxis.value / 32767.f;
                    }
                    else if ( jaxis.axis < 4 )
                    {
                        gamepads.rightAxis[ playerNo ][ jaxis.axis % 2 ] = jaxis.value / 32767.f;
                    }
                    else if ( jaxis.axis < 6 )
                    {
                        gamepads.triggerAxis[ playerNo ][ jaxis.axis % 2 ] = jaxis.value / 32767.f;
                    }
					break;
				}
                case SDL_CONTROLLERBUTTONDOWN:
                case SDL_CONTROLLERBUTTONUP:
                {
                    SDL_ControllerButtonEvent& cbutton = event.cbutton;
                    int playerNo = gamepads.findPlayerIndex( cbutton.which );

                    gamepads.currButtons[ playerNo ][ cbutton.button ] = (cbutton.state == SDL_PRESSED);
                    #ifdef _DEBUG
                    /*printf( "P%i button %s: %i\n",
                            playerNo,
                            cbutton.state ? "down" : "up",
                            event.cbutton.button );*/
                    #endif
                    break;
                }
                case SDL_MOUSEMOTION:
                    //mouse movement - probably not needed
					//std::cout << event.motion.x << " " << event.motion.y << '\n';
                    break;
                case SDL_KEYDOWN:
                    _currKeys[ key_index( event.key.keysym.sym ) ] = true;
                    break;
                case SDL_KEYUP:
                    _currKeys[ key_index( event.key.keysym.sym ) ] = false;
                    break;
                case SDL_QUIT:
					//Matt: Allows us to close the program easily
					SDL_Quit();
					//exit(1); //Leaving this out so we can read the console before closing 
                    //std::invoke( callback );
                    break;
                }
            }
        }

        // Returns true if a specific key just changed from 
        // being unpressed to pressed.
        bool wasKeyPressed( SDL_Keycode key ) const
        {
            return _currKeys[ key_index( key ) ] && !_prevKeys[ key_index( key ) ];
        }

        // Returns true if a specific key just changed from 
        // being pressed to unpressed.
        bool wasKeyReleased( SDL_Keycode key ) const
        {
            return !_currKeys[ key_index( key ) ] && _prevKeys[ key_index( key ) ];
        }

        // Returns true if a specific key is pressed.
        bool isKeyDown( SDL_Keycode key ) const
        {
            return _currKeys[ key_index( key ) ];
        }
    };

	//takes an angle in rads and returns one of 8 directions
	static Direction8Way calculateDirection8Way(float angle) {

		if (angle < InputManager::RAD_22_5 && angle >= -InputManager::RAD_22_5)
			return EAST;
		if (angle < InputManager::RAD_67_5 && angle >= InputManager::RAD_22_5)
			return NORTH_EAST;
		if (angle < InputManager::RAD_112_5 && angle >= InputManager::RAD_67_5)
			return NORTH;
		if (angle < InputManager::RAD_157_5 && angle >= InputManager::RAD_112_5)
			return NORTH_WEST;
		if (angle < -InputManager::RAD_22_5 && angle >= -InputManager::RAD_67_5)
			return SOUTH_EAST;
		if (angle < -InputManager::RAD_67_5 && angle >= -InputManager::RAD_112_5)
			return SOUTH;
		if (angle < -InputManager::RAD_112_5 && angle >= -InputManager::RAD_157_5)
			return SOUTH_WEST;
		                      
		//else 
		return WEST;
	}

} // namespace odin
