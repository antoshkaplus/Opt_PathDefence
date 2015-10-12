//
//  strategy.hpp
//  PathDefence
//
//  Created by Anton Logunov on 10/11/15.
//
//

#pragma once

class Strategy {
public:    
    virtual int init(Board& board, 
                     const vector<Tower>& towers,
                     int money, 
                     int creep_health, 
                     int creep_money) = 0;
                 
    // returns indices of towers that were placed
    virtual vector<TowerPosition> placeTowers(const vector<Creep>& creeps, 
                                              int money, 
                                              vector<Count>& base_health) = 0;
};
