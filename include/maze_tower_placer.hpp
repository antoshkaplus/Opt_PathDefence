//
//  maze_tower_placer.hpp
//  PathDefence
//
//  Created by Anton Logunov on 10/11/15.
//
//

#pragma once

#include "tower_manager.hpp"
#include "maze_routes.hpp"
#include "maze_coverage.hpp"


class MazeTowerPlacer {

    // bunch of score parameters
    struct Params {
        double dmg_pow;
        double cost_pow;
    };

    template<class Score>
    struct Placement : TowerPosition {
        double score;

    private:
        const MazeTowerPlacer* placer_;
    
    public:
        Placement() {}
    
        Placement(TowerPosition placement, Score score, const MazeTowerPlacer& placer) 
        : TowerPosition(placement), score(score), placer_(&placer) {}
        
        bool IsBetterThan(const Placement& item) {
            auto& fs = placer_->tower_factors_; 
            auto& mngr = *placer_->tower_manager_;
            return score * fs[tower] > item.score * fs[item.tower]
                || (tower == item.tower && score == item.score && mngr.tower_scope(*this).positions.size() > mngr.tower_scope(item).positions.size());  
        }
    };


    const MazeRoutes* routes_; 
    TowerManager* tower_manager_; 
    Params params_;
    vector<double> tower_factors_;

    bool tower_placed_ = false;

public:
    
    void Init(const Board& board, TowerManager& tower_manager, const MazeRoutes& routes) {
        tower_manager_ = &tower_manager;
        routes_ = &routes;
        params_ = {1, 1};
        tower_factors_.resize(tower_manager.towers().size());
        ComputeTowerFactors();
    }

    void Place(const vector<MazeBreakThrough>& break_through, int& money) {
        tower_placed_ = false;
        if (break_through.empty()) return;
        
        CountMap<Position, UnorderedMap> bt_pos;
        for (auto& bt : break_through) {
            for (auto& p : bt.path) {
                if (p == bt.path.back()) continue;
                bt_pos.increase(p);
            }
        }
        
        auto& mngr = *tower_manager_;
        auto& ts = mngr.towers();
        
        Placement<Count> best;
        best.score = 0;
        
        bool is_initialized = false;
        auto op = [&](const TowerPosition& tp) {
            if (ts[tp.tower].cost > money) {
                return;
            }
            Count score = 0;
            for (auto& p : mngr.tower_scope(tp).positions) {
                auto it = bt_pos.find(p);
                if (it == bt_pos.end()) continue;
                score += it->second;
            }
            Placement<Count> pl{tp, score, *this};
            if (!is_initialized || pl.IsBetterThan(best)) {
                is_initialized = true;
                best = pl;
            }
        };
        mngr.ForEachOpenTowerPosition(op);
        if (best.score > 0) {
            tower_placed_ = true;
            mngr.PlaceTower(best);
            money -= ts[best.tower].cost;
        }
    }
    
    // need heuristic for current run probably
    void PlaceGlobally(const vector<MazeBreakThrough>& break_through, int& money) {
        tower_placed_ = false;
        if (break_through.empty()) return;
        
        auto& mngr = *tower_manager_;
        auto& ts = mngr.towers();
        auto& rs = *routes_;
        
        Placement<Count> best;
        best.score = 0;
        
        bool is_initialized = false;
        auto op = [&](const TowerPosition& tp) {
            if (ts[tp.tower].cost > money) {
                return;
            }
            Count score = rs.CountRoutes(mngr.tower_scope(tp).positions);
            Placement<Count> pl{tp, score, *this};
            if (!is_initialized || pl.IsBetterThan(best)) {
                is_initialized = true;
                best = pl;
            }
        };
        mngr.ForEachOpenTowerPosition(op);
        if (best.score > 0) {
            tower_placed_ = true;
            mngr.PlaceTower(best);
            money -= ts[best.tower].cost;
        }

    }

    void PlaceCombained(const vector<MazeBreakThrough>& break_through, int& money) {
        tower_placed_ = false;
        if (break_through.empty()) return;
    
        auto& mngr = *tower_manager_;
        auto& ts = mngr.towers();
        
        Placement<double> best;
        best.score = 0;
        
        bool is_initialized = false;
        Index index = 0;
        vector<double> cur_scores = ComputeCurrentScores(break_through, money);
        vector<Count> route_scores = ComputeRouteScores(break_through, money);
        auto op = [&](const TowerPosition& tp) {
            if (ts[tp.tower].cost > money) {
                return;
            }
            double score = cur_scores[index] + route_scores[index]; 
            Placement<double> pl{tp, score, *this};
            if (!is_initialized || pl.IsBetterThan(best)) {
                is_initialized = true;
                best = pl;
            }
            ++index;
        };
        
        mngr.ForEachOpenTowerPosition(op);
        if (best.score > 0) {
            tower_placed_ = true;
            mngr.PlaceTower(best);
            money -= ts[best.tower].cost;
        } else {
            //assert(false);
        }
        
    }
    
    
    void PlaceCombained_2(const vector<MazeBreakThrough>& break_through, int& money) {
        tower_placed_ = false;
        if (break_through.empty()) return;
        
        auto& mngr = *tower_manager_;
        auto& ts = mngr.towers();
        
        Placement<double> best;
        best.score = 0;
        
        bool is_initialized = false;
        Index index = 0;
        vector<double> cur_scores = ComputeCurrentScores(break_through, money);
        vector<double> route_scores = ComputeRouteScores_2(break_through, money);
        auto op = [&](const TowerPosition& tp) {
            if (ts[tp.tower].cost > money) {
                return;
            }
            double score = cur_scores[index] + route_scores[index]/5 + 100000; 
            Placement<double> pl{tp, score, *this};
            if (!is_initialized || pl.IsBetterThan(best)) {
                is_initialized = true;
                best = pl;
            }
            ++index;
        };
        
        mngr.ForEachOpenTowerPosition(op);
        if (best.score > 0) {
            tower_placed_ = true;
            mngr.PlaceTower(best);
            money -= ts[best.tower].cost;
        } else {
            //assert(false);
        }
        
    }
    
    
    void PlaceMoreCoverage(const vector<MazeBreakThrough>& break_through, int& money) {
        tower_placed_ = false;
        if (break_through.empty()) return;
        
        auto& mngr = *tower_manager_;
        auto& rs = *routes_;
        auto& ts = mngr.towers();
        
        MazeCoverage cov;
        cov.Init(mngr.board(), mngr, rs);
        cov.Compute();
             
        Placement<double> best;
        best.score = 0;
        
        Index t_ind = max_element(tower_factors_.begin(), tower_factors_.end())  - tower_factors_.begin();
        
        auto bt_cov = cov.NeededCoverage(break_through);
        
        bool is_initialized = false;
        auto op = [&](const Position& p) {
            if (ts[t_ind].cost > money) {
                return;
            }
            double score = 0;
            auto t_cov = cov.CoverageOfTower({t_ind, p});
            for (auto i = 0; i < t_cov.size(); ++i) {
                if (bt_cov[i] != 0) {
                    score += t_cov[i]; 
                }
            }
            
            Placement<double> pl{{t_ind, p}, score, *this};
            if (!is_initialized || pl.IsBetterThan(best)) {
                is_initialized = true;
                best = pl;
            }
        };
        mngr.ForEachOpenPosition(op);
        if (best.score > 0) {
            tower_placed_ = true;
            mngr.PlaceTower(best);
            money -= ts[best.tower].cost;
        }
    }
    
    


    void Revert(int& money) {
        if (!tower_placed_) return;
        tower_placed_ = false;
        auto& mngr = *tower_manager_;
        auto& ts = mngr.towers();
        assert(!mngr.placed_towers().empty());
        money += ts[mngr.placed_towers().back().tower].cost;
        mngr.PopPlacedTower();
    }

    void set_params(Params& params) {
        params_ = params;
        ComputeTowerFactors();
    } 
    
    
private:

    vector<double> ComputeCurrentScores(
        const vector<MazeBreakThrough>& break_through, int& money) {
        
        vector<double> scores;
        
        auto& mngr = *tower_manager_;
        auto& ts = mngr.towers();
        
        CountMap<Position, UnorderedMap> bt_pos;
        for (auto& bt : break_through) {
            for (auto& p : bt.path) {
                if (p == bt.path.back()) continue;
                bt_pos.increase(p);
            }
        }
        
        auto op = [&](const TowerPosition& tp) {
            scores.push_back(0);
            if (ts[tp.tower].cost > money) {
                return;
            }
            Count score = 0;
            for (auto& p : mngr.tower_scope(tp).positions) {
                auto it = bt_pos.find(p);
                if (it == bt_pos.end()) continue;
                score += it->second;
            }
            scores.back() = /*tower_factors_[tp.tower] **/ score;
        };
        mngr.ForEachOpenTowerPosition(op);
        return scores;
    }
    
    vector<Count> ComputeRouteScores(const vector<MazeBreakThrough>& break_through, int& money) {
        vector<Count> scores;
        
        auto& mngr = *tower_manager_;
        auto& ts = mngr.towers();
        auto& rs = *routes_;
        
        auto op = [&](const TowerPosition& tp) {
            scores.push_back(0);
            if (ts[tp.tower].cost > money) {
                return;
            }
            scores.back() = rs.CountRoutes(mngr.tower_scope(tp).positions);
        };
        mngr.ForEachOpenTowerPosition(op);
        return scores;
    }
    
    
    vector<double> ComputeRouteScores_2(const vector<MazeBreakThrough>& break_through, int& money) {
        auto& mngr = *tower_manager_;
        auto& rs = *routes_;
        auto& ts = mngr.towers();
        
        MazeCoverage cov;
        cov.Init(mngr.board(), mngr, rs);
        cov.Compute();
        vector<double> scores;
        
        auto op = [&](const TowerPosition& tp) {
            scores.push_back(0);
            if (ts[tp.tower].cost > money) {
                return;
            }
            auto v = cov.Coverage(tp);
            if (v.empty()) return;
            scores.back() = *min_element(v.begin(), v.end()) - *max_element(v.begin(), v.end());
//            
//            double ss = std::numeric_limits<double>::max();
//            for (auto s : v) {
//                if (s < ss && s != 0) {
//                    ss = s;  
//                } 
//            }
//            if (ss == std::numeric_limits<double>::max()) return;
//            scores.back() = ss; 
        };
        mngr.ForEachOpenTowerPosition(op);
        return scores;
    }



    void ComputeTowerFactors() {
        auto& ts = tower_manager_->towers();
        for (auto i = 0; i < ts.size(); ++i) {
            tower_factors_[i] = pow(ts[i].dmg, params_.dmg_pow) / pow(ts[i].cost, params_.cost_pow);
        }
    }
};