//
//  board_2.hpp
//  PathDefence
//
//  Created by Anton Logunov on 6/2/15.
//
//

#ifndef PathDefence_board_2_hpp
#define PathDefence_board_2_hpp

#include "board.hpp"

struct TowerScope {
    vector<Position> positions;
    vector<Index> bounds;
};


class Board_2 : public Board {
    
    vector<Tower> towers_;
    vector<TowerPosition> placed_towers_;
    vector<Position> open_tower_positions_;
    Grid<vector<TowerScope>*> tower_scopes_;
    
    TowerScope ComputeTowerScope(const TowerPosition& tp) {
        vector<Position> scope;
        vector<Count> score;
        assert(tp.tower < towers_.size());
        auto rng = towers_[tp.tower].rng;
        auto p = tp.position;
        for (auto r = max(p.row - rng-1, 0); r < min(p.row + rng+1, N); ++r) {
            for (auto c = max(p.col - rng-1, 0); c < min(p.col + rng+1, N); ++c) {
                Position q{r, c};
                auto d = (r - p.row)*(r - p.row) + (c - p.col)*(c - p.col); 
                if (d <= rng*rng && IsRoad(q)) {
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
    
public:
    
    Board_2(vector<string>& b, vector<Tower>& ts) 
            : Board(b), towers_(ts) {
        // computing open_tower_positions
        tower_scopes_.resize(N, N);
        Region R{{0, 0}, {N, N}}; 
        auto func = [&](const Position& p) {
            assert(R.hasInside(p));
            if (IsEmpty(p)) {
                tower_scopes_(p) = new vector<TowerScope>;
                auto& v = *tower_scopes_(p); 
                for (Index i = 0; i < towers_.size(); ++i) {
                     v.push_back(ComputeTowerScope({i, p}));
                }
                open_tower_positions_.push_back(p);
            }
        };
        R.ForEach(func);
    }
    
    void PlaceTower(const TowerPosition& tp) {
        auto& p = tp.position;
        picture_[p.row][p.col] = CELL_TOWER;
        placed_towers_.push_back(tp);
        auto& pp = open_tower_positions_;
        pp.erase(find(pp.begin(), pp.end(), tp.position));
    }
    
    const vector<TowerPosition>& placed_towers() const {
        return placed_towers_;
    }
    
    const vector<Tower>& towers() const {
        return towers_;
    } 
    
    const vector<Position> open_tower_positions() const {
        return open_tower_positions_;
    }
    
    const TowerScope& tower_scope(const TowerPosition& tp) const {
        return (*tower_scopes_(tp.position))[tp.tower];
    }
};


#endif
