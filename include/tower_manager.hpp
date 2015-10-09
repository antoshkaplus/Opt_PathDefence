//
//  tower_placement.hpp
//  PathDefence
//
//  Created by Anton Logunov on 10/7/15.
//
//

#pragma once

#include "board.hpp"
#include "util.hpp"

struct TowerScope {
    vector<Position> positions;
    vector<Index> bounds;
};

// going to place towers from this class

// also this class should know everything cool
class TowerManager {
    
    Board* board_; 
    const vector<Tower>* towers_;
    
    vector<TowerPosition> placed_towers_;
    vector<Position> open_tower_positions_;
    // reserve 0 items if road or base
    Grid<vector<TowerScope>> tower_scopes_;

public:
    TowerManager() {}

    TowerManager(Board& board, const vector<Tower>& towers) {
        Init(board, towers);        
    }
    
    void Init(Board& board, const vector<Tower>& towers) {
        board_ = &board;
        towers_ = &towers;
        
        const auto& b = *board_; 
        const auto& t = *towers_; 
        Count N = b.size();
        // computing open_tower_positions
        tower_scopes_.resize(N, N);
        Region R{{0, 0}, {N, N}}; 
        auto func = [&](const Position& p) {
            assert(R.hasInside(p));
            if (b.IsEmpty(p)) {
                auto& v = tower_scopes_(p); 
                for (auto i = 0; i < t.size(); ++i) {
                    v.push_back(ComputeTowerScope({i, p}));
                }
                // can think about not including position if no 
                // positions in scope
                // may also divide good open position and everything else
                open_tower_positions_.push_back(p);
            } 
            else 
            {
                tower_scopes_(p).reserve(0);
            }
        };
        R.ForEach(func);
    }
    
    
    void PlaceTower(const TowerPosition& tp) {
        auto& b = *board_; 
        auto& p = tp.position;
        b.PlaceTower(p);
        placed_towers_.push_back(tp);
        auto& pp = open_tower_positions_;
        pp.erase(find(pp.begin(), pp.end(), tp.position));
    }
    
    const vector<TowerPosition>& placed_towers() const {
        return placed_towers_;
    }
    
    const vector<Tower>& towers() const {
        return *towers_;
    } 
    
    const vector<Position> open_tower_positions() const {
        return open_tower_positions_;
    }
    
    const TowerScope& tower_scope(const TowerPosition& tp) const {
        return (tower_scopes_(tp.position))[tp.tower];
    }

    const Board& board() const {
        return *board_;
    }
    
    
private:

    
    TowerScope ComputeTowerScope(const TowerPosition& tp) {
        vector<Position> scope;
        vector<Count> score;
        const auto& t = *towers_;
        const auto& b = *board_;
        auto N = b.size();
        assert(tp.tower < t.size());
        auto rng = t[tp.tower].rng;
        auto p = tp.position;
        for (auto r = max(p.row - rng-1, 0); r < min(p.row + rng+1, N); ++r) {
            for (auto c = max(p.col - rng-1, 0); c < min(p.col + rng+1, N); ++c) {
                Position q{r, c};
                auto d = (r - p.row)*(r - p.row) + (c - p.col)*(c - p.col); 
                if (d <= rng*rng && b.IsRoad(q)) {
                    scope.push_back(q);
                    score.push_back(d);
                }
            }
        }
        vector<Index> inds(score.size());
        iota(inds.begin(), inds.end(), 0);
        sort(inds.begin(), inds.end(), [&](Index i_0, Index i_1) {
            return score[i_0] < score[i_1];
        });
        TowerScope res;
        for (Index i : inds) {
            res.positions.push_back(scope[i]);
        } 
        
        for (Index i = 1; i < res.positions.size(); ++i) {
            if (score[inds[i-1]] != score[inds[i]]) { // res.positions[i-1] != res.positions[i]) {
                res.bounds.push_back(i);
            }
        }
        res.bounds.push_back(res.positions.size());
        return res;
    }
    


};
