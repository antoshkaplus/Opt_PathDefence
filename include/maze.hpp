//
//  maze.hpp
//  PathDefence
//
//  Created by Anton Logunov on 10/11/15.
//
//

#pragma once

#include "util.hpp"


class Maze {
    
    using Direction = array<bool, kDirCount>;

    unordered_map<Position, Direction> avail_next_;

    class Deductor {
        const Board* board_;
        Maze* maze_;
        
        Path path_;
    public:    
        Deductor(const Board& board, Maze& maze)
        : board_(&board), maze_(&maze) {}
        
        void Deduct() {
            // deduction from bases should be called before
            // defuction from spawns
            DeductFromBases();
            DeductFromSpawns();
        }
        
    private:
        // dir - where is direction to go back from current position
        // from current position look for single way father.
        // if found continue recursion
        void DeductFromBase(const Position& pos, grid::Direction dir) {
            auto& b = *board_;
            
            path_.push_back(pos);
            
            Count n = 0;
            Position next;
            for (auto d = 0; d < kDirCount; ++d) {
                if (d == dir) continue;
                auto sh = pos.Shifted(d);
                if (b.IsInside(sh) && b.IsRoad(sh)) {
                    ++n;
                    next = sh;
                }
            }
            if (n != 1) {
                return;   
            }
            DeductFromBase(next, FromDirVector(pos - next));
        }
    
        void DeductFromBase(const Position& p) {
            auto& b = *board_;
            auto& m = *maze_;
            for (auto d = 0; d < kDirCount; ++d) {
                path_.clear();
                path_.push_back(p);
                auto next = p.Shifted(kDirOpposite[d]);
                // base is in the distance from board bounds
                if (!b.IsRoad(next)) continue;
                DeductFromBase(next, d);
                // maybe path just passes through this base
                if (path_.size() == 2) continue;
                for (auto i = 1; i < path_.size(); ++i) {
                    m.CheckIn(path_[i-1], path_[i]);
                }
            }
        }
        
        void DeductFromSpawn(const Position& p, grid::Direction dir) {
            auto& b = *board_;
            
            path_.push_back(pos);
            
            Count n = 0;
            Position next;
            for (auto d = 0; d < kDirCount; ++d) {
                if (d == dir) continue;
                auto sh = pos.Shifted(d);
                if (b.IsInside(sh) && b.IsRoad(sh)) {
                    ++n;
                    next = sh;
                }
            }
            if (n != 1) {
                return;   
            }
            DeductFromBase(next, FromDirVector(pos - next));
        }
        
        
        void DeductFromSpawn(const Position& p) {
            auto& m = *maze_;
            
            auto nn = m.Next(p);
            auto c = count(nn.begin(), nn.end(), true);
            assert(c > 1);
            // deduction found from base
            if (c == 1) return;
            path_.clear();
            path_.push_back(p);
            Deduct(p.Shifted(kDirOpposite[d]), d);
            for (auto i = 1; i < path_.size(); ++i) {
                m.CheckIn(path_[i], path_[i-1]);
            }
        }
        
        void DeductFromBases() {
            auto& b = *board_;
            for (auto& p : b.base_locs()) {
                DeductFromBase(p);
            }
        }
    
        void DeductFromSpawns() {
            auto& b = *board_;
            for (auto& p : b.spawn_locs()) {
                DeductFromSpawn(p);
            }
        }
    };


public:

    
    
    void Deduct(const Board& b) {
        Deductor(b, *this).Deduct();
        return;
        Path path;
        function<void(const Position&, grid::Direction)> op;
        op = [&](const Position& pos, grid::Direction dir) {
            if (!b.IsRoad(pos)) return;
            Count n = 0;
            Position next;
            for (auto d = 0; d < kDirCount; ++d) {
                if (d == dir) continue;
                auto sh = pos.Shifted(d);
                if (b.IsRoad(sh)) {
                    ++n;
                    next = sh;
                }
            }
            if (n != 1) {
                return;   
            }
            path.push_back(pos);
            op(next, FromDirVector(pos - next));
        };
        for (auto& p : b.base_locs()) {
            for (auto d = 0; d < kDirCount; ++d) {
                path.clear();
                path.push_back(p);
                op(p, d);
                for (auto i = 1; i < path.size(); ++i) {
                    CheckIn(path[i-1], path[i]);
                }
            }
        }
    }

    
    void CheckIn(const Position& pos, const Position& prev) {
        auto d = FromDirVector(pos - prev);
        auto it = avail_next_.find(prev);
        if (it == avail_next_.end()) {
            Direction ds = {{false, false, false, false}};
            it = avail_next_.emplace(prev, ds).first;
        }         
        it->second[d] = true;
    }

    const Direction& Next(const Position& pos) const {
        auto it = avail_next_.find(pos);
        if (it == avail_next_.end()) {
            static Direction empty = {{false, false, false, false}};
            return empty;
        }
        return it->second;
    }
    
    const Direction Next(const Position& pos, const Position& prev) const {
        Direction res = Next(pos);
        auto back_dir = FromDirVector(prev-pos);
        res[back_dir] = false;
        return res;
    }

private:
    
    
    
    
    void DeductFromSpawns(const Board& b) {
        Path path;
        
    }
    


};