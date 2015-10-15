
#include "gtest/gtest.h"

#include "maze.hpp"


using Picture = vector<string>;

TEST(Maze, MazeDeduction) {
    Picture p_0 = {
    "####",
    "..0#",
    "##.#",
    "##.#"
    };
    Picture p_1 = {
    "#####",
    "...0#",
    "##.##",
    "##.##",
    "##.##"
    };
    Picture p_2 = {
    "######",
    "...###",
    "##..0#",
    "...###",
    "######"
    };
    Picture p_3 = {
    "##.###",
    "....0#",
    "##.###",
    "##.###",
    "...###",
    ".#.###"
    };
    Picture p_4 = {
    "######",
    "#.##0#",
    "#.##.#",
    "......",
    "####.#"
    };
    vector<Picture> ps = {p_0, p_1, p_2, p_3, p_4};
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
                for (auto d = 0; d < n; ++d) {
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

TEST(Maze, MazeDeduction_2) {
    vector<string> pic = {
    "#####",
    "...0#",
    "##.##",
    "##.##"
    };
}

TEST(Maze, MazeDeduction_3) {

}



int main(int argc, char **argv) {
    return RUN_ALL_TESTS();
}

