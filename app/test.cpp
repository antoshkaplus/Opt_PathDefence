
#include "gtest/gtest.h"

#include "maze.hpp"
#include "maze_simulator.hpp"
#include "tower_manager.hpp"

using Picture = vector<string>;

Picture p_0 = {
    "####",
    "..0#",
    "##.#",
    "##.#"
};
/*
 1,0 => 1,1
 1,1 => 1,2
 2,2 => 1,2
 3,2 => 2,2
 */

Picture p_1 = {
    "#####",
    "...0#",
    "##.##",
    "##.##",
    "##.##"
};
/*
 1,0 => 1,1
 1,1 => 1,2
 1,2 => 1,3
 2,2 => 1,2
 3,2 => 2,2
 4,2 => 3,2
 */

Picture p_2 = {
    "######",
    "...###",
    "##..0#",
    "...###",
    "######"
};
/*
 1,0 => 1,1
 1,1 => 1,2
 1,2 => 2,2
 2,2 => 2,3
 2,3 => 2,4
 3,0 => 3,1
 3,1 => 3,2
 3,2 => 2,2
 */

Picture p_3 = {
    "##.###",
    "....0#",
    "##.###",
    "##.###",
    "...###",
    ".#.###"
};
/*
 0,2 => 1,2
 1,0 => 1,1
 1,1 => 1,2
 1,2 => 1,3
 1,3 => 1,4
 2,2 => 1,2
 3,2 => 2,2
 4,0 => 4,1
 4,1 => 4,2
 4,2 => 3,2
 5,0 => 4,0
 5,2 => 4,2
 */

Picture p_4 = {
    "######",
    "#.##0#",
    "#.##.#",
    "......",
    "####.#",
    "####.#"
};
/*
 2,4 => 1,4
 3,0 => 3,1
 3,4 => 2,4
 3,5 => 3,4
 4,4 => 3,4
 5,4 => 4,4
 */

Picture p_5 = {
    "....###",
    "###.###",
    "..#.###",
    "#.#.###",
    "..#.###",
    "#...###",
    "###0###"
};

TEST(Maze, MazeDeduction) {
    vector<Picture> ps = {p_0, p_1, p_2, p_3, p_4, p_5};
    for (auto& p : ps) {
        cout << "picture:" << endl;
        Board b{p};
        Maze m;
        m.Deduct(b);
        Count n = p.size();
        for (auto r = 0; r < n; ++r) {
            for (auto c = 0; c < n; ++c) {
                // output every position next in any direction
                auto poss = m.Next({r, c});
                for (auto d = 0; d < kDirCount; ++d) {
                    if (poss[d]) {
                        auto p = Position{r,c} + kDirVector[d];
                        cout << r << "," << c << " => " << p.row << "," << p.col << endl;
                    }
                }
            }
        }    
        cout << endl << endl;
    }
}

TEST(Simulator, Simulation) {
    vector<Picture> ps = {p_0, p_1, p_2, p_3, p_4};
    
    for (auto& p : ps) {
        if (p == ps.back()) continue;
        Board b{p};
        Maze m;
        m.Deduct(b);
        TowerManager tm;
        tm.Init(b, vector<Tower>());
        MazeSimulator simulator;
        simulator.Init(m, tm);
        for (auto c : b.spawn_locs()) {
            Creep cc{0, 0, c, 10};
            // should compute prev
            Position prev;
            for (auto d = 0; d < kDirCount; ++d) {
                prev = c + kDirVector[d];
                if (!b.IsInside(prev)) {
                    break;   
                }
            }
            simulator.Simulate({cc}, {prev});
            ASSERT_FALSE(simulator.break_through().empty());
        }
    }
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

