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
    
    // first Position is spawn, last - base
    vector<Path> available_paths_;
    vector<Path> current_paths_;
    const Board* board_;
    
public:
    Next(const Board& board) : board_(&board) {
        PathFinder pf;
        available_paths_ = pf.FindPaths(board);
        sort(available_paths_.begin(), available_paths_.end(), [](auto& p_1, auto& p_2) {
            return p_1.size() < p_2.size();
        });
        auto sp = board.spawn_locs();
        for (auto s : sp) {
            for (auto it = available_paths_.begin(); it != available_paths_.end(); ++it) {
                auto& ap = *it;
                if (s == ap[0]) {
                    current_paths_.push_back(ap);
                    available_paths_.erase(it);
                    break;
                }
            }
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
        
    }
    
    Index base_for_spawn(Index spawn) const {
        
    }
    
    void check(const Creep& creep) {
        auto& cp = current_paths_[creep.spawn];
        if (cp[creep.ticks] != creep.pos) {
            Replace(creep);
        }
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
        current_paths_[spawn] = *it;
        available_paths_.erase(it);
    }

};




