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

    void Place(vector<MazeBreakThrough>& break_through, int money) {
        // first strategy: try to take out break through item
        // as much as possible
        // maybe a group may come next
        // but if same number... take as much cells covered as possible
        
        auto& mngr = *tower_manager_;
        
        auto op = [](const TowerPosition& tp) {
            
        };
        mngr.ForEachOpenTowerPosition(op);
        
    }


};