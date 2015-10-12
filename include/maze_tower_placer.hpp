//
//  maze_tower_placer.hpp
//  PathDefence
//
//  Created by Anton Logunov on 10/11/15.
//
//

#pragma once

#include "tower_manager.hpp"


class MazeTowerPlacer {

    TowerManager* tower_manager_; 

public:

    void Init(TowerManager& tower_manager) {
        tower_manager_ = &tower_manager;
    }

    void Place(const vector<MazeBreakThrough>& break_through, int money) {
        // first strategy: try to take out break through item
        // as much as possible
        // maybe a group may come next
        // but if same number... take as much cells covered as possible
        
        if (break_through.empty()) return;
        
        CountMap<Position, UnorderedMap> bt_pos;
        for (auto& bt : break_through) {
            for (auto& p : bt.path) {
                if (p == bt.path.back()) continue;
                bt_pos.increase(p);
            }
        }
        
        auto& mngr = *tower_manager_;
        auto& ts = mngr.towers();
        Count best_score = 0;
        TowerPosition best_tp;
        bool is_initialized = false;
        auto op = [&](const TowerPosition& tp) {
            if (ts[tp.tower].cost > money) {
                return;
            }
            Count score = 0;
            for (auto& p : mngr.tower_scope(tp).positions) {
                auto it = bt_pos.find(p);
                if (it == bt_pos.end()) continue;
                score += it->second;
            }
            // maybe change powers a little bit
            if (!is_initialized || score * (1.L*ts[tp.tower].dmg/ts[tp.tower].cost) > 
                best_score * (1.L*ts[best_tp.tower].dmg/ts[best_tp.tower].cost) ||
                (tp.tower == best_tp.tower && score == best_score && mngr.tower_scope(tp).positions.size() > mngr.tower_scope(tp).positions.size())) {
                
                is_initialized = true;
                best_tp = tp;
                best_score = score;
            }
        };
        mngr.ForEachOpenTowerPosition(op);
        if (best_score > 0) {
            mngr.PlaceTower(best_tp);
        }
    }


};