//
//  maze.hpp
//  PathDefence
//
//  Created by Anton Logunov on 10/11/15.
//
//

#pragma once

#include "util.hpp"


class Maze {
    
    using Direction = array<bool, kDirCount>;

    unordered_map<Position, Direction> avail_next_;

public:

    void CheckIn(const Position& pos, const Position& prev) {
        auto d = FromDirVector(pos - prev);
        auto it = avail_next_.find(prev);
        if (it == avail_next_.end()) {
            Direction ds = {{false, false, false, false}};
            it = avail_next_.emplace(prev, ds).first;
        }         
        it->second[d] = true;
    }

    const Direction& Next(const Position& pos) const {
        auto it = avail_next_.find(pos);
        if (it == avail_next_.end()) {
            static Direction empty = {{false, false, false, false}};
            return empty;
        }
        return it->second;
    }
    
    const Direction Next(const Position& pos, const Position& prev) const {
        Direction res = Next(pos);
        auto back_dir = FromDirVector(prev-pos);
        res[back_dir] = false;
        return res;
    }
    
};