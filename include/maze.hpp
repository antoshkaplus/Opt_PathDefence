//
//  maze.hpp
//  PathDefence
//
//  Created by Anton Logunov on 10/11/15.
//
//

#pragma once

#include "util.hpp"
#include "board.hpp"


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
        
        struct CrossroadDeduction {
            Position next;
            bool success;
        };
        
        CrossroadDeduction DeductCroossroadFromSpawn(const Position& pos, grid::Direction dir) {
            CrossroadDeduction res;
            auto& m = *maze_;
            // has something that goes out?
            if (m.IsInitialized(pos)) {
                // have nothing to do
                res.success = false;
                return res;
            }
            auto next_dir = -1;
            Count in_count = 0;
            for (auto d = 0; d < kDirCount; ++d) {
                if (d == dir) continue; 
                auto from = pos + kDirVector[d]; 
                auto exists = m.Next(from)[kDirOpposite[d]];
                if (exists) {
                    ++in_count;
                } else {
                    next_dir = d;
                }
            }
            Possible possible = PossibleDirections(pos, dir);
            if (possible.count != in_count + 1) {
                // to many possible directions out
                // should be only one
                res.success = false;
                return res;
            } 
            res.next = pos.Shifted(next_dir);
            res.success = true;  
            return res;
        }
        
        
        struct Possible {
            Direction dirs;
            Count count;
        };
        
        Possible PossibleDirections(const Position& pos, grid::Direction dir) {
            auto& b = *board_;
            Possible res;
            res.count = 0;
            for (auto d = 0; d < kDirCount; ++d) {
                res.dirs[d] = false;
                if (d == dir) continue;
                auto sh = pos.Shifted(d);
                if (b.IsInside(sh) && (b.IsRoad(sh) || b.IsBase(sh))) {
                    ++res.count;
                    res.dirs[d] = true;
                }
            }
            return res;
        }
        
        void DeductFromSpawn(const Position& pos, grid::Direction dir) {
            auto& b = *board_;
            
            path_.push_back(pos);
            if (b.IsBase(pos)) {
                return;
            }
            
            Count n = 0;
            Position next;
            for (auto d = 0; d < kDirCount; ++d) {
                if (d == dir) continue;
                auto sh = pos.Shifted(d);
                if (b.IsInside(sh) && (b.IsRoad(sh) || b.IsBase(sh))) {
                    ++n;
                    next = sh;
                }
            }
            assert(n != 0);
            if (n > 1) {
                auto deduction = DeductCroossroadFromSpawn(pos, dir);
                if (!deduction.success) {
                    return;   
                } 
                next = deduction.next;
            }
            DeductFromSpawn(next, FromDirVector(pos - next));
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
            DeductFromSpawn(p, -1);
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
    
    // excluding direction to previous position
    const Direction Next(const Position& pos, const Position& prev) const {
        Direction res = Next(pos);
        auto back_dir = FromDirVector(prev-pos);
        res[back_dir] = false;
        return res;
    }
    
    bool IsInitialized(const Position& pos) const {
        return avail_next_.find(pos) != avail_next_.end();
    }
    
    

private:
    
    
    
    
    void DeductFromSpawns(const Board& b) {
        Path path;
        
    }
    


};