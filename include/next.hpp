//
//  next.hpp
//  PathDefence
//
//  Created by Anton Logunov on 10/6/15.
//
//

#pragma once

#include "util.hpp"
#include "path_finder.hpp"

class Next {
    
    using VecPath = vector<Path>; 
    
    // first Position is spawn, last - base
    vector<Path> available_paths_;
    vector<Path> current_paths_;
    
    
    // index: spawn, value: base index for spawn
    vector<Index> spawn_to_base_;
    const Board* board_;
    
    // if only I could keep uint_t in each cell of the grid
    // I could store routes 
    // and for towers what tower this is
    
    // bit index correspond to spawn index

    
public:
    Next(const Board& board) {
        Init(board);
    }
    
    void Init(const Board& board) {
        board_ = &board;
        PathFinder pf;
        available_paths_ = pf.FindPaths(board);
        current_paths_.resize(board_->base_count());
        sort(available_paths_.begin(), available_paths_.end(), [](auto& p_1, auto& p_2) {
            return p_1.size() < p_2.size();
        });
        auto sp = board.spawn_locs();
        Index spawn = 0;
        for (auto s : sp) {
            for (auto it = available_paths_.begin(); it != available_paths_.end(); ++it) {
                auto& ap = *it;
                if (s == ap[0]) {
                    SetCurrentPath(spawn, it);
                    break;
                }
            }
            ++spawn;
        }
        if (current_paths_.size() != sp.size()) {
            throw runtime_error("lol");
        }
    }
    
    Position base_loc_for_spawn(const Position& spawn) const {
        for (auto& p : current_paths_) {
            if (p[0] == spawn) return p.back();
        }
        throw logic_error("no base location for spawn!");
    }
    
    Position base_loc_for_spawn(Index spawn) const {
        return board_->base_loc(spawn_to_base_[spawn]);
    }
    
    Index base_for_spawn(Index spawn) const {
        return spawn_to_base_[spawn];
    }
    
    // returns true if some route were changed
    bool check(const Creep& creep) {
        auto& cp = current_paths_[creep.spawn];
        if (cp[creep.ticks] != creep.pos) {
            Replace(creep);
            return true;
        }
        return false;
    }
    
    Position next(const Creep& creep) const {
        auto& cp = current_paths_[creep.spawn];
        return cp[creep.ticks+1];
    }
    
private:
    void Replace(const Creep& creep) {
        auto spawn = creep.spawn;
        auto& sp = board_->spawn_locs();
        auto it = std::find_if(available_paths_.begin(), available_paths_.end(), [&](auto& ap) {
            return ap[0] == sp[creep.spawn] && ap[creep.ticks] == creep.pos;
        });
        SetCurrentPath(spawn, it);
    }
    
    void SetCurrentPath(Index spawn, VecPath::iterator avail_path) {
        auto& cp = current_paths_[spawn];
        cp = move(*avail_path);
        available_paths_.erase(avail_path);
        spawn_to_base_[spawn] = board_->base(cp.back());
    }
    
};




