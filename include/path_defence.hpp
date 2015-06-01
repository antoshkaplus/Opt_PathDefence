//
//  path_defence.hpp
//  PathDefence
//
//  Created by Anton Logunov on 5/28/15.
//
//

#ifndef PathDefence_path_defence_hpp
#define PathDefence_path_defence_hpp

#include "util.hpp"
#include "simulator.hpp"

struct Score {
    double count;
    double health;
    
    Score() {
        count = 0;
        health = 0;
    }
};

class PathDefense {
private:
   
        Count N;
    vector<Position> base_positions_;
    vector<vector<Position>> creep_positions_;
    vector<bool> creep_discarded_;
    
    // they are doubles because we going to decrease every tick 
    // by some small value (after every tower placement ?? )
    Grid<double> total_count_;
    Grid<double> total_health_;
    vector<Tower> towers_;
    Grid<char> board_;
    Count creeps_got_through_ = 0;
    double regression_factor_ = 0.01;
    Count creep_health_;
    
    
    Grid<bitset<MAX_BASE_COUNT>> path_to_base_exists;
    
    // for each path position we store creeps who got into 
    // the castle
    unordered_map<uint64_t, vector<Index>> heroes_path;
    
    Count tower_count_ = 0;
    
    
    // init with -1 as not present
    array<Index, MAX_CREEP_COUNT> creeps_spawn_tick_;
    vector<Creep> current_creeps_;
    set<Index> previous_creeps_;
    
    array<Count, TICK_COUNT> tick_creep_count_;
    
    constexpr static Count CREEP_COUNT_BEFORE_CAPACITY = 10;
    queue<Count> creep_count_before_;
    Count creep_total_count_before_ = 0;
    
    Index current_tick_ = 0;
    double need_towers_ = 0;
    
    array<bool, TICK_COUNT> average_creep_count_increased_; 
    Count current_money_;
    
    
    void UpdateScore(Creep& c) {
        for (auto& p : creep_positions_[c.id]) {
            heroes_path[Hash(p.row, p.col)].push_back(c.id);
            if (creep_discarded_[c.id]) continue;
            total_count_(p) += 1;
            total_health_(p) += c.hp;
        }
    }   
    
    // depends on number of bases.. but can use array of 8
    vector<Count> ComputeBaseCoverage(Index t, const Position& p) {
        vector<Count> res(base_positions_.size(), 0);
        Count rng = towers_[t].rng;
        for (auto r = max(p.row - rng, 0); 
             r < min(p.row + rng, N); ++r) {
            for (auto c = max(p.col - rng, 0); 
                 c < min(p.col + rng, N); ++c) {
                if ((r - p.row)*(r - p.row) + (c - p.col)*(c - p.col) <= rng*rng
                    && board_(r, c) == '.') {
                    
                    for (int i = 0; i < base_positions_.size(); ++i) {
                        if (path_to_base_exists(r, c).test(i)) {
                            ++res[i];
                        }
                    }
                }
            }
        }
        return res;
    }
    
    Count CountEmptyCells(Index t, const Position& p) {
        Count count = 0;
        Count rng = towers_[t].rng;
        for (auto r = max(p.row - rng, 0); 
             r < min(p.row + rng, N); ++r) {
            for (auto c = max(p.col - rng, 0); 
                 c < min(p.col + rng, N); ++c) {
                if ((r - p.row)*(r - p.row) + (c - p.col)*(c - p.col) <= rng*rng
                    && board_(r, c) == '.') {
                    ++count;
                }
            }
        }
        return count;
    }
    
    Score TowerScore(const Tower& t, const Position& p) {
        Score s;
        set<Index> creep_ids;
        for (auto r = max(p.row - t.rng, 0); 
             r < min(p.row + t.rng, N); ++r) {
            for (auto c = max(p.col - t.rng, 0); 
                 c < min(p.col + t.rng, N); ++c) {
                if ((r - p.row)*(r - p.row) + (c - p.col)*(c - p.col) <= t.rng*t.rng) {
                    auto& f = heroes_path[Hash(r, c)];
                    for (Index i : f) {
                        creep_ids.insert(i);
                    }
                    // take everything from this position
                    s.count += total_count_(r, c);
                    s.health += total_health_(r, c);
                }
            }
        }
        s.count = creep_ids.size();
        return s;
    }
    
    bool IsBaseNearby(const Position& p) {
        for (auto b : base_positions_) {
            for (auto s : {Indent{0, -1}, Indent{-1, 0}, 
                Indent{1, 0}, Indent{0, 1}}) {
                    Position q = b + s;
                    if (p == q) return true;
                }
        }
        return false;
    }
    
    Index BaseNearby(const Position& p) {
        for (auto b_i = 0; b_i < base_positions_.size(); ++b_i) {
            for (auto s : {Indent{0, -1}, Indent{-1, 0}, 
                Indent{1, 0}, Indent{0, 1}}) {
                    Position q = base_positions_[b_i] + s;
                    if (p == q) return b_i;
                }
        }
        return -1;
    }
    
    void AddTower(vector<int>& ts, int col, int row, int type) {
        ts.push_back(col);
        ts.push_back(row);
        ts.push_back(type);
    }
    
    void UpdateCreeps(const vector<int>& creeps_int) {
        current_creeps_.resize(creeps_int.size()/4);
        auto& cc = creeps_int;
        for (int i = 0; i < current_creeps_.size(); ++i) {
            // id, hp, col, row
            // can store just pointers inside Creep structure
            // and take everything by index
            Index id = cc[4*i];
            current_creeps_[i] = {id, cc[4*i+1], cc[4*i+3], cc[4*i+2]};
            if (creeps_spawn_tick_[id] == -1) {
                creeps_spawn_tick_[id] = current_tick_;
            }
        }
    }
    
    vector<int> ConvertTowerPositions(vector<TowerPosition>& ts) {
        vector<int> r;
        for (auto& t : ts) {
            AddTower(r, t.position.col, t.position.row, t.tower);
        }
        return r;
    };
    
    
    void UpdateAverageCreepCount() {
        Count count = current_creeps_.size();
        if (creep_count_before_.size() < CREEP_COUNT_BEFORE_CAPACITY) {
            creep_total_count_before_ += count;
            creep_count_before_.push(count);
        } else {
            creep_total_count_before_ -= creep_count_before_.front();
            creep_count_before_.pop();
            creep_count_before_.push(count);
            creep_total_count_before_ += count;
        }
    }
    
    double average_creep_count() const {
        return 1. * creep_total_count_before_ / CREEP_COUNT_BEFORE_CAPACITY;
    }
    
    double current_wave_score() const {
        Index i = current_tick_;
        Count inc = 0;
        while (i >= 0 && average_creep_count_increased_[i]) {
            inc += 1;
            --i;
        }
        return inc * current_creeps_.size();
    }
    
    pair<TowerPosition, bool> PlaceTower() {
        int t = 0;
       
        Position best_pos;
        Score best_score;
        for (auto r = 0; r < N; ++r) {
            for (auto c = 0; c < N; ++c) {
                if (board_(r, c) == '#') {
                    for (auto i = 0; i < towers_.size(); ++i) {
                        Position p{r, c};
                        Score s = TowerScore(towers_[i], p);
                        if (s.count > best_score.count || 
                            (s.count == best_score.count && 
                             towers_[t].cost > towers_[i].cost) ||
                                (s.count == best_score.count && 
                                 towers_[t].cost == towers_[i].cost &&
                                 CountEmptyCells(t, best_pos) < CountEmptyCells(i, p))) {
                                t = i;
                                best_pos = {r, c};
                                best_score = s;
                            } 
                    }   
                }
            }
        }
        TowerPosition tp{t, best_pos};
        if (current_money_ >= towers_[t].cost) {
            board_(best_pos) = '*';
            current_money_ -= towers_[t].cost;
            tower_count_ += 1;
            return {tp, true};
        }
        return {tp, false};
    }
    
    
    
public:
    
    PathDefense() {
        fill(creeps_spawn_tick_.begin(), creeps_spawn_tick_.end(), -1);
    }

    int init(vector<string> board, 
             int money, 
             int creep_health, 
             int creep_money, 
             vector<int> tower_type) {
        
        
        creep_health_ = creep_health;
        creep_positions_.resize(2000);
        creep_discarded_.resize(2000, false);
        N = board.size();
        path_to_base_exists.resize(N, N);
        fill(path_to_base_exists.begin(), path_to_base_exists.end(), 0);
        total_count_.resize(N, N);
        fill(total_count_.begin(), total_count_.end(), 0);
        total_health_.resize(N, N); 
        fill(total_health_.begin(), total_health_.end(), 0);
        board_.resize(N, N);
        for (int r = 0; r < N; ++r) {
            for (int c = 0; c < N; ++c) {
                board_(r, c) = board[r][c];
                if (board[r][c] != '.' && board[r][c] != '#') {
                    base_positions_.push_back({r, c});
                } 
            }
        }
        for (int i = 0; i < tower_type.size()/3; ++i) {
            towers_.emplace_back(tower_type[3*i], 
                                 tower_type[3*i+1], 
                                 tower_type[3*i+2]);   
        }
        return 0;        
    }

    void UpdateCreepPositions() {
        set<Index> cc;
        for (auto c : current_creeps_) {
            creep_positions_[c.id].emplace_back(c.r, c.c);
            cc.insert(c.id);
        }
        for (auto prev : previous_creeps_) {
            if (cc.count(prev) == 0) {
                // dead
                Index i = BaseNearby(creep_positions_[prev].back());
                if (i == -1) continue;
                for (Position& p : creep_positions_[prev]) {
                    path_to_base_exists[p].set(i);
                }
            }
        }
    }

    // return: col, row, type
    vector<int> placeTowers(const vector<int>& creeps_int, 
                            int money, 
                            vector<int>& base_health) {
        current_money_ = money;
        vector<TowerPosition> res;
        UpdateCreeps(creeps_int);
        tick_creep_count_[current_tick_] = current_creeps_.size();
        double prev = average_creep_count();
        UpdateAverageCreepCount();
        average_creep_count_increased_[current_tick_] = (prev < average_creep_count());
        UpdateCreepPositions();
        
        // not +1 or 1999 score decrease
        if (current_tick_ % HP_INCREASE_PERIOD == 0) {
            Count count = tower_count_;
            for (int i = 0; i < count/2; ++i) {
                auto pt = PlaceTower();
                if (pt.second) res.push_back(pt.first);
            }
        }
        
        if (current_wave_score() >= 200) {
            for (auto& c : current_creeps_) {
                creep_discarded_[c.id] = true;
            }
        } else {
            
            if (current_tick_ < 3000) {
                for (auto& t : total_count_) {
                    t *= 1-regression_factor_;   
                }
                for (auto& t : total_health_) {
                    t *= 1-regression_factor_;
                }
                for (auto c : current_creeps_) {
                    creep_positions_[c.id].emplace_back(c.r, c.c);
                    if (!creep_discarded_[c.id] && IsBaseNearby(Position{c.r, c.c})) {
                        // need to increase scores
                        // ave shots 4. ave dmg 3
                        need_towers_ += c.hp / (4 * 3.);
                        UpdateScore(c);
                        ++creeps_got_through_; 
                    }
                }
                //if (need_towers_ > 1) { 
                if (creeps_got_through_ >= 1) {
                    auto pt = PlaceTower();
                    if (pt.second) res.push_back(pt.first);
                    --creeps_got_through_;
                }
            }
            
            
        }
        previous_creeps_.clear();
        for (auto& c : current_creeps_) {
            previous_creeps_.insert(c.id);
        }
        
        ++current_tick_;
        return ConvertTowerPositions(res);
    }
    
    const decltype(creeps_spawn_tick_)& creeps_spawn_tick() const {
        return creeps_spawn_tick_;
    }
    
    const decltype(tick_creep_count_)& tick_creep_count() const {
        return tick_creep_count_;
    }
    
    const decltype(average_creep_count_increased_)& average_creep_count_increased() const {
        return average_creep_count_increased_;
    }
    
    array<double, TICK_COUNT> wave_score() const {
        array<double, TICK_COUNT> ws;
        int inc = 0;
        for (Index i = 0; i < TICK_COUNT; ++i) {
            if (average_creep_count_increased_[i]) inc += 1;
            else inc = 0;
            ws[i] = inc * tick_creep_count_[i];
        }
        return ws;
    }
    
};


#endif
