

#include <iostream>


#include "ant/core/core.hpp"



using namespace std;



class PathDefense {
public:

    int init(vector<string> board, 
             int money, 
             int creep_health, 
             int creep_money, 
             vector<int> tower_type) {
        return 0;
    }


    // return: col, row, type
    vector<int> placeTowers(vector<int>& creep, 
                            int money, 
                            vector<int>& base_health) {
        return vector<int>();
    }

};


int main(int argc, const char* argv[])
{
    auto read_int = [] (int& i) {
        cin >> i;
    };
    auto read_string = [] (string& s) {
        cin >> s;
    };
    int N, M;
    cin >> N >> M;
    vector<string> b(N);
    for_each(b.begin(), b.end(), read_string);
    int h, m;
    cin >> h >> m;
    int nt;
    cin >> nt;
    vector<int> nts(nt);
    for_each(nts.begin(), nts.end(), read_int);
    PathDefense defense;
    defense.init(b, M, h, m, nts);
    int nc; 
    for (int i = 0; i < 2000; ++i) {
        cin >> m >> nc;
        vector<int> creep(nc);
        for_each(creep.begin(), creep.end(), read_int);
        int B;
        cin >> B;
        vector<int> bh(B);
        for_each(bh.begin(), bh.end(), read_int);
        auto ret = defense.placeTowers(creep, m, bh);
        cout << ret.size() << endl;
        for (auto r : ret) {
            cout << r << endl;
        }
        cout.flush();
    }
}
