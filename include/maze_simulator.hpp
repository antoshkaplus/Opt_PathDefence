//
//  maze_simulator.hpp
//  PathDefence
//
//  Created by Anton Logunov on 10/11/15.
//
//

#pragma once

#include "util.hpp"
#include "maze.hpp"

class MazeSimulator {

    struct Shadow {
        Count hp;
        Index id;
        Position pos;
        Index prev;
        
        Shadow() {}
        
        Shadow(Index id, Count hp, const Position& pos, Index prev) 
        : hp(hp), id(id), pos(pos), prev(prev) {}
    };
    
    
    double shadow_strength_;
    const Maze* maze_;
    const TowerManager* tower_manager_;
    vector<Shadow> history_;
    vector<Shadow> current_;

    vector<MazeBreakThrough> break_through_;

public:
    void Init(const Maze& maze, const TowerManager& tower_manager) {
        shadow_strength_ = 1.;
        maze_ = &maze;
        tower_manager_ = &tower_manager;
    }

    void Simulate(const vector<Creep>& creeps, const vector<Position>& prev_locs) {
        history_.clear();
        current_.clear();
        break_through_.clear();
        
        auto& b = tower_manager_->board(); 
        Init(creeps, prev_locs);
        while (!current_.empty()) {
            MoveShadows();
            ShootTowers();
        }
        for (auto& h : history_) {
            if (b.IsBase(h.pos)) {
                // should make history
                Path path;
                auto s = h;
                while (true) {
                    path.push_back(s.pos);
                    if (s.prev == -1) {
                        break;
                    }
                    s = history_[s.prev];
                }
                break_through_.emplace_back(h.hp, path);
            }
        }
    }
   
    const vector<MazeBreakThrough>& break_through() const {
        return break_through_;
    }
    
    
    
private:
    void Init(const vector<Creep>& creeps, const vector<Position>& prev_locs) {
        for (auto i = 0; i < creeps.size(); ++i) {
            auto& c = creeps[i];
            history_.emplace_back(c.id, c.hp, prev_locs[i], -1);
        }
        auto i = 0;
        for (auto& c : creeps) {
            current_.emplace_back(c.id, c.hp, c.pos, i);
            ++i;
        }    
    }
    
    void MoveShadows() {
        vector<Shadow> next;
        auto& m = *maze_; 
        auto& b = tower_manager_->board();
        Count offset = history_.size();
        int c_i = 0;
        for (auto& c : current_) {
            auto dirs = m.Next(c.pos, history_[c.prev].pos);
            auto C = count(dirs.begin(), dirs.end(), true);
            for (auto i = 0; i < kDirCount; ++i) {
                if (!dirs[i]) continue;
                auto p = c.pos;
                p.Shift(i);
                next.emplace_back(c.id, c.hp*shadow_strength_/C, p, offset + c_i);
            }
            ++c_i;
        }
        history_.insert(history_.end(), current_.begin(), current_.end());
        auto rem_start = remove_if(next.begin(), next.end(), [&](const Shadow& s) {
            return b.IsBase(s.pos);
        });
        history_.insert(history_.end(), rem_start, next.end());
        current_.assign(next.begin(), rem_start);
    }
    
    void ShootTowers() {
        unordered_multimap<Position, Index> positioning_;
        for (auto i = 0; i < current_.size(); ++i) {
            positioning_.emplace(current_[i].pos, i);
        }
        auto& ts = tower_manager_->towers();
        for (auto& pt : tower_manager_->placed_towers()) {
            Index first = 0;
            auto& sc = tower_manager_->tower_scope(pt);
            for (Index last : sc.bounds) {
                vector<Index> inds;
                for (Index j = first; j < last; ++j) {
                    auto r = positioning_.equal_range(sc.positions[j]);
                    for (auto it = r.first; it != r.second; ++it) {
                        inds.push_back(it->second);
                    }
                }
                // now we have array of minions on equal distance
                sort(inds.begin(), inds.end(), [&] (Index i_0, Index i_1) {
                    return current_[i_0].id < current_[i_1].id;
                });
                if (ShootAliveCreep(inds, ts[pt.tower].dmg)) {
                    break;
                }
            }
        }
        auto start = remove_if(current_.begin(), current_.end(), [](const Shadow& s) {
            return s.hp <= 0;
        });
        current_.erase(start, current_.end()); 
    }

    bool ShootAliveCreep(const vector<Index>& creep_indices, Count dmg) {
        for (auto i : creep_indices) {
            if (current_[i].hp > 0) {
                current_[i].hp -= dmg;
                return true;
            }
        }
        return false;
    }

};