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
#include <unordered_map>

#include "ant/core/core.hpp"
#include "ant/geometry/d2.hpp"
#include "ant/grid.hpp"



using namespace std;
using namespace ant;
using namespace ant::grid;


using Path = vector<Position>;

class TowerManager;


struct Creep {
    Position pos;
    Index id;
    Index spawn;
    Count hp;
    // how long stays alive
    Count ticks;
    
    Creep() {}
    Creep(Index id, Index spawn, Position pos, Count hp, Count ticks = 0)
    : pos(pos), id(id), spawn(spawn), hp(hp), ticks(ticks) {}
};

struct Tower {
    // range
    Count rng; 
    Count dmg;
    Count cost;
    
    Tower() {}
    
    Tower(Count rng, Count dmg, Count cost) 
    : rng(rng), dmg(dmg), cost(cost) {}
    
    bool IsInRange(const Position& tower_pos, const Position& pos) const {
        auto row_diff = tower_pos.row - pos.row;
        auto col_diff = tower_pos.col - pos.col;
        return row_diff*row_diff + col_diff*col_diff <= rng*rng;
    }
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

struct MazeBreakThrough {
    // first: where creep started, last: creep hits base
    Path path;
    Count hp;
    
    MazeBreakThrough() {}
    
    MazeBreakThrough(Count hp, const Path& path) 
    : path(path), hp(hp) {}
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


extern const string output_path;

vector<double> ComputeCoverage(const TowerManager& b, const TowerPosition& tp);


#endif
