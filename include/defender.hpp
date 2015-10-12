//
//  defender.hpp
//  PathDefence
//
//  Created by Anton Logunov on 10/8/15.
//
//

#pragma once

#include "simulator.hpp"
#include "tower_placer.hpp"
#include "strategy.hpp"

class Defender : public Strategy {
    
    Board* board_;
    
    // in this case destruction order is important
    Next next_;
    TowerManager tower_manager_;
    Simulator simulator_;
    Coverage coverage_;
    // this should be something like strategy
    // better use polymorphysm
    TowerPlacer tower_placer_; 
        
public:
    
    int init(Board& board, 
             const vector<Tower>& towers,
             int money, 
             int creep_health, 
             int creep_money) override {
        
        board_ = &board;
        next_.Init(board);
        tower_manager_.Init(board, towers);
        simulator_.Init(board, tower_manager_, next_);
        coverage_.Init(board, tower_manager_, next_);
        tower_placer_.Init(tower_manager_, next_, coverage_);
        
        return 1;
    }     
    
    // returns indices of towers that were placed
    vector<TowerPosition> placeTowers(const vector<Creep>& creeps, 
                              int money, 
                              vector<Count>& base_health) override {
        auto sim_creeps = creeps;
        auto& placed_towers = tower_manager_.placed_towers();
        auto tower_count = placed_towers.size();
        while (true) {
            // should check how old code worked probably
            simulator_.Simulate(sim_creeps);
            tower_placer_.Place(creeps, sim_creeps, money);
            break;
        }
        return {placed_towers.begin()+tower_count, placed_towers.end()};
    }
};
