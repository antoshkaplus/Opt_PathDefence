//
//  maze_coverage.hpp
//  PathDefence
//
//  Created by Anton Logunov on 10/23/15.
//
//

#ifndef maze_coverage_h
#define maze_coverage_h

#include "maze.hpp"
#include "maze_routes.hpp"
#include "tower_manager.hpp"


class MazeCoverage {
    const MazeRoutes* routes_;
    const TowerManager* mngr_;
    const Board* board_;
  
    // how much towers cover particular route
    vector<double> weights_;
    // if one spawn lead to multiple bases: 
    // factor to multiply weight (number of bases)
    vector<double> factors_;

public:

    void Init(const Board& board, const TowerManager& mngr, const MazeRoutes& maze_routes) {
        board_ = &board;
        mngr_ = &mngr;
        routes_ = &maze_routes;
    }

    void Compute() {
        auto& b = *board_; 
        auto& rs = *routes_;
        auto& mngr =  *mngr_; 
        auto r_count = rs.total_count();
        weights_.resize(r_count);
        fill(weights_.begin(), weights_.end(), 0);
        factors_.resize(r_count);
        fill(factors_.begin(), factors_.end(), 0);
        
        for (auto spawn = 0; spawn < b.spawn_loc_count(); ++spawn) {
            Count count = 0;
            for (auto base = 0; base < b.base_count(); ++base) {
                for (auto r = 0; r < r_count; ++r) {
                    auto rr = rs.route(r);
                    if (rr.base == base && rr.spawn == spawn) {
                        ++count; 
                    } 
                }
            }
            for (auto r = 0; r < r_count; ++r) {
                auto rr = rs.route(r);
                if (rr.spawn == spawn) factors_[r] = count;
            }
        }
        
        // now have to compute weights
        for (auto tp : mngr.placed_towers()) {
            auto ws = ComputeForTower(tp);
            for (auto i = 0; i < ws.size(); ++i) {
                weights_[i] += ws[i];
            }
        }
    }

    vector<double> Coverage(const TowerPosition& tp) {
        auto ws = ComputeForTower(tp);
        for (auto i = 0; i < ws.size(); ++i) {
            ws[i] += weights_[i];
            ws[i] *= factors_[i];
        }
        return ws;
    }

private:
    vector<double> ComputeForTower(const TowerPosition& tp) {
        auto& mngr = *mngr_;
        auto& rs = *routes_; 
        auto r_count = rs.total_count();
        vector<double> ws(r_count, 1);
        for (auto p : mngr.tower_scope(tp).positions) {
            for (auto r = 0; r < r_count; ++r) {
                ws[r] += (rs.in_route(p, r) ? 1 : 0);
            }
        }
        auto cost = mngr.tower(tp.tower).cost;
        for (auto r = 0; r < r_count; ++r) {
            ws[r] =  cost * log(ws[r]);
        }
        return ws;
    }
};



#endif /* maze_coverage_h */
