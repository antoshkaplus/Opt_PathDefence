

class PathDefense_2 {
public:

    // spawn position can be convrted to integer as everything else
    // current position, position of spawn, 
    // map should be a pointer??? 
    // try to use grid
    // need grid to do things faster
    // use hash from spawn location
    Grid<Position, map<uint64_t, Position>>



    Init() {
        // find all spawn locations
        
        // from each location go breadth first search 
        // find base and while go back assign grid
    }


    Simulate() {
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

    FindTowerPlacements() {
        
    
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