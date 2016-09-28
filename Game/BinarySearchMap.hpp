// Andrew Meckling
#pragma once

#include <algorithm>
#include <initializer_list>

namespace meck
{

    template< typename KeyType, typename ValueType >
    struct MapEntry
    {
        KeyType   key;
        ValueType value;
    };

    // A dense binary searching map object. All keys are stored contiguously.
    // All values are stored contiguously.
    template< typename KeyType_, typename ValueType_ >
    class BinarySearchMap
    {
    public:

        using KeyType   = KeyType_;
        using ValueType = ValueType_;

        static constexpr size_t KEY_SIZE   = sizeof( KeyType );
        static constexpr size_t VALUE_SIZE = sizeof( ValueType );

        struct Iterator;
        struct ConstIterator;

        using EntryType           = MapEntry< KeyType, ValueType >;
        using InitializerListType = std::initializer_list< EntryType >;

        void*  _pData;
        size_t _capacity;
        size_t _count;

        // Allocates one block of memory for the keys and values.
        static void* _allocate( size_t capacity )
        {
            return new char[ (KEY_SIZE + VALUE_SIZE) * capacity ];
        }

        static void _deallocate( void* pData )
        {
            delete[] pData;
        }

        #define _pKeys _keys()

        KeyType* _keys()
        {
            return reinterpret_cast< KeyType* >( _pData );
        }

        const KeyType* _keys() const
        {
            return reinterpret_cast< const KeyType* >( _pData );
        }

        #define _pValues _values()

        ValueType* _values()
        {
            return reinterpret_cast< ValueType* >( _pKeys + _capacity );
        }

        const ValueType* _values() const
        {
            return reinterpret_cast< const ValueType* >( _pKeys + _capacity );
        }

        explicit BinarySearchMap( long long capacity = 12 )
            : _pData( capacity > 0 ? _allocate( capacity ) : throw "capacity must be positive" )
            , _capacity( capacity )
            , _count( 0 )
        {
        }

        BinarySearchMap( InitializerListType list, long long extra = 0 )
            : BinarySearchMap( extra >= 0 ? list.size() + extra : throw "extra capacity must be positive" )
        {
            bool sorted = true;
            const EntryType* prev = nullptr;
            for ( const EntryType& entry : list )
            {
                if ( sorted && prev != nullptr && prev->key > entry.key )
                    sorted = false;
                new( _pKeys + (&entry - list.begin()) ) KeyType( entry.key );
                prev = &entry
            }

            _count = list.size();
            if ( !sorted )
                std::sort( _pKeys, _pKeys + _count );

            for ( EntryType entry : list )
                new( search( entry.key ) ) ValueType( entry.value );
        }

        BinarySearchMap( const BinarySearchMap& copy )
            : BinarySearchMap( copy._capacity )
        {
            for ( size_t i = 0; i < copy._count; ++i )
                _construct( i, copy._pKeys[ i ], copy._pValues[ i ] );

            _count = copy._count;
        }

        BinarySearchMap( BinarySearchMap&& move )
            : _pData( move._pData )
            , _capacity( move._capacity )
            , _count( move._count )
        {
            move._pData = nullptr;
            move._count = 0;
            move._capacity = 0;
        }

        BinarySearchMap& operator =( const BinarySearchMap& copy )
        {
            if ( _capacity != copy._capacity )
            {
                clear();
                _deallocate( _pData );
                _pData    = _allocate( copy._capacity );
                _capacity = copy._capacity;
            }

            for ( size_t i = 0; i < copy._count; ++i )
                _construct( i, copy._pKeys[ i ], copy._pValues[ i ] );

            _count = copy._count;
        }

        BinarySearchMap& operator =( BinarySearchMap&& move )
        {
            std::swap( *this, move );
            return *this;
        }

        ~BinarySearchMap()
        {
            clear();
            _deallocate( _pData );
        }

        // Returns the number of elements held.
        size_t count() const
        {
            return _count;
        }

        // Returns the number of elements that can be held without reallocating memory.
        size_t capacity() const
        {
            return _capacity;
        }

        // Returns the size of allocated memory in bytes.
        size_t size() const
        {
            return (KEY_SIZE + VALUE_SIZE) * _capacity;
        }

        void clear()
        {
            while ( _count > 0 )
                _destroy( --_count );
        }

        ValueType& operator []( const KeyType& key )
        {
            Iterator itr = _count > 0 ? search( key ) : Iterator( nullptr, _pKeys );
            return itr ? *itr : *_insert( itr._index( this ), key, ValueType {} );
        }

        const ValueType& operator []( const KeyType& key ) const
        {
            ConstIterator itr = _count > 0 ? search( key ) : ConstIterator( nullptr, _pKeys );
            return itr ? *itr : throw "key not found";
        }

        // Performs a binary search over the keys.
        Iterator search( KeyType key )
        {
            const KeyType* lower = _pKeys;
            const KeyType* upper = _pKeys + (_count - 1);
            const KeyType* mid   = _pKeys + (_count / 2);

            while ( lower <= upper )
            {
                if ( *mid < key ) lower = mid + 1;
                else if ( *mid > key ) upper = mid - 1;
                else return Iterator( this, mid );

                mid = lower + (upper - lower) / 2;
            }

            return Iterator( nullptr, mid );
        }

        // Performs a binary search over the keys.
        ConstIterator search( KeyType key ) const
        {
            return const_cast< BinarySearchMap* >( this )->search( key );
        }

        void reallocate( size_t size )
        {
            if ( size == _capacity )
                return;

            BinarySearchMap tmp( size );
            tmp._count = std::min( _count, size );

            std::swap( *this, tmp );

            for ( size_t i = 0; i < _count; ++i )
                _construct( i,
                    std::move( tmp._pKeys[ i ] ),
                    std::move( tmp._pValues[ i ] ) );

            for ( size_t i = _count; i < tmp._count; i++ )
                tmp._destroy( i );

            tmp._count = 0;
        }

        ValueType* add( KeyType key, ValueType value )
        {
            Iterator itr = _count > 0 ? search( key ) : Iterator( nullptr, _pKeys );
            if ( itr )
                return nullptr;

            return (ValueType*) _insert( itr._index( this ), key, std::move( value ) );
        }

        bool remove( KeyType key )
        {
            Iterator itr = search( key );
            if ( !itr )
                return false;

            _erase( itr.index() );
            return true;
        }

        void _construct( size_t pos, KeyType key, ValueType value )
        {
            new(_pKeys + pos) KeyType( std::move( key ) );
            new(_pValues + pos) ValueType( std::move( value ) );
        }

        void _destroy( size_t pos )
        {
            _pKeys[ pos ].~KeyType();
            _pValues[ pos ].~ValueType();
        }

        Iterator _insert( size_t pos, KeyType key, ValueType value )
        {
            if ( pos > _count )
                throw "index out of bounds";

            // TODO: Improve efficiency when reallocating 
            // AND inserting to the middle.
            if ( _count == _capacity )
                reallocate( _capacity * 2 );

            // Shift chunk of data right when not appending
            for ( size_t i = _count; i > pos; --i )
            {
                _pKeys[ i ]   = std::move( _pKeys[ i - 1 ] );
                _pValues[ i ] = std::move( _pValues[ i - 1 ] );
            }

            _construct( pos, key, std::move( value ) );
            ++_count;

            return Iterator( this, _pKeys + pos );
        }

        Iterator _erase( size_t pos )
        {
            if ( pos > _count )
                throw "index out of bounds";

            // Shift chunk of data left when erasing from middle
            for ( size_t i = pos; i < _count - 1; ++i )
            {
                _pKeys[ i ]   = std::move( _pKeys[ i + 1 ] );
                _pValues[ i ] = std::move( _pValues[ i + 1 ] );
            }

            _destroy( --_count );

            return Iterator( this, _pKeys + pos );
        }

        #pragma region Iterators

        struct Iterator
        {
            const KeyType* pKey;

            BinarySearchMap* const _pMap;

            Iterator( BinarySearchMap* this_map, const KeyType* pKey )
                : pKey( pKey )
                , _pMap( this_map )
            {
            }

            size_t index() const
            {
                return _index( _pMap );
            }

            size_t _index( const BinarySearchMap* pMap ) const
            {
                ptrdiff_t diff = pKey - pMap->_pKeys;
                #ifdef _DEBUG
                if ( diff < 0 || size_t( diff ) > pMap->_capacity )
                    throw "key does not belong to map";
                #endif
                return diff;
            }

            const KeyType& key()
            {
                return *pKey;
            }

            #pragma region Operators

            operator bool() const
            {
                return _pMap != nullptr;
            }

            bool isValid() const
            {
                return _pMap != nullptr;
            }

            explicit operator const ValueType*() const
            {
                return _pMap->_pValues + index();
            }

            const ValueType& operator *() const
            {
                return _pMap->_pValues[ index() ];
            }

            const ValueType* operator ->() const
            {
                return _pMap->_pValues + index();
            }

            explicit operator ValueType*()
            {
                return _pMap->_pValues + index();
            }

            ValueType& operator *()
            {
                return _pMap->_pValues[ index() ];
            }

            ValueType* operator ->()
            {
                return _pMap->_pValues + index();
            }

            auto operator ++()
            {
                ++pKey;
                return *this;
            }

            auto operator --()
            {
                --pKey;
                return *this;
            }

            auto operator ++(int)
            {
                auto tmp = *this;
                ++*this;
                return tmp;
            }

            auto operator --(int)
            {
                auto tmp = *this;
                --*this;
                return tmp;
            }

            bool operator ==( const Iterator& itr )
            {
                return pKey == itr.pKey;
            }

            bool operator !=( const Iterator& itr )
            {
                return pKey != itr.pKey;
            }

            #pragma endregion
        };

        struct ConstIterator
        {
            const KeyType* pKey;

            const BinarySearchMap* const _pMap;

            ConstIterator( const BinarySearchMap* this_map, const KeyType* pKey )
                : pKey( pKey )
                , _pMap( this_map )
            {
            }

            ConstIterator( Iterator itr )
                : pKey( itr.pKey )
                , _pMap( itr._pMap )
            {
            }

            size_t index() const
            {
                return _index( _pMap );
            }

            size_t _index( const BinarySearchMap* pMap ) const
            {
                ptrdiff_t diff = pKey - pMap->_pKeys;
                #ifdef _DEBUG
                if ( diff < 0 || size_t( diff ) > pMap->_capacity )
                    throw "key does not belong to map";
                #endif
                return diff;
            }

            const KeyType& key()
            {
                return *pKey;
            }

            #pragma region Operators

            operator bool() const
            {
                return _pMap != nullptr;
            }

            explicit operator const ValueType*() const
            {
                return _pMap->_pValues + index();
            }

            const ValueType& operator *() const
            {
                return _pMap->_pValues[ index() ];
            }

            const ValueType* operator ->() const
            {
                return _pMap->_pValues + index();
            }

            auto operator ++()
            {
                ++pKey;
                return *this;
            }

            auto operator --()
            {
                --pKey;
                return *this;
            }

            auto operator ++(int)
            {
                auto tmp = *this;
                ++*this;
                return tmp;
            }

            auto operator --(int)
            {
                auto tmp = *this;
                --*this;
                return tmp;
            }

            bool operator ==( const ConstIterator& itr )
            {
                return pKey == itr.pKey;
            }

            bool operator !=( const ConstIterator& itr )
            {
                return pKey != itr.pKey;
            }

            #pragma endregion
        };

        #pragma region STD Iterator Functions

        using iterator = Iterator;
        using const_iterator = ConstIterator;

        iterator begin()
        {
            return Iterator( this, _pKeys );
        }

        iterator end()
        {
            return Iterator( this, _pKeys + _count );
        }

        const_iterator cbegin() const
        {
            return ConstIterator( this, _pKeys );
        }

        const_iterator cend() const
        {
            return ConstIterator( this, _pKeys + _count );
        }

        const_iterator begin() const
        {
            return cbegin();
        }

        const_iterator end() const
        {
            return cend();
        }

        #pragma endregion

        #pragma endregion

        #undef _pKeys
        #undef _pValues

    };

}

namespace std
{
    template< typename K, typename V >
    void swap(
        meck::BinarySearchMap< K, V >& a,
        meck::BinarySearchMap< K, V >& b )
    {
        std::swap( a._pData, b._pData );
        std::swap( a._capacity, b._capacity );
        std::swap( a._count, b._count );
    }
}