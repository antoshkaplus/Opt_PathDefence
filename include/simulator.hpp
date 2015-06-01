//
//  simulator.hpp
//  PathDefence
//
//  Created by Anton Logunov on 5/30/15.
//
//

#ifndef PathDefence_simulator_hpp
#define PathDefence_simulator_hpp

#include "ant/graph/graph.hpp"

using namespace ant::graph;

class Board {
    
    // can have actual pointers instead of Positon
    // against memory segmentation
    // Grid<map<Index, Index>> some;
public:    
    // locs, paths, where to go next 
    vector<map<Index, Index>> some;


    Board() {}
    
    Board(vector<string>& b) {
        Count N = b.size();
        Grid<Index> gg;
        // could use iterator and std method
        gg.fill(-1);
        Count locs = 0;
        for (int r = 0; r < N; ++r) {
            for (int c = 0; c < N; ++c) {
                if (b[r][c] == CELL_ROAD) {
                    gg(r, c) = locs++;
                }
            }
        }
        NodeAdjacencyList adj(locs); 
        for (int r = 0; r < N; ++r) {
            for (int c = 0; c < N; ++c) {
                if (b[r][c] == CELL_ROAD) {
                    if (r > 0) {
                        if (b[r-1][c] == CELL_ROAD || b[r-1][c] == CELL_BASE) {
                            adj[gg(r, c)].push_back(gg(r-1, c));
                        }
                    }
                    if (c > 0) {
                        // same here
                    }
                    if (r != N-1) {
                    
                    }
                    if (c != N-1) {
                    
                    }
                }
            }
        }
        // run for each spawn location
        Index v;
        // assign minimum that is around? ? ? 
        vector<Count> time(locs);
        for (int i = 0; i < spawn_loc_count(); ++i) {
            fill(time.begin(), time.end(), 0);
            Index b;
            auto proc = [&](Index v, Index prev) {
                time[v] = time[prev]+1;
                if (IsBaseNearby(v)) {
                    b = v;
                    return BFS_Flow::Terminate;
                }
                return BFS_Flow::Continue; 
            };
            BFS_Prev(CreateGraph(adj), v, proc);
            auto SmallestNeighbor = [] (Index i) {
                return 0;
            };
            
            // now from base we go and initialize
            // -1 means base 
            some[b][i] = -1;
            while (true) {
                Index n = SmallestNeighbor(b);
                some[n][i] = b;
                if (n == v) break;
                b = n;
            }
        }
        // now what happens if we there is no such path
    
    }
    
    
    
    
    bool IsBase(Index t) { 
        return true;
    }
    
    bool IsBaseNearby(Index t) {
        return true;
    }
    
    Count spawn_loc_count() const {
    
    }
    
    Index spawn_loc(const Position& pos) const {
    
    }
    
    void ComputeInitialRoutes() {
        
    
    
    }
    
};



class Simulator {
public:
    
    Board board;
    vector<Tower> towers;
    vector<TowerPosition> tower_positions;
    // each tower, path points sorted by range  
    vector<vector<Index>> tower_scope;
    vector<vector<Index>> tower_scope_bounds;
    
    map<Position, Index> dd;
    
    Simulator(const Board& board, vector<Tower>& towers, vector<TowerPosition>& tower_positions) {
    
    }
    
    
    // spawn position can be convrted to integer as everything else
    // current position, position of spawn, 
    // map should be a pointer??? 
    // try to use grid
    // need grid to do things faster
    // use hash from spawn location
    
    // Index - spawn location
    // Position - next position for this spawn loc
    
    
    // later
    
    
    void Init() {
        // find all spawn locations
        
        // from each location go breadth first search 
        // find base and while go back assign grid
    }
    
    void Train(vector<Creep>& creep) {}
    
    
    // for each creep we assign spawn location
    vector<Count> Simulate(vector<Creep>& creep, vector<Index> spawn_loc) {
        vector<Index> pos(creep.size());
        // get positions and convert to shit
        Count creeps_alive = creep.size();
        
        // should output how many get through out of which route
        
        vector<Count> miss_hp_per_route(board.spawn_loc_count(), 0);
        
        unordered_multiset<Index> creep_pos;
        
        
        Count N = creep.size();
        
        // can create vector with indices actually
        while (creeps_alive > 0) {
            // creeps moves and attacks
            for (Index i = 0; i < N; ++i) {
                if (board.IsBaseNearby(pos[i])) {
                    miss_hp_per_route[spawn_loc[i]] = creep[i].hp;
                    creep[i].hp = 0;
                    --creeps_alive; 
                } else {
                    if (board.some[pos[i]].find(spawn_loc[i]) == board.some[pos[i]].end()) throw runtime_error("lol");
                    // what if new location for us... thats why we run train before this one
                    pos[i] = board.some[pos[i]][spawn_loc[i]];
                }
            } 
            // now we do shooting
            for (Index i = 0; i < tower_positions.size(); ++i) {
                Tower& t = towers[tower_positions[i].tower];
                // could make use of some big chunk of memory instead of those small arrays
                
                
                // want to have continious memory ?? 
                Index first = 0;
                Index creep_to_kill = -1;
                for (Index last : tower_scope_bounds[i]) {
                    for (Index j = first; j < last; ++j) {
                        auto r = creep_pos.equal_range(tower_scope[i][j]);
                        for (auto it = r.first; it != r.second; ++it) {
                            Index x = *it;
                            if (creep[x].hp > 0 && (creep_to_kill == -1 || creep[creep_to_kill].id > creep[x].id)) {
                                creep_to_kill = x;
                            } 
                        }
                    }
                    if (creep_to_kill != -1) break;
                    first = last;
                }
                if (creep_to_kill != -1) {
                    creep[creep_to_kill].hp -= t.dmg;
                    if (creep[creep_to_kill].hp <= 0) {
                        --creeps_alive;
                    }
                }
            }
            
            // towers made their shit
        }   
    
       
        // now we can compute stuff that left alive
        return miss_hp_per_route;
    
    
        // each tower shoots to nearest location (they are sorted) that contain creep inside, if
        // just use binary search??? // can use loop actually // and vector in each location
        // creep has hp > 0
        
        // remove all dead creeps 
        // while at least one creep alive creep 
        // for each creep we simulate where he goes
        
        // if next position is base we define that creep got to base with certain hp
        
        // we also know each position he went through
        // return vector of creeps with hp who got through
    }
    
    void FindTowerPlacements(vector<Count>& route_miss_hp) {
        // also base
        
        // coverage is vector of doubles
        
        // want to normilize between tours // so each tour would have certain coverage
        
        // we can sort all possible locations ??? 
        // coverage is calculated by considering all placed towers:
        // how many tours inside, how many points for each tour: sum points and get persantage for everyone... and after that multiply persantage by points number
        // put this number inside coverage for every tour
        
        // then we just use some metric to make thing big and equal:
        // multiply everyone?
        
        // counting how many positions bad against vawas... so can reduce points by two, 
        // but 1 is minimum
        
        // or just has his path in range
        // also we consider if tower would kill unwanted creep(s) 
        // or at least takes maximum of his health ???
    }
    
    
    
    
    
    
    
    
};

#endif
