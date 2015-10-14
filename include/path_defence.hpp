//
//  path_defence_2.hpp
//  PathDefence
//
//  Created by Anton Logunov on 6/2/15.
//
//

#pragma once

#include "defender.hpp"
#include "maze_defender.hpp"


class PathDefense {

    Board board_;
    vector<Tower> towers_;
    vector<Count> creep_alive_ticks;
    vector<Index> creep_spawn;    
    unique_ptr<Strategy> strategy_;
    
    vector<Creep> ConvertToCreeps(const vector<int>& creeps_int) {
        Count N = creeps_int.size()/4;
        vector<Creep> creeps(N);
        auto& cc = creeps_int;
        for (int i = 0; i < N; ++i) {
            // can store just pointers inside Creep structure
            // and take everything by index
            Index id = cc[4*i];
            Position pos = {cc[4*i+3], cc[4*i+2]};
            Count ticks = creep_alive_ticks[id];
            if (ticks == 0) creep_spawn[id] = board_.spawn(pos);
            creeps[i] = Creep{id, creep_spawn[id], pos, cc[4*i+1], ticks};
        }
        return creeps;
    }

    vector<Tower> ConvertToTowers(const vector<int> tower_type) {
        vector<Tower> towers;
        for (int i = 0; i < tower_type.size(); i+=3) {
            // rng, dmg, cost
            towers.emplace_back(tower_type[i], tower_type[i+1], tower_type[i+2]);
        }
        return towers;
    }

    void AddTower(vector<int>& ts, const TowerPosition& tp) {
        ts.push_back(tp.position.col);
        ts.push_back(tp.position.row);
        ts.push_back(tp.tower);
    }
    
public:

    PathDefense() {
        strategy_ = make_unique<MazeDefender>();
        creep_alive_ticks.resize(MAX_CREEP_COUNT, 0);
        creep_spawn.resize(MAX_CREEP_COUNT);
    }


    void set_strategy(unique_ptr<Strategy> strategy) {
        strategy_ = move(strategy);
    }

    int init(vector<string> board, 
             int money, 
             int creep_health, 
             int creep_money, 
             vector<int> tower_type) {
        
        board_ = Board(board);
        towers_ = ConvertToTowers(tower_type);
        strategy_->init(board_, towers_, money, creep_health, creep_money);
        return 1;
    }     
    
    vector<int> placeTowers(const vector<int>& creeps_int, 
                            int money, 
                            vector<int>& base_health) {
        
        vector<int> res;
        vector<Creep> creeps = ConvertToCreeps(creeps_int);
        auto tps = strategy_->placeTowers(creeps, money, base_health);
        for (auto& tp : tps) {
            AddTower(res, tp);
        }
        for (auto& c : creeps) {
            ++creep_alive_ticks[c.id];
        }
        return res;
    }
    
};

