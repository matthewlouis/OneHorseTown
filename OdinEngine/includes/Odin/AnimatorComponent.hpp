// Andrew Meckling
#pragma once

#include <vector>
#include <array>

namespace odin
{

    struct AnimatorComponent
    {
        static constexpr size_t MAX_ANIMATIONS = 10;

		bool play = true; //play animation
		bool loop = true; //loop animation

        int animState = 0;
        int currentFrame = 0;
        int maxFrames; //the length of the longest animation on sprite sheet
        int totalAnim; //the total number of anim states

        int animLengths[ MAX_ANIMATIONS ]; //array of length values - index == anim loop, val == length of loop in frames if 0, not animated

        int _frameDelay = 0; //how many draws before changing animation frame

        AnimatorComponent( std::initializer_list< int > il )
            : maxFrames( std::max( il ) )
            , totalAnim( il.size() )
        {
            std::copy( il.begin(), il.end(), animLengths );
        }

        //increment current frame to draw
        void incrementFrame() 
        {
            if ( play && _frameDelay++ > 3 )
            {
                ++currentFrame %= animLengths[ animState ];
                _frameDelay = 0;
				if (!loop && currentFrame == 0) {
					play = false; //if not looped, then stop after playing once
				}
            }
        }

        void switchAnimState( int state )
        {
            animState = state;
            currentFrame %= animLengths[ state ];
        }

    };

} // namespace odin