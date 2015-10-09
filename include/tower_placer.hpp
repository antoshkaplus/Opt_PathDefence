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

// this class is used to determine where to place 
// new tower
class TowerPlacer {
public:
    TowerPlacer(TowerManager& manager, const Next& next) {
        Init(manager, next);
    }
    
    void Init(TowerManager& manager, const Next& next) {
        tower_manager_ = &manager; 
        current_coverage.resize(manager.board().spawn_loc_count());
        fill(current_coverage.begin(), current_coverage.end(), 0);
    }
    
    
    // coverage starts with 0-s and it's equal to number of routes
    // break through : spawn
    void Place(const vector<Creep>& creeps, 
               vector<Creep>& after_simulation_creeps,
               Count& money) {
        // coverage should be computed in another class
        // should handle query (tower_position, tower)
        // return coverage for each route
    }
    
    
    
    TowerManager* tower_manager_;
    const Next* next_; 
    vector<double> current_coverage; 
    
};

