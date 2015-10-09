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
    vector<Path>* result_paths_;
    Path current_path_;
    AvailDirs dir_avail;
    
public:
    PathFinder() {
        fill(dir_avail.begin(), dir_avail.end(), true);
    }
    
    vector<Path> FindPaths(const Board& board) {
        vector<Path> result_paths;
        board_ = &board;
        result_paths_ = &result_paths;
        
        for (auto p : board.spawn_locs()) {
            FindPathsRecursion(p);
        }
        return result_paths;
    }

private:    
    void FindPathsRecursion(Position& p) {
        auto& b = *board_;
        current_path_.push_back(p);
        if (b.IsBase(p)) {
            result_paths_->push_back(current_path_);
        } else {
            for (auto dir = 0; dir < kDirCount; ++dir) {
                auto vis = dir_avail[dir];
                if (!vis) continue;
                p.Shift(dir);
                auto opp = kDirOpposite[dir];
                if (board_->IsInside(p) && (board_->IsRoad(p) || board_->IsBase(p))) {
                    auto opp_vis = dir_avail[opp];
                    if (opp_vis) {
                        dir_avail[opp] = false;
                    }
                    FindPathsRecursion(p);
                    if (opp_vis) {
                        dir_avail[opp] = true;
                    }
                }
                p.Shift(opp);
            }
        }
        current_path_.pop_back();
        return;
    }

};