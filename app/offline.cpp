
#include "path_defence_2.hpp"
#include "path_finder.hpp"
#include "tower_manager.hpp"
#include "next.hpp"

// expecting to be run from bin
int main(int argc, const char* argv[])
{
    // need to set logger inside class or create some special class probably
    ofstream out("../output/out.txt");
    ifstream cin("../output/input.txt");
    ofstream res("../output/res.txt");
    // can't track because online closes connection after computation is over
    ofstream spawn_ticks("../output/spawn_ticks.txt");
    ofstream tick_creep_count("../output/tick_creep_count.txt");
    ofstream ave_creep_count("../output/ave_creep_count.txt");
    ofstream wave_score("../output/wave_score.txt");
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
        PathDefense_2 defense;
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
            //cout << "before" <<endl;
            ret = defense.placeTowers(creep, m, bh);
            //cout << "after" << endl;
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
//        auto& s = defense.creeps_spawn_tick();
//        for (Index t : s) {
//            if (t == -1) break;
//            spawn_ticks << t << endl;
//        }
//        auto& s_2 = defense.tick_creep_count();
//        for (Index t : s_2) {
//            tick_creep_count << t << endl;
//        } 
//        auto& s_3 = defense.average_creep_count_increased();
//        for (bool b : s_3) {
//            ave_creep_count << b << endl;
//        }
//        auto s_4 = defense.wave_score();
//        for (double b : s_4) {
//            wave_score << b << endl;
//        } 
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
    return 0;
}
