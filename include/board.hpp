//
//  board.hpp
//  PathDefence
//
//  Created by Anton Logunov on 6/1/15.
//
//

#ifndef PathDefence_board_hpp
#define PathDefence_board_hpp

#include "ant/graph/graph.hpp"

#include "util.hpp"


using namespace ant::graph;

// this class is performing role of world
class Board {
    
    Count N;
    // also known as routes
    vector<Position> spawn_locations_;
    // for some time base location may not be defined or defined wrong
    vector<Position> base_locations_;
    Grid<map<Index, Position>*> paths_; 
    vector<string> picture_;
    
    
public:    
    
    // this one straight from input
    Board(vector<string>& b) {
        N = b.size();
        picture_ = b;
        paths_.resize(N, N);
        Region rr{{0, 0}, {N, N}};
        for (auto p : rr) {
            assert(rr.hasInside(p));
            if (IsRoad(p)) {
                paths_[p] = new map<Index, Position>();
            }
        }
        ComputeSpawnLocations();
    }
    
    template<class Process>
    void ForEachNearby(const Position& t, Process& proc) {
        if (t.row > 0) {
            proc(t+Indent{-1, 0});
        }
        if (t.row < N-1) {
            proc(t+Indent{ 1, 0});
        }
        if (t.col > 0) {
            proc(t+Indent{ 0,-1});
        }
        if (t.col < N-1) {
            proc(t+Indent{ 0, 1});
        }
    }
    
    bool IsBaseNearby(const Position& t) {
        bool b = false;
        auto func = [&](const Position& p) {
            if (IsBase(p)) {
                b = true;
            }
        };
        ForEachNearby(t, func);
        return b; 
    }
    
    
    bool IsBase(const Position& t) {
        char ch = picture_[t.row][t.col];
        return ch >= '0' && ch <= '9';
    }
    
    bool IsRoad(const Position& t) {
        return picture_[t.row][t.col] == CELL_ROAD;
    }
    
    bool IsEmpty(const Position& t) {
        return picture_[t.row][t.col] == CELL_EMPTY;
    }
    
    Count spawn_loc_count() const {
        return spawn_locations_.size();
    }
    
    void PlaceTower(const Position& p) {
        picture_[p.row][p.col] = CELL_TOWER;
    }
 
    Index spawn(const Position& p) {
        for (Index i = 0; i < spawn_locations_.size(); ++i) {
            if (spawn_locations_[i] == p) {
                return i;
            }
        }
        throw logic_error("spawn unknown!");
    }
    
    Position spawn_loc(Index spawn) const {
        return spawn_locations_[spawn];
    }
    
    pair<Position, bool> base_loc_for_spawn(Index spawn) const {
        return {base_locations_[spawn], base_locations_[spawn].row != -1};
    }
    
    void set_next(Index spawn, const Position& cur, const Position& next) {
        auto& p = *paths_[cur];
        p[spawn] = next;
    }
    
    void set_base_for_spawn(Index spawn, const Position& base) {
        base_locations_[spawn] = base;
    }
    
    pair<Position, bool> next(Index spawn, const Position& cur) {
        auto end = paths_[cur]->end();
        auto it = paths_[cur]->find(spawn);
        pair<Position, bool> res;
        if (it == end) {
            res.second = false;
        } else {
            res.second = true;
            res.first = it->second;
        }
        return res;
    }
    
    const map<Index, Position>& nexts(const Position& cur) const {
        return *paths_(cur);
    }
    
private:
 
    void ComputeSpawnLocations() {
        for (Index i = 0; i < N; ++i) {
            using P = Position;
            for (const P& p : {P{0, i}, P{N-1, i}, P{i, 0}, P{i, N-1}}) {
                if (IsRoad(p)) {
                    spawn_locations_.push_back(p);
                }
            }
        }
        base_locations_.resize(spawn_locations_.size(), {-1, -1});
    }
    
    friend class Board_3;
    friend class Board_2;
};




#endif
