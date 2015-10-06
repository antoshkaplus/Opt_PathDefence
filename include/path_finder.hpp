//
//  path_finder.hpp
//  PathDefence
//
//  Created by Anton Logunov on 10/6/15.
//
//

#pragma once

#include "board.hpp"

class PathFinder {

    using AvailDirs = array<bool, kDirCount>;

    const Board* board_;  
    const vector<Path>* result_paths_;
    Path current_path_;
    AvailDirs dir_avail{true};
    
public:
    vector<Path> FindPaths(const Board& board) {
        vector<Path> result_paths;
        board_ = &board;
        result_path_ = &result_paths;
        
        for (auto p : board.spawn_locs()) {
            FindPathsRecursion(p);
        }
    }

private:    
    void FindPathsRecursion(Position& p) {
        current_path_->push_back(p);
        if (IsBase(p)) {
            result_paths_->push_back(current_path);
        } else {
            for (auto dir = 0; dir < kDirCount; ++dir) {
                auto vis = dir_avail[dir];
                if (vis) continue;
                p.Shift(dir);
                if (board_->IsInside(p) && (board_->IsRoad(p) || board_->IsBase(p))) {
                    auto opp = kDirOpposite[dir];
                    auto opp_vis = dir_avail[opp];
                    if (!opp_vis) {
                        dir_avail[opp] = true;
                    }
                    FindPathsRecursion(p);
                    if (!opp_vis) {
                        dir_avail[opp] = false;
                    }
                }
                p.Shift(opp);
            }
        }
        current_path->pop_back();
        return;
    }

};