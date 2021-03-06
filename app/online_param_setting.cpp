
#include "path_defence.hpp"
#include "collector.hpp"
#include "maze_simulator.hpp"


// expecting to be run from bin
int main(int argc, const char* argv[])
{
    // this file contains everything we receive from the game
    ofstream out("../output/input.txt");
    // this file contains different stats we want to get from the solver
    ofstream res("../output/res.txt");
    // this file contains everything we want to log:
    // debugging information, errors and so forth
    ofstream logger("../output/log.txt");
    
    try {
        auto read_int = [&] (int& i) {
            cin >> i;
            out << i << endl;
        };
        auto read_string = [&] (string& s) {
            cin >> s;
            out << s << endl;
        };
        int N, M;
        
        cin >> N >> M;
        out << N << " " << M << endl;
        
        vector<string> b(N);
        for_each(b.begin(), b.end(), read_string);
        int h, m;
        
        cin >> h >> m;
        out << h << " " << m << endl;
        
        int nt;
        
        cin >> nt;
        out << nt << endl;
        
        vector<int> nts(nt);
        for_each(nts.begin(), nts.end(), read_int);
        PathDefense defense;
        defense.init(b, M, h, m, nts);
        int nc; 
        for (int i = 0; i < 2000; ++i) {
            cin >> m >> nc;
            out << m << " " << nc << endl;
            
            vector<int> creep(nc);
            for_each(creep.begin(), creep.end(), read_int);
            int B;
            cin >> B;
            out << B << endl;
            
            vector<int> bh(B);
            for_each(bh.begin(), bh.end(), read_int);
            vector<int> ret;
            ret = defense.placeTowers(creep, m, bh);
            logger << i << endl;
            cout << ret.size() << endl;
            res << ret.size() << endl;
            for (auto r : ret) {
                cout << r << endl;
                res << ret.size() << " ";
            }
            res << endl;
            res.flush();
            cout.flush();
        }
    } catch (exception& exp) {
        logger << exp.what();
        logger.flush();
    } catch (string exp) {
        logger << exp;
        logger.flush();
    } catch (...) {
        logger << "some sort of exception";
        logger.flush();
    }
}
