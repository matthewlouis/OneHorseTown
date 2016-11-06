// Andrew Meckling
#pragma once

#include <vector>
#include <array>

namespace odin
{
	enum AnimationType {
		SPRITESHEET,
		FADEOUT
	};
    struct AnimatorComponent
    {
        static constexpr size_t MAX_ANIMATIONS = 5;

		AnimationType type;
        int animState = 0;
        int currentFrame = 0;
        int maxFrames; //the length of the longest animation on sprite sheet
        int totalAnim; //the total number of anim states

        int animLengths[ MAX_ANIMATIONS ]; //array of length values - index == anim loop, val == length of loop in frames if 0, not animated

        int _frameDelay = 0; //how many draws before changing animation frame

        AnimatorComponent( std::initializer_list< int > il, AnimationType t = SPRITESHEET )
            : maxFrames( std::max( il ) )
            , totalAnim( il.size() )
			, type(t)
        {
            std::copy( il.begin(), il.end(), animLengths );
        }

        //increment current frame to draw
        void incrementFrame() 
        {
            if ( _frameDelay++ > 3 )
            {
                ++currentFrame %= animLengths[ animState ];
                _frameDelay = 0;
            }
        }

        void switchAnimState( int state )
        {
            animState = state;
            currentFrame %= animLengths[ state ];
        }

    };

} // namespace odin