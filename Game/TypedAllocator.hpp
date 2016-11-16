// Andrew Meckling
#pragma once

#include "Allocators.hpp"

#include <bitset>

template< typename T, size_t Max >
class TypedAllocator
{
public:

    using Type = T;
    static constexpr size_t SIZE = Max;

    std::bitset< SIZE > occupancy;
    Type slots[ SIZE ];

    Blk allocate( size_t size = sizeof( Type ) )
    {
        assert( size == sizeof( Type ) );

        size_t i = 0;
        while ( i < SIZE && occupancy.test( i ) )
            ++i;

        if ( i == SIZE )
            return nullptr;

        occupancy.set( i );
        return slots + i;
    }

    void deallocate( Blk blk )
    {
        if ( blk.ptr == nullptr )
            return;

        assert( owns( blk.ptr ) );

        size_t i = ((Type*) blk.ptr) - slots;
        occupancy.set( i, false );
    }

    constexpr bool owns( void* ptr )
    {
        return slots <= ptr && ptr < slots + SIZE;
    }
};
