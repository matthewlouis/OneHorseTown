// Andrew Meckling
#pragma once

#include "Allocators.hpp"

namespace context_allocator
{
    IAllocator& get();
    void push( IAllocator& allocator );
    void push( std::nullptr_t );
    void pop();
}

#define context_alloc( TYPE ) ALLOC( context_allocator::get(), TYPE )
#define context_alloc_n( TYPE, LENGTH ) ALLOC_N( context_allocator::get(), TYPE, LENGTH )
#define context_dealloc( BLOCK ) DEALLOC( context_allocator::get(), BLOCK )
