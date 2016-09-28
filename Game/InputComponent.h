#pragma once


#include "BinarySearchMap.hpp"
template< typename KeyType, typename ValueType >
using Map = meck::BinarySearchMap< KeyType, ValueType >;

#include <glm/glm.hpp>

#include <vector>

struct Node
{
    int x, y;

    operator unsigned long() const
    {
        return *reinterpret_cast< const unsigned long* >( this );
    }
};

double estimate( Node a, Node b )
{
    return glm::distance( glm::vec2{ a.x, a.y }, glm::vec2{ b.x, b.y } );
}

std::vector< Node > neighbor_nodes( Node current, const char* grid, int width, int height )
{
    int x = current.x;
    int y = current.y;

    std::vector< Node > neighbors;

    const auto ind = [=]( int x, int y ) -> char {
        if ( x < 0 || y < 0 || x > width || y > height )
            return 'X';
        return grid[ (y * width) + x ];
    };

    const auto check = [&]( int x, int y ) {
        if ( ind( x, y ) != 'X' )
            neighbors.push_back( { x, y } );
    };

    check( x, y+1 );
    check( x-1, y );
    check( x+1, y );
    check( x, y-1 );

    return neighbors;
}

// The rest of this file is pure A* algorithm.

Node find_cheapest( std::vector< Node >& open_set, Map< Node, double >& f_score )
{
    Node cheapest = open_set[ 0 ];

    for ( Node node : open_set )
    {
        auto nodeScore = f_score[ node ];
        auto cheapScore = f_score[ cheapest ];
        if ( nodeScore != 0 && cheapScore != 0 && nodeScore < cheapScore )
            cheapest = node;
    }
    return cheapest;
}

std::vector< Node > reconstruct( Map< Node, Node >& came_from, Node current )
{
    std::vector< Node > total_path = { current };

    while ( came_from.search( current ) )
    {
        current = came_from[ current ];
        total_path.push_back( current );
    }
    // Remove the node the enemy is already standing on.
    //total_path.RemoveAt( total_path.Count - 1 );

    return total_path;
}

/// <summary>
/// Uses A* to generate a path to the destination.
/// </summary>
/// <param name="dest">Location to find a path to.</param>
/// <returns>A list of sequential points which mark a path from the 
/// destination to the actor. Returns an empty list if a path cannot 
/// be found.</returns>
std::vector< Node > FindPathTo( Node start, Node goal, const char* grid, int width, int height )
{
    // Fair initial capacity reduces reallocations for dictionaries.
    //int initial_capacity = (int) Math.Sqrt( Dungeon.WIDTH * Dungeon.HEIGHT );

    // A* algorithm begins here. The algorithm is well understood and 
    // documented online and in textbooks; hence the lack of comments 
    // in the code. If you'd like to know more, feel free to visit: 
    // http://en.wikipedia.org/wiki/A*_search_algorithm, which 
    // contains the pseduo-code I used to implement the algorithm.

    std::vector< Node > closed_set;
    std::vector< Node > open_set = { start };
    Map<Node, Node> came_from;// (initial_capacity * 4);

    Map<Node, double> g_score;// (initial_capacity);
    g_score[ start ] = 0;

    Map<Node, double> f_score;// (initial_capacity);
    f_score[ start ] = estimate( start, goal );

    while ( open_set.size() > 0 )
    {
        Node current = find_cheapest( open_set, f_score );

        if ( current.x == goal.x && current.y == goal.y )
            return reconstruct( came_from, goal ); // <-- exit point

        open_set.erase( std::find( open_set.begin(), open_set.end(), current ) );
        closed_set.push_back( current );
        for ( Node neighbor : neighbor_nodes( current, grid, width, height ) )
        {
            if ( std::find( closed_set.begin(), closed_set.end(), neighbor ) != closed_set.end() )
                continue;

            double tentative_g_score = g_score[ current ] + 1;

            if ( std::find( open_set.begin(), open_set.end(), neighbor ) == open_set.end()
                 || tentative_g_score < g_score[ neighbor ] )
            {
                came_from[ neighbor ] = current;
                g_score[ neighbor ] = tentative_g_score;
                f_score[ neighbor ] = tentative_g_score + estimate( neighbor, goal );
                if ( std::find( open_set.begin(), open_set.end(), neighbor ) == open_set.end() )
                    open_set.push_back( neighbor );
            }
        }
    }

    return {};
}

