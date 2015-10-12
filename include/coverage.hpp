//
//  coverage.hpp
//  PathDefence
//
//  Created by Anton Logunov on 10/9/15.
//
//

#pragma once

#include "tower_manager.hpp"
#include "next.hpp"

class Coverage {
    
    const Board* board_;
    const Next* next_;
    const vector<Tower>* towers_;
    
    // for each tower type... // for each route (spawn)
    Grid<vector<vector<double>>> coverage_;

public:
    Coverage() {}

    void Init(const Board& board, const TowerManager& tower_manager, const Next& next) {
        board_ = &board;
        next_ = &next;
        towers_ = &tower_manager.towers();
        
        auto& ts = *towers_;
        auto& b = *board_;
        auto N = b.size();
        coverage_.resize(N, N);
        for (auto& p : Region({N, N})) {
            auto& c = coverage_[p];
            if (!b.IsEmpty(p)) {
                c.reserve(0);
                continue;
            }
            c.reserve(ts.size());
            c.resize(ts.size());
            for (auto t_ind = 0; t_ind < ts.size(); ++t_ind) {
                auto& cc = c[t_ind];
                cc.reserve(b.spawn_loc_count());
                cc.resize(b.spawn_loc_count());
            }
        }
        for (Index spawn = 0; spawn < board.spawn_loc_count(); ++spawn) {
            Recompute(spawn);
        }
    }

    const vector<double>& coverage(const Position& pos, Index tower_index) const {
        return coverage_[pos][tower_index]; 
    } 
    
    const vector<double>& coverage(const TowerPosition& tp) const {
        return coverage(tp.position, tp.tower);
    }

    void Recompute(Index spawn) {
        auto& b = *board_;
        auto& n = *next_;
        auto& ts = *towers_; 
        auto N = b.size();
        for (auto& p : Region({N, N})) {
            if (!b.IsEmpty(p)) {
                continue;
            }
            auto& c = coverage_[p];
            for (auto t_ind = 0; t_ind < ts.size(); ++t_ind) {
                auto& cc = c[t_ind];
                auto& ccc = cc[spawn];
                ccc = 0;
                for (auto& ppp : n.path(spawn)) {
                    if (!b.IsBase(ppp) && ts[t_ind].IsInRange(p, ppp)) {
                        ++ccc;
                    }
                }
            }
        }
    }
};
