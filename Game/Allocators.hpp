// Andrew Meckling
#pragma once

#include <cstring>
#include <cassert>
#include <malloc.h>

#define ALLOC( ALLOCATOR, TYPE ) \
    new( (ALLOCATOR).allocate( sizeof(TYPE) ).ptr ) TYPE

#define ALLOC_N( ALLOCATOR, TYPE, LENGTH ) \
    Array< TYPE >{ 0, LENGTH } \
    * new( (ALLOCATOR).allocate( sizeof(TYPE) * LENGTH ).ptr ) TYPE[ LENGTH ]

#define DEALLOC( ALLOCATOR, BLOCK ) (ALLOCATOR).deallocate( BLOCK )

using byte = unsigned char;

struct Blk
{
    void*  ptr;
    size_t size;

    Blk() = default;

    constexpr Blk( void* ptr, size_t size )
        : ptr( ptr ), size( size )
    {
    }

    template< typename T >
    constexpr Blk( T* ptr )
        : ptr( ptr ), size( sizeof( T ) )
    {
    }

    void set( byte val )
    {
        std::memset( ptr, val, size );
    }
};

template< typename T >
struct Array
{
    T*     ptr;
    size_t count;

    constexpr operator Blk()
    {
        return { ptr, sizeof( T ) * count };
    }

    void fill( const T& val )
    {
        std::fill( ptr, ptr + count, val );
    }
};

template< typename T >
constexpr Array< T > operator *( const Array< T >& arr, T* ptr )
{
    return { ptr, arr.count };
}

constexpr size_t round_to_alignment( size_t size, size_t align )
{
    return size % align ? size + align - size % align : size;
}


struct IAllocator
{
    virtual Blk allocate( size_t ) = 0;
    virtual void deallocate( Blk ) = 0;
};


class Mallocator
{
public:

    Blk allocate( size_t size )
    {
        return { malloc( size ), size };
    }

    void deallocate( Blk blk )
    {
        free( blk.ptr );
    }
};


class NullAllocator
{
public:

    constexpr Blk allocate( size_t )
    {
        return { nullptr, 0 };
    }

    void deallocate( Blk b ) noexcept
    {
        assert( owns( b.ptr ) );
    }

    constexpr bool owns( void* ptr )
    {
        return ptr == nullptr;
    }
};


template< typename Allocator >
class VirtualAllocator
    : public IAllocator
    , protected Allocator
{
public:

    Blk allocate( size_t n )
    {
        return Allocator::allocate( n );
    }

    void deallocate( Blk b )
    {
        return Allocator::deallocate( b );
    }

    bool owns( void* p )
    {
        return Allocator::owns( p );
    }
};



template< class Primary, class Fallback >
class FallbackAllocator
    : protected Primary
    , protected Fallback
{
    using P = Primary;
    using F = Fallback;
public:

    Blk allocate( size_t n )
    {
        Blk r = P::allocate( n );
        if ( !r.ptr ) r = F::allocate( n );
        return r;
    }

    void deallocate( Blk b )
    {
        if ( P::owns( b ) ) P::deallocate( b );
        else F::deallocate( b );
    }

    bool owns( void* p )
    {
        return P::owns( p ) || F::owns( p );
    }
};



template< size_t Threshold, class Smaller, class Larger >
class ThresholdAllocator
    : protected Smaller
    , protected Larger
{
    using S = Smaller;
    using L = Larger;
public:

    static constexpr size_t THRESHOLD = Threshold;

    Blk allocate( size_t size )
    {
        return size <= THRESHOLD ? S::allocate( size )
            : L::allocate( size );
    }

    void deallocate( Blk blk )
    {
        if ( blk.size <= THRESHOLD ) S::deallocate( blk );
        else L::deallocate( blk );
    }

    bool owns( void* ptr )
    {
        return S::owns( ptr ) || L::owns( ptr );
    }
};



template< size_t Bytes, size_t Align >
class StackAllocator
{
public:

    static constexpr size_t SIZE = Bytes;
    static constexpr size_t ALIGNMENT = Align;

    byte* ptr = memory;
    byte memory[ SIZE ];

    Blk allocate( size_t size )
    {
        size_t n1 = _roundToAligned( size );
        if ( n1 > memory + SIZE - ptr )
            return { nullptr, 0 };

        Blk result = { ptr, size };
        ptr += n1;
        return result;
    }

    void deallocate( Blk blk )
    {
        if ( blk.ptr + _roundToAligned( blk.size ) == ptr )
            ptr = blk.ptr;
    }

    constexpr bool owns( void* ptr )
    {
        return memory <= ptr && ptr < memory + SIZE;
    }

private:

    constexpr size_t _roundToAligned( size_t size )
    {
        return round_to_alignment( size, ALIGNMENT );
    }
};


template< size_t Bytes, size_t Align = sizeof( int ) >
class BitsetAllocator
{
public:

    using Word = unsigned long long;

    static constexpr size_t SIZE = Bytes;
    static constexpr size_t ALIGNMENT = Align;
    static constexpr size_t BITS_PER_WORD = 8 * sizeof( Word );

    Word bitset[ SIZE / ALIGNMENT / BITS_PER_WORD + 1 ] = { 0 };
    byte memory[ SIZE ];

    Blk allocate( size_t size ) noexcept
    {
        size_t rounded_size = _roundToAligned( size );
        size_t bitlen = rounded_size / ALIGNMENT;

        for ( size_t pos = 0; pos < SIZE - rounded_size; )
        {
            size_t bitpos = pos / ALIGNMENT;
            if ( _checkRange( bitpos, bitlen, false, &pos ) )
            {
                _setRange( bitpos, bitlen, true );
                #ifdef _DEBUG
                std::memset( memory + pos, 0xbb, size );
                #endif
                return { memory + pos, size };
            }
        }
        return { nullptr, size };
    }

    void deallocate( Blk blk ) noexcept
    {
        if ( blk.ptr == nullptr )
            return;

        assert( owns( blk.ptr ) );

        #ifdef _DEBUG
        blk.set( 0xdd );
        #endif

        size_t pos = (byte*) blk.ptr - memory;
        _setRange( pos / ALIGNMENT, _roundToAligned( blk.size ) / ALIGNMENT, false );
    }

    constexpr bool owns( void* ptr )
    {
        return memory <= ptr && ptr < memory + SIZE;
    }

private:

    constexpr size_t _roundToAligned( size_t size )
    {
        return round_to_alignment( size, ALIGNMENT );
    }

    void _setRange( size_t pos, size_t len, bool bit ) noexcept
    {
        Word* pWord0 = &bitset[ pos / BITS_PER_WORD ];
        size_t idx0 = pos % BITS_PER_WORD;

        Word* pWord1 = &bitset[ (pos + len) / BITS_PER_WORD ];
        size_t idx1 = BITS_PER_WORD - (pos + len) % BITS_PER_WORD;

        for ( Word* pWord = pWord0; pWord <= pWord1; ++pWord )
        {
            Word mask = ~0;

            if ( pWord == pWord0 )
                mask = (mask >> idx0) << idx0;

            if ( pWord == pWord1 )
                mask = (mask << idx1) >> idx1;

            if ( bit )
                *pWord |= mask;
            else
                *pWord &= ~mask;
        }
    }

    bool _checkRange( size_t pos, size_t len, bool bit,
                      size_t* outpos = nullptr ) const noexcept
    {
        const Word* pWord0 = &bitset[ pos / BITS_PER_WORD ];
        size_t idx0 = pos % BITS_PER_WORD;

        const Word* pWord1 = &bitset[ (pos + len) / BITS_PER_WORD ];
        size_t idx1 = BITS_PER_WORD - (pos + len) % BITS_PER_WORD;

        for ( const Word* pWord = pWord0; pWord <= pWord1; ++pWord )
        {
            Word mask = ~0;

            if ( pWord == pWord0 )
                mask = (mask >> idx0) << idx0;

            if ( pWord == pWord1 )
                mask = (mask << idx1) >> idx1;

            if ( (*pWord & mask) != (bit ? mask : 0) )
            {
                if ( outpos )
                {
                    size_t offset = _skip( *pWord, mask, bit ) * ALIGNMENT;
                    size_t chunk = (bitset - pWord) * BITS_PER_WORD;
                    *outpos = _roundToAligned( chunk + offset );
                }
                return false;
            }
        }
        return true;
    }

    static size_t _skip( Word word, Word mask, bool bit )
    {
        size_t offset;
        if ( bit ) {
            offset =  _first_0( word & mask ); // Skip 1s
            offset += _first_1( word >> offset ); // Skip 0s
        } else {
            offset =  _first_1( word & mask ); // Skip 0s
            offset += _first_0( word >> offset ); // Skip 1s
        }
        return std::min( offset, BITS_PER_WORD );
    }

    static size_t _first_1( Word word )
    {
        size_t pos = 0;
        while ( !(word & 1) && ++pos < BITS_PER_WORD )
            word >>= 1;
        return pos;
    }

    static size_t _first_0( Word word )
    {
        size_t pos = 0;
        while ( (word & 1) && ++pos < BITS_PER_WORD )
            word >>= 1;
        return pos;
    }

};
