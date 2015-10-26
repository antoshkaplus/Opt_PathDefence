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
#include "maze_routes.hpp"


class MazeDefender : public Strategy {
    
    Maze maze_;
    TowerManager tower_manager_;
    MazeSimulator simulator_;
    MazeTowerPlacer tower_placer_;
    MazeRoutes routes_;
    
    const Board* board_;
    
    ofstream out_crossroads_;
    ofstream out_creep_count_; 
    ofstream out_deduction_;
    
    Index iteration_;
    unordered_map<Index, Creep> creeps_prev_;
    unordered_map<Index, Creep> creeps_;
    
    public:
    int init(Board& board, 
             const vector<Tower>& towers,
             int money, 
             int creep_health, 
             int creep_money) override {
        
        board_ = &board;
        iteration_ = 0;
        out_crossroads_.open(output_path + "crossroards.txt");
        out_creep_count_.open(output_path + "creep_count.txt");
        out_deduction_.open(output_path + "deduction.txt");
        tower_manager_.Init(board, towers);
        routes_.Init(board);
        tower_placer_.Init(board, tower_manager_, routes_);
        simulator_.Init(maze_, tower_manager_);
        maze_.Deduct(board);
        maze_.Print(out_deduction_);
        return 1;
    }
    
    // returns indices of towers that were placed
    vector<TowerPosition> placeTowers(const vector<Creep>& creeps, 
                                      int money, 
                                      vector<Count>& base_health) override {
        out_creep_count_ << iteration_ << " : " << creeps.size() << endl;
        
        auto N = board_->size(); 
        vector<Position> creep_prev_vec;
        creeps_.clear();
        for (auto& c : creeps) {
            creeps_[c.id] = c;
            auto it = creeps_prev_.find(c.id);
            if (it == creeps_prev_.end()) {
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
            creeps_prev_.erase(it);
        }
        // now we have in creep_prev_ those who not with us anymore
        auto& b = *board_;
        for (auto& p : creeps_prev_) {
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
        // should think about it more
        while (++iteration < 10) {
            auto& bt = simulator_.break_through(); 
            simulator_.Simulate(creeps, creep_prev_vec);
            auto dmg_before = simulator_.total_dmg();
            if (iteration == 1 && !bt.empty()) {
                CheckInRoutes(bt);
            } 
            if (bt.empty()) break;
            auto bbt = bt;
            for (auto& b : bbt) {
                b.id = creeps_[b.id].spawn;
            }
            tower_placer_.Place(bt, money);
            
//            
//            tower_placer_.PlaceMoreCoverage(bbt, money); 
//            simulator_.Simulate(creeps, creep_prev_vec);
//            if (simulator_.total_dmg() == dmg_before) {
//                tower_placer_.Revert(money);
//                tower_placer_.Place(bt, money);
//            }
        }
        if (!simulator_.break_through().empty()) {
            cerr << "Warning! Not all creeps killed!";
        }
        
        ++iteration_;
        if (iteration_ == TICK_COUNT) {
            auto& b = *board_;
            Count N = b.size();
            for (auto& p : Region({N, N})) {
                if (b.IsRoad(p)) {
                    auto& ds = maze_.Next(p);
                    Count cc = count(ds.begin(), ds.end(), true);
                    if (cc == 1) continue;
                    out_crossroads_ << "row: " << p.row << ", col: " << p.col << ", how many: " << cc << endl;  
                }
            }
            out_crossroads_.close();
        }
        
        creeps_prev_ = creeps_;
        return {placed_towers.begin()+tower_count, placed_towers.end()};
    }
    
private:

    void CheckInRoutes(const vector<MazeBreakThrough>& break_through_) {
        for (auto& bt : break_through_) {
            routes_.CheckIn(creeps_[bt.id].spawn, bt.path);
        }
    }
        
};
