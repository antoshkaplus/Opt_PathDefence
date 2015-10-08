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

class Defender {
    
    Board* board_;
    
    // in this case destruction order is important
    Next next_;
    TowerManager tower_manager_;
    Simulator simulator_;
    // this should be something like strategy
    // better use polymorphysm
    TowerPlacer tower_placer_; 
    
    Index iteration = 0;
    array<Index, MAX_CREEP_COUNT> creep_spawn;
    Count last_tower_count;
    vector<double> coverage;
        
public:
    
    int init(Board& board, 
             const vector<Tower> towers,
             int money, 
             int creep_health, 
             int creep_money) {
        
        board_ = &board;
        next_.Init(board);
        tower_manager_.Init(board, towers);
        simulator_.Init(board, tower_manager_, next_);
        tower_placer_.Init(tower_manager_, next_);
        
        coverage.resize(this->board_->spawn_loc_count(), 0);
        return 1;
    }     
    
    // returns indices of towers that were placed
    vector<TowerPosition> placeTowers(const vector<Creep>& creeps, 
                              int money, 
                              vector<Count>& base_health) {
        // need to make it smarter
        int m = *min_element(base_health.begin(), base_health.end());
        if (m < 400) {
            return vector<TowerPosition>();
        }
        vector<Index> spawns(creeps.size());
        for (Index i = 0; i < creeps.size(); ++i) {
            auto& c = creeps[i];
            if (creep_spawn[c.id] == -1) {
                creep_spawn[c.id] = board_->spawn(c.pos);
            }
            spawns[i] = creep_spawn[c.id];
        }
        vector<int> res;
        vector<Count> route_hp_break;
        vector<BreakThrough> break_through;
        if (iteration == 472) {
            int r = 0; 
            ++r;
        }
        while (true) {
            // should check how old code worked probably
            tie(route_hp_break, break_through) = simulator_.Simulate(creeps);
            if (!break_through.empty()) {
                tower_placer_.Place(route_hp_break, break_through, money);
            }
            auto& tt = board->placed_towers();
            if (last_tower_count == tt.size()) break;
            for (auto i = last_tower_count; i < tt.size(); ++i) {
                AddTower(res, tt[i]);
            }
            last_tower_count = tt.size();
        }
        ++iteration;
        return res;
    }
};
