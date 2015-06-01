//
//  simulator.hpp
//  PathDefence
//
//  Created by Anton Logunov on 5/30/15.
//
//

#ifndef PathDefence_simulator_hpp
#define PathDefence_simulator_hpp

#include <unordered_map>

#include "board.hpp"



class Simulator {
public:
    
    struct CreepInfo {
        Position prev_location;
        Index spawn;
         
    };
    
    Board board_;
    vector<Tower> towers;
    vector<TowerPosition> tower_positions;
    // each tower, path points sorted by range  
    vector<vector<Index>> tower_scope;
    vector<vector<Index>> tower_scope_bounds;
    
    // creep id
    unordered_map<Index, CreepInfo> creep_info; 
    
    
    
    map<Position, Index> dd;
    
    Simulator(const Board& board, vector<Tower>& towers, vector<TowerPosition>& tower_positions) {
    
    }
    
    void Train(vector<Creep>& creep) {
        for (Creep& c : creep) {
            auto it = creep_info.find(c.id);
            if (it == creep_info.end()) {
                creep_info[c.id] = {c.pos, board_.spawn(c.pos)};
            } else {
                auto& info = it->second;
                board_.set_next(info.spawn, info.prev_location, c.pos);
            }
        }
    }
    
    
    // for each creep we assign spawn location
    vector<Count> Simulate(vector<Creep>& creep, vector<Index> spawn_loc) {
        vector<Index> alive(creep.size());
        iota(alive.begin(), alive.end(), 0);
        vector<Count> route_hp_break(board_.spawn_loc_count(), 0);
        while (!alive.empty()) {
            // creep moves and attacks
            for (Index i = 0; i < alive.size();) {
                auto s = spawn_loc[alive[i]];
                auto& c = creep[alive[i]];
                auto n = board_.next(s, c.pos);
                if (!n.second) {
                    if (board_.IsBaseNearby(c.pos)) {
                        route_hp_break[s] = c.hp;
                        swap(alive.back(), alive[i]);
                        alive.pop_back();
                    } else {
                        ++i;
                    }
                } else {
                    c.pos = n.first;
                    ++i;
                }
            } 
            // now we do shooting
            for (Index i = 0; i < tower_positions.size(); ++i) {
                Tower& t = towers[tower_positions[i].tower];
                // could make use of some big chunk of memory instead of those small arrays
                
                
                // want to have continious memory ?? 
                Index first = 0;
                Index creep_to_kill = -1;
                for (Index last : tower_scope_bounds[i]) {
                    for (Index j = first; j < last; ++j) {
                        auto r = creep_pos.equal_range(tower_scope[i][j]);
                        for (auto it = r.first; it != r.second; ++it) {
                            Index x = *it;
                            if (creep[x].hp > 0 && (creep_to_kill == -1 || creep[creep_to_kill].id > creep[x].id)) {
                                creep_to_kill = x;
                            } 
                        }
                    }
                    if (creep_to_kill != -1) break;
                    first = last;
                }
                if (creep_to_kill != -1) {
                    creep[creep_to_kill].hp -= t.dmg;
                    if (creep[creep_to_kill].hp <= 0) {
                        --creeps_alive;
                    }
                }
            }
            
            // towers made their shit
        }   
    
       
        // now we can compute stuff that left alive
        return miss_hp_per_route;
    
    
        // each tower shoots to nearest location (they are sorted) that contain creep inside, if
        // just use binary search??? // can use loop actually // and vector in each location
        // creep has hp > 0
        
        // remove all dead creeps 
        // while at least one creep alive creep 
        // for each creep we simulate where he goes
        
        // if next position is base we define that creep got to base with certain hp
        
        // we also know each position he went through
        // return vector of creeps with hp who got through
    }
    
    
    vector<Position> open_tower_positions;
    vector<double> coverage;
    
    double Score(vector<double>& c) {
        return accumulate(c.begin(), c.end(), 1, [](double c_0, double c_1) {
            return c_0*c_1;
        });
    }
    
    void FindTowerPlacements(vector<Count>& route_miss_hp) {
        if (accumulate(route_miss_hp.begin(), route_miss_hp.end(), 0) == 0) return;
        
        TowerPosition best_tower_position;
        double best_score = 0;
        Count best_pos_count = numeric_limits<int>::max(); 
        for (const Position& p : open_tower_positions) {
            for (Index i = 0; i < towers.size(); ++i) {
                auto s = ComputeScope(TowerPosition(i, p));
                // returns indices of paths
                // need to compute set of all routes for this tower
                // if nothing interesting we just skip it
                bool stupid = true;
                for (int j : s) {
                    if (board.some[j].count()) {
                    
                    }
                }
                vector<double> coverage(route_count());
                Count count = 0;
                for (int j : s) {
                    // first - route index
                    // 
                    for (auto pp : board.some[j]) {
                       coverage[pp.first] += 1.;
                       count += 1; 
                    }
                }
                for (auto& c : coverage) {
                    c /= count;
                }
                transform(coverage.begin(), coverage.end(), this->coverage.begin(), this->coverage.end(), [](double c_0, double c_1) {
                    return c_0 + c_1;
                });
                // need to inlude somehow how much hp it takes out or something like this
                // should be able to recievce iterators
                double score = Score(coverage + this->coverage);
                if (score > best_score || (abs(best_score - score) < 0.001 && best_pos_count < s.size())) {
                    best_pos_count = s.size();
                    best_score = score;
                    best_tower_position = TowerPosition(i, p);
                } 
            }
        }
        PlaceTower(best_tower_position);
        
        
        // also base
        
        // coverage is vector of doubles
        
        // want to normilize between tours // so each tour would have certain coverage
        
        // we can sort all possible locations ??? 
        // coverage is calculated by considering all placed towers:
        // how many tours inside, how many points for each tour: sum points and get persantage for everyone... and after that multiply persantage by points number
        // put this number inside coverage for every tour
        
        // then we just use some metric to make thing big and equal:
        // multiply everyone?
        
        // counting how many positions bad against vawas... so can reduce points by two, 
        // but 1 is minimum
        
        // or just has his path in range
        // also we consider if tower would kill unwanted creep(s) 
        // or at least takes maximum of his health ???
    }
    
    
    
    
    
    
    
    
};

#endif
