//
//  util.cpp
//  PathDefence
//
//  Created by Anton Logunov on 6/2/15.
//
//

#include "util.hpp"
#include "board_2.hpp"


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


vector<double> ComputeCoverage(const Board_2& b, const TowerPosition& tp) {
    auto& s = b.tower_scope(tp);
    vector<double> coverage(b.spawn_loc_count(), 0);
    for (Position p : s.positions) {
        // need first for spawn index
        auto& bn = b.nexts(p);
        for (auto& q : bn) {
            coverage[q.first] += 1;
        }
    }
    return coverage;
}



// coverage starts with 0-s and it's equal to number of routes
// break through : spawn
void PlaceTower(vector<Count>& route_miss_hp, 
                Board_2& board, 
                vector<double>& current_coverage, 
                Count& money,
                const vector<BreakThrough>& break_through) {
    bool no_hp_miss = true;
    for (auto c : route_miss_hp) {
        if (c > 0) {
            no_hp_miss = false;
            break;
        }
    }
    if (no_hp_miss) {
        return;
    }
    
    TowerPosition best_tower_position;
    Score best_score{static_cast<Count>(current_coverage.size()), numeric_limits<double>::max()};
    Count best_count = 0; 
    vector<double> best_coverage(current_coverage.size(), 0);
    vector<double> buf_coverage(current_coverage.size());
    auto sum = [](double c_0, double c_1) {
        return c_0 + c_1;
    };
    auto can_wound = [](const Position& creep, const Position& tower, const Position& base) {
        Indent c = creep - base;
        Indent t = tower - base;
        int d = (c.row*c.row + c.col*c.col) - (t.col*t.col + t.row*t.row);
        return d >= 4;
    };
    auto& ts = board.towers();
    for (const Position& p : board.open_tower_positions()) {
        int k = 0;
        for (int i = 1; i < ts.size(); ++i) {
            if (ts[k].cost > ts[i].cost) {
                k = i;
            }
        }
        for (Index i = 0; i < ts.size(); ++i) {
            auto coverage = ComputeCoverage(board, {i, p});
            //if (i != k) continue;
            auto count = int(accumulate(coverage.begin(), coverage.end(), 0));
            transform(coverage.begin(), coverage.end(), coverage.begin(), [&](double d) {
                if (d > 1) {
                    d = (d-1)/2 + 1;
                }
                return d*ts[i].dmg;
            });
            if (count == 0) {
                continue;
            }
            bool stupid = true;
            // set another coverage to see how good it is against all those current creeps
            for (auto i = 0; i < coverage.size(); ++i) {
                if (route_miss_hp[i] > 0 && coverage[i] > 0.5) {
                    stupid = false;
                    break;
                }
                //coverage[i] /= count;
            }
            if (stupid) continue;
            stupid = true;
            for (auto& b : break_through) {
                auto bb = board.base_loc_for_spawn(b.spawn_loc);
                if (!bb.second) continue;
                if (coverage[b.spawn_loc] > 0 && can_wound(b.cur_loc, p, bb.first)) {
                    stupid = false;
                    break;
                }
            }
            if (stupid) continue;
            transform(coverage.begin(), coverage.end(), 
                      current_coverage.begin(), 
                      buf_coverage.begin(), sum);
            // need to inlude somehow how much hp it takes out or something like this
            // should be able to recievce iterators
            Score score{buf_coverage};
            if (money >= ts[i].cost && (best_score.zeros == current_coverage.size() || 
                (score.IsBetterThan(best_score) || (best_score.min - score.min < 1. && 1.*best_count*ts[best_tower_position.tower].dmg/ts[best_tower_position.tower].cost < 1.*count*ts[i].dmg/ts[i].cost)))) {
                best_count = count;
                best_score = score;
                best_coverage = coverage;
                best_tower_position = TowerPosition(i, p);
            } 
        }
    }
    // could'n find anyhing
    if (best_count == 0) return;
    auto cost = ts[best_tower_position.tower].cost;
    if (money < cost) return;
    board.PlaceTower(best_tower_position);
    money -= cost;
    transform(current_coverage.begin(), 
              current_coverage.end(), 
              best_coverage.begin(), 
              current_coverage.begin(), sum);
    
    // counting how many positions bad against vawas... so can reduce points by two, 
    // but 1 is minimum
    
    // or just has his path in range
    // also we consider if tower would kill unwanted creep(s) 
    // or at least takes maximum of his health ???
}



