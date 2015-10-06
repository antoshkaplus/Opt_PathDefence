//
//  util.hpp
//  PathDefence
//
//  Created by Anton Logunov on 5/30/15.
//
//

#ifndef PathDefence_util_hpp
#define PathDefence_util_hpp


#include <iostream>
#include <fstream>
#include <array>
#include <queue>


#include "ant/core/core.hpp"
#include "ant/geometry/d2.hpp"
#include "ant/grid.hpp"



using namespace std;
using namespace ant;
using namespace ant::grid;


using Path = vector<Position>;


struct Creep {
    Index spawn;
    Position pos;
    Index id;
    Count hp;
    // how long stays alive
    Count ticks;
    
    Creep() {}
    Creep(Index id, Position pos, Count hp, Count ticks = 0)
    : pos(pos), id(id), hp(hp), ticks(ticks) {}
};

struct Tower {
    // range
    Count rng; 
    Count dmg;
    Count cost;
    
    Tower() {}
    
    Tower(Count rng, Count dmg, Count cost) 
    : rng(rng), dmg(dmg), cost(cost) {}
};

struct TowerPosition {
    Index tower;
    Position position;
    
    TowerPosition() {}
    
    TowerPosition(Index tower, Position position) 
    : tower(tower), position(position) {}
};

struct BreakThrough {
    Position cur_loc;
    Index spawn_loc;
    
    BreakThrough(const Position& loc, Index spawn) 
        : cur_loc(loc), spawn_loc(spawn) {}
};

constexpr Count MAX_CREEP_COUNT = 2000;
constexpr Count TICK_COUNT = 2000;
constexpr Count HP_INCREASE_PERIOD = 500;
constexpr Count MAX_BASE_COUNT = 8;

// after we place a tower we will add "*" in that place
// '.' - road
// '#' - tower
// road is not route
constexpr char CELL_ROAD = '.';
constexpr char CELL_TOWER = '*';
constexpr char CELL_EMPTY = '#'; 
constexpr char CELL_BASE = 'b';

//using Board = Grid<char>;

class Board_2;

vector<double> ComputeCoverage(const Board_2& b, const TowerPosition& tp);


#endif
