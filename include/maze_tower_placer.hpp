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

    // bunch of score parameters
    struct Params {
        double dmg_pow;
        double cost_pow;
    };

    struct Placement : TowerPosition {
        Count score;

    private:
        const MazeTowerPlacer* placer_;
    
    public:
        Placement() {}
    
        Placement(TowerPosition placement, Count score, const MazeTowerPlacer& placer) 
        : TowerPosition(placement), score(score), placer_(&placer) {}
        
        bool IsBetterThan(const Placement& item) {
            auto& fs = placer_->tower_factors_; 
            auto& mngr = *placer_->tower_manager_;
            return score * fs[tower] > item.score * fs[item.tower]
                || (tower == item.tower && score == item.score && mngr.tower_scope(*this).positions.size() > mngr.tower_scope(item).positions.size());  
        }
    };


    TowerManager* tower_manager_; 
    Params params_;
    vector<double> tower_factors_;


public:
    

    void Init(const Board& board, TowerManager& tower_manager) {
        tower_manager_ = &tower_manager;
        params_ = {1, 1};
        tower_factors_.resize(tower_manager.towers().size());
        ComputeTowerFactors();
    }

    void Place(const vector<MazeBreakThrough>& break_through, int& money) {
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
        
        Placement best;
        best.score = 0;
        
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
            Placement pl{tp, score, *this};
            if (!is_initialized || pl.IsBetterThan(best)) {
                is_initialized = true;
                best = pl;
            }
        };
        mngr.ForEachOpenTowerPosition(op);
        if (best.score > 0) {
            mngr.PlaceTower(best);
            money -= ts[best.tower].cost;
        }
    }

    void set_params(Params& params) {
        params_ = params;
        ComputeTowerFactors();
    } 
    
private:

    void ComputeTowerFactors() {
        auto& ts = tower_manager_->towers();
        for (auto i = 0; i < ts.size(); ++i) {
            tower_factors_[i] = pow(ts[i].dmg, params_.dmg_pow) / pow(ts[i].cost, params_.cost_pow);
        }
    }
};