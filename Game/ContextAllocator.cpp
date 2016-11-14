// Andrew Meckling
#include "ContextAllocator.hpp"

#include <vector>

thread_local std::vector< IAllocator* > allocator_pool;

IAllocator& context_allocator::get()
{
    static PolymorphicAllocator< Mallocator > default_allocator;
    return allocator_pool.empty() ? default_allocator
        : *allocator_pool.back();
}

void context_allocator::push( IAllocator& allocator )
{
    allocator_pool.push_back( &allocator );
}

void context_allocator::push( nullptr_t )
{
    static PolymorphicAllocator< NullAllocator > null_allocator;
    allocator_pool.push_back( &null_allocator );
}

void context_allocator::pop()
{
    allocator_pool.pop_back();
}
