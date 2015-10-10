//
//  place_tower.hpp
//  PathDefence
//
//  Created by Anton Logunov on 6/5/15.
//
//

#pragma once


#include "util.hpp"
#include "tower_manager.hpp"
#include "coverage.hpp"

// this class is used to determine where to place 
// new tower
class TowerPlacer {
public:
    TowerPlacer() {}

    TowerPlacer(TowerManager& manager, const Next& next, const Coverage& coverage) {
        Init(manager, next, coverage);
    }
    
    void Init(TowerManager& manager, const Next& next, const Coverage& coverage) {
        tower_manager_ = &manager; 
        next_ = &next;
        coverage_ = &coverage;
    }
    
    
    // coverage starts with 0-s and it's equal to number of routes
    // break through : spawn
    void Place(const vector<Creep>& creeps, 
               vector<Creep>& after_simulation_creeps,
               Count& money) {
        
        auto& mngr = *tower_manager_;
        auto& b = mngr.board();
        auto& ts = mngr.towers();
        auto& cov = *coverage_;
        
        vector<int> spawn_hp_break(b.spawn_loc_count(), 0);
        for (auto& c : after_simulation_creeps) {
            if (b.IsBase(c.pos)) {
                spawn_hp_break[c.spawn] += c.hp;
            }
        }
        TowerPosition best_tp;
        int best_success = 0;
        int best_total_c = 0;
        bool best_initialized = false;
        for (auto p : mngr.open_tower_positions()) {
            for (auto t_ind = 0; t_ind < ts.size(); ++t_ind) {
                auto& t = ts[t_ind];
                if (t.cost > money) continue;
                auto c = cov.coverage(p, t_ind);
                auto total_c = 0;
                for (auto& v : c) {
                    v *= t.dmg;
                    total_c += v;
                }
                int success = 0;
                for (auto spawn = 0; spawn < b.spawn_loc_count(); ++spawn) {
                    success += min<int>(c[spawn], spawn_hp_break[spawn]); 
                }
                //if (success > best_success) {
                if (!best_initialized || success* (1.L*ts[t_ind].dmg/ts[t_ind].cost) > best_success* (1.L*ts[best_tp.tower].dmg/ts[best_tp.tower].cost) 
                        || (success* (1.L*ts[t_ind].dmg/ts[t_ind].cost) == best_success* (1.L*ts[best_tp.tower].dmg/ts[best_tp.tower].cost) && total_c > best_total_c)) {
                    best_initialized = true;
                    best_success = success;
                    best_total_c = total_c;
                    best_tp = TowerPosition{t_ind, p};
                }
            }
        }
        if (best_success > 0) {
            mngr.PlaceTower(best_tp);
        }
    }
    
    TowerManager* tower_manager_;
    const Next* next_;
    const Coverage* coverage_;
};

