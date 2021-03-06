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
    vector<Position> spawn_locations_;
    vector<Position> base_locations_;
    Grid<char> picture_;
    
      
public:    
    
    Board() {}
    // this one straight from input
    Board(const vector<string>& b) {
        N = static_cast<Count>(b.size());
        picture_ = ToGrid(b);
        ComputeSpawnLocations();
        ComputeBaseLocations();
    }
    
    template<class Process>
    void ForEachNearby(const Position& t, Process& proc) const {
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
    
    template<class Process>
    void ForEachNearby_PassDir(const Position& t, Process& proc) const {
        if (t.row > 0) {
            proc(t+Indent{-1, 0}, kDirDown);
        }
        if (t.row < N-1) {
            proc(t+Indent{ 1, 0}, kDirUp);
        }
        if (t.col > 0) {
            proc(t+Indent{ 0,-1}, kDirRight);
        }
        if (t.col < N-1) {
            proc(t+Indent{ 0, 1}, kDirLeft);
        }
    }
    
    bool IsBaseNearby(const Position& t) const {
        bool b = false;
        auto func = [&](const Position& p) {
            if (IsBase(p)) {
                b = true;
            }
        };
        ForEachNearby(t, func);
        return b; 
    }
    
    
    bool IsBase(const Position& t) const {
        char ch = picture_[t];
        return ch >= '0' && ch <= '9';
    }
    
    bool IsRoad(const Position& t) const {
        return picture_[t] == CELL_ROAD;
    }
    
    bool IsEmpty(const Position& t) const {
        return picture_[t] == CELL_EMPTY;
    }
    
    bool IsInside(const Position& t) const {
        return picture_.isInside(t);
    }
    
    Count spawn_loc_count() const {
        return spawn_locations_.size();
    }
    
    Index base(const Position& p) const {
        return picture_[p] - '0';
    }
    
    Count base_count() const {
        return base_locations_.size();
    }
    
    void PlaceTower(const Position& p) {
        picture_[p] = CELL_TOWER;
    }
 
    Index spawn(const Position& p) const {
        for (Index i = 0; i < spawn_locations_.size(); ++i) {
            if (spawn_locations_[i] == p) {
                return i;
            }
        }
        throw logic_error("spawn unknown!");
    }
    
    const vector<Position>& spawn_locs() const {
        return spawn_locations_;
    }
    
    const vector<Position>& base_locs() const {
        return base_locations_;
    } 
    
    Position spawn_loc(Index spawn) const {
        return spawn_locations_[spawn];
    }
    
    Position base_loc(Index base) const {
        return base_locations_[base];
    }
    
    Count size() const {
        return N;
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
    }
    
    void ComputeBaseLocations() {
        Count count = CountBases();
        base_locations_.resize(count);
        for (auto p : Region(picture_.size())) {
            if (IsBase(p)) {
                base_locations_[base(p)] = p;
            }
        }
    }
    
    Count CountBases() {
        Count count = 0;
        for (auto p : Region(picture_.size())) {
            if (IsBase(p)) {
                ++count;
            }
        }
        return count;
    }
};




#endif
