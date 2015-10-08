//
//  place_tower.hpp
//  PathDefence
//
//  Created by Anton Logunov on 6/5/15.
//
//

#pragma once


#include "util.hpp"
#include "tower_manager.hpp"

// this class is used to determine where to place 
// new tower
class TowerPlacer {

    struct Item : TowerPosition {
        // want it bigger
        
        // also we need put those in the groups probably
        // by total coverage, so that we didn't run out of good placements
        // 
        
        
        // min from total covarage. this is global dmg per route // not zero
        // need some generalization
        double min_total_coverage;
        // persantage value
        double miss_hp_coverage;
        
        // by this particular element 
        double coverage;
        
        // count * dmg
        
        // how many points 
        
        
        Count count;
    };
    // 
    double Score(const Item& t) {
        auto& ts = tower_manager_->towers();
        return 1.*t.min_total_coverage + 20.*t.miss_hp_coverage + 10*t.count*ts[t.tower].dmg;
    }

    pair<Item, bool> ChooseItem(vector<Item>& items, int money) {
        auto& ts = tower_manager_->towers();
        Count N = items.size();
        vector<double> values(N);
        vector<Index> inds(N);
        for (Index i = 0; i < N; ++i) {
            auto& t = items[i];
            double d = Score(t);
            values[i] = d;
            inds[i] = i;
        }
        sort(inds.begin(), inds.end(), [&](Index i_0, Index i_1) {
            return values[i_0] > values[i_1];
        });
        vector<Item> items_2(N);
        for (Index i = 0; i < N; ++i) {
            items_2[i] = items[inds[i]];
        }   
        for (auto& i : items_2) {
            if (money >= ts[i.tower].cost) {
                return {i, true}; 
            }
        }
        return {Item(), false};
    }

    double MinTotalCoverage(const vector<double>& c) {
        double min = numeric_limits<double>::max();
        for (auto m : c) {
            if (m != 0 && m < min) {
                min = m;
            }
        }
        return min;
    }


public:
    TowerPlacer(TowerManager& manager, const Next& next) {
        Init(manager, next);
    }
    
    void Init(TowerManager& manager, const Next& next) {
        tower_manager_ = &manager; 
        current_coverage.resize(manager.board().spawn_loc_count());
        fill(current_coverage.begin(), current_coverage.end(), 0);
    }
    
    
    // coverage starts with 0-s and it's equal to number of routes
    // break through : spawn
    void Place(vector<Count>& route_miss_hp, 
               const vector<BreakThrough>& break_through,
               Count& money) {
        
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
        auto& ts = tower_manager_->towers();
        
        auto func = [&](const Item& i_0, const Item& i_1) {
            double d_0 = Score(i_0);
            double d_1 = Score(i_1);
            return d_0 > d_1;
            //return i_0.miss_hp_coverage < i_1.miss_hp_coverage; 
        };
        set<Item, decltype(func)> best_items(func);
        const int BEST_MAX_COUNT = 10;
        for (const Position& p : tower_manager_->open_tower_positions()) {
            for (Index i = 0; i < ts.size(); ++i) {
                Item item;
                item.tower = i;
                item.position = p;
                auto coverage = ComputeCoverage(*tower_manager_, item);
                item.count = int(accumulate(coverage.begin(), coverage.end(), 0));
                transform(coverage.begin(), coverage.end(), coverage.begin(), [&](double d) {
                    if (d > 1) {
                        d = (d-1)/2 + 1;
                    }
                    return d*ts[i].dmg;
                });
                if (item.count == 0) {
                    continue;
                }
                bool stupid = true;
                // set another coverage to see how good it is against all those current creeps
                item.miss_hp_coverage = 0;
                for (auto i = 0; i < coverage.size(); ++i) {
                    if (route_miss_hp[i] > 0 && coverage[i] > 0.5) {
                        item.miss_hp_coverage += double(1.) * std::min<Count>(route_miss_hp[i], coverage[i]) / route_miss_hp[i];
                        stupid = false;
                        //break;
                    }
                    //coverage[i] /= count;
                }
                item.miss_hp_coverage = int(item.miss_hp_coverage);
                if (stupid) continue;
                stupid = true;
                for (auto& b : break_through) {
                    auto bb = next_->base_loc_for_spawn(b.spawn_loc);
                    if (coverage[b.spawn_loc] > 0 && can_wound(b.cur_loc, p, bb)) {
                        stupid = false;
                        break;
                    }
                }
                if (stupid) continue;
                transform(coverage.begin(), coverage.end(), 
                          current_coverage.begin(), 
                          buf_coverage.begin(), sum);
                item.min_total_coverage = MinTotalCoverage(buf_coverage);
                
                if (best_items.size() < BEST_MAX_COUNT) {
                    best_items.insert(item);
                }else if (best_items.begin()->miss_hp_coverage < item.miss_hp_coverage) {
                    best_items.erase(best_items.begin());
                    best_items.insert(item);
                }
            }
        }
        vector<Item> v{best_items.begin(), best_items.end()};
        pair<Item, bool> p = ChooseItem(v, money);
        if (p.first.position.col == 11 && p.first.position.row == 10) {
            p.first.position.col++;
            p.first.position.col--;
        }
        if (p.second) {
            tower_manager_->PlaceTower(p.first);
            money -= tower_manager_->towers()[p.first.tower].cost;
            auto coverage = ComputeCoverage(*tower_manager_, p.first);
            transform(coverage.begin(), coverage.end(), 
                      current_coverage.begin(), 
                      current_coverage.begin(), sum);
            
        }
        
    }
    
    
    
    TowerManager* tower_manager_;
    const Next* next_; 
    vector<double> current_coverage; 
    
};

