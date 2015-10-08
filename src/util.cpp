//
//  util.cpp
//  PathDefence
//
//  Created by Anton Logunov on 6/2/15.
//
//

#include "util.hpp"
#include "tower_manager.hpp"

//
//vector<Position> open_tower_positions;
//vector<double> coverage;

struct Score {
    double min;
    Count zeros;
    
    // best function score ever!
    Score(const vector<double>& c) {
        zeros = 0;
        min = numeric_limits<double>::max();
        for (auto m : c) {
            if (m == 0) {
                //   ++zeros; 
            } else if (m < min) {
                min = m;
            }
        }
    }
    
    Score(Count zeros, double min) 
        : min(min), zeros(zeros) {}
    
    bool IsBetterThan(const Score& s) {
        return zeros < s.zeros || (zeros == s.zeros && min > s.min);
    }
};


vector<double> ComputeCoverage(const TowerManager& m, const TowerPosition& tp) {
    auto& s = m.tower_scope(tp);
    vector<double> coverage(m.board().spawn_loc_count(), 0);
    for (Position p : s.positions) {
        (void)p;
        // need first for spawn index
//        auto& bn = b.nexts(p);
//        for (auto& q : bn) {
//            coverage[q.first] += 1;
//        }
    }
    return coverage;
}





