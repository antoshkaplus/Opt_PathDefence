//
//  maze_defender.hpp
//  PathDefence
//
//  Created by Anton Logunov on 10/12/15.
//
//


#pragma once

#include "strategy.hpp" 
#include "maze.hpp"
#include "util.hpp"
#include "maze_simulator.hpp"
#include "maze_tower_placer.hpp"


class MazeDefender : public Strategy {
    
    Maze maze_;
    TowerManager tower_manager_;
    MazeSimulator simulator_;
    MazeTowerPlacer tower_placer_;
    
    const Board* board_;
    
    ofstream crossroads_;
    Index iteration;
    map<Index, Creep> creep_prev_;
    
    
    public:
    int init(Board& board, 
             const vector<Tower>& towers,
             int money, 
             int creep_health, 
             int creep_money) override {
        
        board_ = &board;
        iteration = 0;
        crossroads_.open(output_path + "crossroards.txt");
        tower_manager_.Init(board, towers);
        tower_placer_.Init(board, tower_manager_);
        simulator_.Init(maze_, tower_manager_);
        maze_.Deduct(board);
        return 1;
    }
    
    // returns indices of towers that were placed
    vector<TowerPosition> placeTowers(const vector<Creep>& creeps, 
                                      int money, 
                                      vector<Count>& base_health) override {
        auto N = board_->size(); 
        vector<Position> creep_prev_vec;
        for (auto& c : creeps) {
            auto it = creep_prev_.find(c.id);
            if (it == creep_prev_.end()) {
                // just spawned
                auto p = c.pos;
                if (p.row == 0) {
                    p.Shift(kDirUp);
                } else if (p.col == 0) {
                    p.Shift(kDirLeft);
                } else if (p.row == N-1) {
                    p.Shift(kDirRight);
                } else {
                    p.Shift(kDirDown);
                }
                creep_prev_vec.push_back(p);
                continue;
            }
            auto p = it->second.pos;
            maze_.CheckIn(c.pos, p);
            creep_prev_vec.push_back(p);
            creep_prev_.erase(it);
        }
        // now we have in creep_prev_ those who not with us anymore
        auto& b = *board_;
        for (auto& p : creep_prev_) {
            auto op = [&](const Position& n) {
                if (b.IsBase(n)) {
                    maze_.CheckIn(n, p.second.pos);
                }
            };
            b.ForEachNearby(p.second.pos, op);
        }
        
        auto& placed_towers = tower_manager_.placed_towers();
        Count tower_count = placed_towers.size();
        Index iteration = 0;
        while (++iteration < 10) {
            simulator_.Simulate(creeps, creep_prev_vec);
            if (simulator_.break_through().empty()) break;
            tower_placer_.Place(simulator_.break_through(), money);
        }
        ++iteration;
        if (iteration == TICK_COUNT) {
            auto& b = *board_;
            Count N = b.size();
            for (auto& p : Region({N, N})) {
                if (b.IsRoad(p)) {
                    auto& ds = maze_.Next(p);
                    Count cc = count(ds.begin(), ds.end(), true);
                    if (cc == 1) continue;
                    crossroads_ << "row: " << p.row << ", col: " << p.col << ", how many: " << cc << endl;  
                    }
                    }
                    crossroads_.close();
                    }
                    
                    creep_prev_.clear();
                    for (auto& c : creeps) {
                        creep_prev_[c.id] = c;
                    }
                    return {placed_towers.begin()+tower_count, placed_towers.end()};
                    }
                    
                    };
