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
#include <map>

#include "board_2.hpp"


class Simulator {
    
    Board_2& board_;

    bool IsAnyAlive(const vector<Creep>& cs) {
        for (auto& c : cs) {
            if (c.hp > 0) {
                return true;
            }
        }
        return false;
    }
    
public:

    Simulator(Board_2& board) : board_(board) {}
    
    // for each creep we assign spawn location
    tuple<vector<Count>, vector<BreakThrough>> Simulate(const vector<Creep>& creep_const) {
        vector<Creep> creep = creep_const;
        Count N = creep.size();
        // getting i din't know why
        vector<Position> creep_starting_positions(N);
        for (Index i = 0; i < N; ++i) {
            creep_starting_positions[i] = creep[i].pos;
        }
        // wtf???
        vector<Count> route_hp_break(board_.spawn_loc_count(), 0);
        vector<BreakThrough> break_through;
        // just for loop for everyone... and don't mess up with this array
        // may skip it actually
        Index iteration = 0;
        // why someone alive or ...
        while (IsAnyAlive(creep) && iteration++ < 200) {
            // creep moves and attacks
            for (Index i = 0; i < N; ++i) {
                auto& c = creep[i];
                if (c.hp <= 0) continue;
                auto s = spawn_loc[i];
                auto n = board_.next(s, c.pos);
                if (!n.second) {
                    if (board_.IsBaseNearby(c.pos)) {
                        // later should change this data
                        break_through.emplace_back(creep_starting_positions[i], s);
                        board_.set_base_for_spawn(s, c.pos);
                        route_hp_break[s] += c.hp;
                        c.hp = 0;
                    } else {
                        c.hp = 0;
                    }
                } else {
                    c.pos = n.first;
                }
            } 
            
            unordered_multimap<Position, Index> m;
            for (Index i = 0; i < N; ++i) {
                m.insert({creep[i].pos, i});
            }
            
            // now we do shooting
            for (auto& pt : board_.placed_towers()) {
                Index first = 0;
                // could just go with index
                auto& sc = board_.tower_scope(pt);
                for (Index last : sc.bounds) {
                    vector<Index> inds;
                    for (Index j = first; j < last; ++j) {
                        auto r = m.equal_range(sc.positions[j]);
                        for (auto it = r.first; it != r.second; ++it) {
                            inds.push_back(it->second);
                        }
                    }
                    // now we have array of minions on equal distance
                    sort(inds.begin(), inds.end(), [&] (Index i_0, Index i_1) {
                        return creep[i_0].id < creep[i_1].id;
                    });
                    for (auto i : inds) {
                        if (creep[i].hp > 0) {
                            auto& tt = board_.towers();
                            creep[i].hp -= tt[pt.tower].dmg;
                            goto next_tower;
                        }
                    }
                }
                next_tower:;
            }
        }   
        return {route_hp_break, break_through};
    }
};

#endif
