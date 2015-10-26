//
//  maze_routes.hpp
//  PathDefence
//
//  Created by Anton Logunov on 10/20/15.
//
//

#pragma once

#include "util.hpp"
#include "maze.hpp"
 

class MazeRoutes {

    const static int MAX_ROUTE_COUNT = 64;
    
    using RouteSet = bitset<MAX_ROUTE_COUNT>; 

    
    vector<Route> route_set_;

    unordered_map<Position, RouteSet> routes_;
    
    const Board* board_;
public:
    
    MazeRoutes() {}
    
    void Init(const Board& board) {
        board_ = &board;   
    }
  
    void CheckIn(Index spawn, Path path) {
        Route r{board_->base(path.back()), spawn};
        Index r_index = route_index(r);
        
        for (Position p : path) {
            if (p == path.back()) continue;
            auto it = routes_.find(p);
            if (it == routes_.end()) {
                it = routes_.emplace(p, 0).first;
            }
            set_route(it->second, r_index);
        }
    }
    
    Index route_index(Index spawn, const Position& base) const {
        Route r{board_->base(base), spawn};
        return route_index(r);
    }
    
    Count CountRoutes(const vector<Position>& ps) const {
        RouteSet set = 0;
        for (auto p : ps) {
            auto it = routes_.find(p);
            if (it == routes_.end()) continue;
            set |= it->second;
        }
        return set.count();
    }
    
    Count total_count() const {
        return route_set_.size();
    }
    
    const Route& route(Index route_index) const {
        return route_set_[route_index];
    } 
    
    bool in_route(const Position& p, Index route_index) const {
        auto it = routes_.find(p);
        if (it == routes_.end()) {
            return false;
        }
        return it->second.test(route_index);
    }
    
    
private:
    
    Index route_index(Route r) const {
        for (Index i = 0; i < route_set_.size(); ++i) {
            if (route_set_[i] == r) {
                return i;
            }
        }
        return -1;
    }
    
    Index route_index(Route r) {
        const auto& s = *this;
        Index res = s.route_index(r);
        if (res == -1) {
            res = route_set_.size();
            route_set_.push_back(r);
        }
        return res;
    }
    
    void set_route(RouteSet& set, Index route_index) {
        set.set(route_index);
    }
    


};