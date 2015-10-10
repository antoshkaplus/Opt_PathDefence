//
//  simulator.hpp
//  PathDefence
//
//  Created by Anton Logunov on 5/30/15.
//
//

#pragma once


#include <unordered_map>
#include <map>

#include "tower_manager.hpp"
#include "next.hpp"


// simulating creep movement on the given board
// under tower shooting.

// can have some statistics inside
// not really forsed to return it right away
class Simulator {
    
    const Board* board_;
    const TowerManager* tower_manager_;
    const Next* next_;
    
    // going to update creeps in place
    vector<Creep>* creeps_; 
    // after creep moves into base skip him
    // when creep becomes dead also skip him
    Count skip_count_;
    vector<bool> skip_;
    // creep location => index in vector
    unordered_multimap<Position, Index> creep_positioning_;
    
public:

    Simulator() {}
    Simulator(const Board& board, const TowerManager& tower_manager, const Next& next) 
        : board_(&board), tower_manager_(&tower_manager), next_(&next) {}
    
    void Init(const Board& board, const TowerManager& tower_manager, const Next& next) {
        board_ = &board;
        tower_manager_ = &tower_manager;
        next_ = &next;
    }
    
    void Simulate(vector<Creep>& creeps) {
        creeps_ = &creeps;
        skip_.resize(creeps.size());
        fill(skip_.begin(), skip_.end(), false);
        skip_count_ = 0;

        while (skip_count_ != creeps.size()) {
            CreepsMove();
            TowersShoot();
            
        }
    }
    
private:

    void CreepsMove() {
        auto& cs = *creeps_;
        auto& nt = *next_;
        auto& b = *board_;
        creep_positioning_.clear();
        for (auto i = 0; i < cs.size(); ++i) {
            if (skip_[i]) continue;
            cs[i].pos = nt.next(cs[i]);
            creep_positioning_.emplace(cs[i].pos, i);
            ++cs[i].ticks;
            if (b.IsBase(cs[i].pos)) {
                skip_[i] = true;
                ++skip_count_;
            }
        }
    }

    void TowersShoot() {
        // now we do shooting
        auto& cs = *creeps_;
        auto& ts = tower_manager_->towers();
        for (auto& pt : tower_manager_->placed_towers()) {
            Index first = 0;
            auto& sc = tower_manager_->tower_scope(pt);
            for (Index last : sc.bounds) {
                vector<Index> inds;
                for (Index j = first; j < last; ++j) {
                    auto r = creep_positioning_.equal_range(sc.positions[j]);
                    for (auto it = r.first; it != r.second; ++it) {
                        inds.push_back(it->second);
                    }
                }
                // now we have array of minions on equal distance
                sort(inds.begin(), inds.end(), [&] (Index i_0, Index i_1) {
                    return cs[i_0].id < cs[i_1].id;
                });
                if (ShootAliveCreep(inds, ts[pt.tower].dmg)) {
                    break;
                }
            }
        }
    }
    
    void ComputeCreepPositioning() {
        creep_positioning_.clear();
        const auto& cs = *creeps_; 
        for (Index i = 0; i < cs.size(); ++i) {
            if (skip_[i]) continue; 
            creep_positioning_.insert({cs[i].pos, i});
        }
    }
    
    bool ShootAliveCreep(const vector<Index>& creep_indices, Count dmg) {
        auto& cs = *creeps_;
        for (auto i : creep_indices) {
            if (cs[i].hp > 0) {
                cs[i].hp -= dmg;
                if (cs[i].hp <= 0) {
                    skip_[i] = true;
                    ++skip_count_;
                }
                return true;
            }
        }
        return false;
    }

};
