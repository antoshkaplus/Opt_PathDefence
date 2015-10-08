//
//  path_defence_2.hpp
//  PathDefence
//
//  Created by Anton Logunov on 6/2/15.
//
//

#pragma once

#include "simulator.hpp"
#include "tower_placer.hpp"


class PathDefense {

    

    Simulator* simulator;
    Board* board;
    TowerPlacer* place; 
    
    Index iteration = 0;
    array<Index, MAX_CREEP_COUNT> creep_spawn;
    Count last_tower_count;
    vector<double> coverage;

    vector<Creep> ConvertToCreeps(const vector<int>& creeps_int) {
        Count N = creeps_int.size()/4;
        vector<Creep> creeps(N);
        auto& cc = creeps_int;
        for (int i = 0; i < N; ++i) {
            // can store just pointers inside Creep structure
            // and take everything by index
            Index id = cc[4*i];
            creeps[i] = Creep{id, {cc[4*i+3], cc[4*i+2]}, cc[4*i+1]};
        }
        return creeps;
    }

    void AddTower(vector<int>& ts, const TowerPosition& tp) {
        ts.push_back(tp.position.col);
        ts.push_back(tp.position.row);
        ts.push_back(tp.tower);
    }
    
public:

    int init(vector<string> board, 
             int money, 
             int creep_health, 
             int creep_money, 
             vector<int> tower_type) {
        fill(creep_spawn.begin(), creep_spawn.end(), -1);
        vector<Tower> towers;
        for (int i = 0; i < tower_type.size()/3; ++i) {
            towers.emplace_back(tower_type[3*i], 
                                tower_type[3*i+1], 
                                tower_type[3*i+2]);   
        }
        // why we do this shit dynamically???
        this->board = new Board(board, towers);
        this->simulator = new Simulator(*(this->board));
        this->place = new PlaceTower(*(this->board));
        coverage.resize(this->board->spawn_loc_count(), 0);
        return 1;
    }     
    
    vector<int> placeTowers(const vector<int>& creeps_int, 
                            int money, 
                            vector<int>& base_health) {
        // need to make it smarter
        int m = *min_element(base_health.begin(), base_health.end());
        if (m < 400) {
            return vector<int>();
        }
        const auto creeps = ConvertToCreeps(creeps_int);
        vector<Index> spawns(creeps.size());
        for (Index i = 0; i < creeps.size(); ++i) {
            auto& c = creeps[i];
            if (creep_spawn[c.id] == -1) {
                creep_spawn[c.id] = board->spawn(c.pos);
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
            tie(route_hp_break, break_through) = simulator->Simulate(creeps, spawns);
            if (!break_through.empty()) {
                place->Place(route_hp_break, break_through, money);
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
    
    ~PathDefense_2() {
        delete simulator;
        delete board;
    }
};

