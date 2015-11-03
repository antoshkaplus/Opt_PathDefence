#define SUBMIT
#define SOLUTION 1

#include<iostream>
#include<deque>
#include<queue>
#include<cmath>
#include<sstream>
#include<algorithm>
#include<string>
#include<vector>
#include<cstdio>
#include<cstring>
#include<map>
#include<set>
#include<cstdlib>
using namespace std;
#define F0(i,n) for (int i=0; i<n; i++)
#define F1(i,n) for (int i=1; i<=n; i++)
#define CL(a,x) memset(x, a, sizeof(x));
#define SZ(x) ((int)x.size())
const int inf = 1000000009;
typedef pair<int,int> pii;
typedef pair<pii,int> Move;
typedef long long ll;
const int dx[] = { 0, -1, 0, 1 };
const int dy[] = { 1, 0, -1, 0 };
int bc[16];

#ifdef _MSC_VER
#include<ctime>
inline double GetSeconds() { return clock() * 1.0 / CLOCKS_PER_SEC; }
#else
inline ll GetTSC() {
    ll lo, hi;
    asm volatile ("rdtsc": "=a"(lo), "=d"(hi));
    return lo + (hi << 32);
}
inline double GetSeconds() {
#ifdef SUBMIT
    return GetTSC() / 2.5e9;
#else
    return GetTSC() / 2.3e9;
#endif
}
#endif

const int MAX_RAND = 1 << 30;
struct Rand {
    ll x, y, z, w;
    Rand(ll seed) { reseed(seed); }
    void reseed(ll seed) { x = 0x498b3bc5 ^ seed; y = 0; z = 0; w = 0;  F0(i, 10) mix(); }
    void mix() { ll t = x ^ (x << 11); x = y; y = z; z = w; w = w ^ (w >> 19) ^ t ^ (t >> 8); }
    ll rand() { mix(); return x & (MAX_RAND - 1); }
    int nextInt(int n) { return rand() % n; }
};
Rand my(123);
double time_spent;
double t_o[10];
void Init() {
    F0(i, 3) t_o[i] = 0.0;
    my.reseed(789);
    F1(i, 15) bc[i] = bc[i&(i - 1)] + 1;
}
void Report() { 
    cerr << "Elapsed time: " << time_spent << " sec" << endl;
    F0(i, 10) if (t_o[i] > 0) cerr << "t_o[" << i << "] = " << t_o[i] << endl;
}

int sq(int x) { return x * x; }
int d2(int x, int y) { return x * x + y * y; }

struct Creep {
    int id, x, y, health, spawn_time;
    Creep(int id, int x, int y, int h, int s) : id(id), x(x), y(y), health(h), spawn_time(s) {}
    bool operator<(const Creep& other) const { return spawn_time < other.spawn_time; }
};

struct TowerType {
    int range, damage, cost, dead;
    TowerType() {}
    TowerType(int r, int d, int c) : range(r), damage(d), cost(c), dead(0) {}
};
TowerType tower_types[20];

struct Tower {
    int x, y, type;
    int range() { return tower_types[type].range; }
    int damage() { return tower_types[type].damage; }
    int cost() { return tower_types[type].cost; }
    bool in_range(int ox, int oy) { return d2(x - ox, y - oy) <= sq(range()); };
    Tower(int x, int y, int t) : x(x), y(y), type(t) {}
};

struct PossibleTower {
    int x, y, type, built;
    double goodness, basic_goodness;
    int range() { return tower_types[type].range; }
    int damage() { return tower_types[type].damage; }
    int cost() { return tower_types[type].cost; }
    bool in_range(int ox, int oy) { return d2(x - ox, y - oy) <= sq(range()); };
    bool operator < (const PossibleTower& other) const { return goodness > other.goodness; }
};

const int SIMULATION_TIME = 2000;
//const int MIN_CREEP_COUNT = 500;
const int MAX_CREEP_COUNT = 2000;

const int N = 60;
vector<string> board;
int max_id;
vector<pii> creep_moves[MAX_CREEP_COUNT + 1];
int first_move[MAX_CREEP_COUNT + 1];
vector<Creep> fake_creeps;
vector<pii> fake_creep_moves[MAX_CREEP_COUNT + 1];
int total_damage[N][N];
int baseX[8], baseY[8], baseCnt, spawnX[100], spawnY[100], spawnCnt, base_dmg[N][N];
vector<int> baseHealth;
int from_base[8][4][N][N], from_spawn[100][4][N][N], path_masks[N][N], path_dist[N][N][4], path_dist_sum[N][N], health_sum[N][N];
vector<int> in_tower[N][N];
int n, money, ttn, creepHealth, creepMoney;
vector<Tower> towers;
vector<PossibleTower> possible_towers;
int step;
int health[8];

int random_dir(int x, int y) {
    int z = my.nextInt(path_dist_sum[x][y]) + 1;
    F0(dir, 4) {
        z -= path_dist[x][y][dir];
        if (z <= 0) return dir;
    }
    throw 12;
    return -1;
}

bool inside(int x, int y) { return 0 <= x && x < n && 0 <= y && y < n; }
bool on_border(int x, int y) { return 0 == x || x == n-1 || 0 == y || y == n-1; }
bool on_free(int x, int y) { return inside(x, y) && board[x][y] == '#'; }
bool on_path(int x, int y) { return inside(x, y) && board[x][y] == '.'; }

void fill_from(int d[N][N], int dir0, int bx, int by) {
    F0(i, n) F0(j, n) d[i][j] = 0;
    queue<pii> Q; Q.push(pii(bx, by)); d[bx][by] = 1;
    while (!Q.empty()) {
        pii p = Q.front(); Q.pop();
        F0(k, 2) {
            int dir = (dir0 + k) % 4;
            int x = p.first + dx[dir], y = p.second + dy[dir];
            if (on_path(x, y) && !d[x][y]) {
                d[x][y] = 1;
                Q.push(pii(x, y));
            }
        }
    }
}

int rng[10][1000][2500];
vector<int> in_creeps[N*N];
int killed;

int calc_dmg_single(vector<Creep> creeps, int w, set<int>& damagers, int upper_bound, bool save_rng = false) {
    vector<int> base_health = baseHealth;
    int ret = 0;
    vector<Creep> new_creeps;
    int iter = 0;
    while (!creeps.empty()) {
        iter++;
        if (iter >= 2 * n) break;
        new_creeps.clear();
        for (Creep c : creeps) {
            int k = base_dmg[c.x][c.y];
            if (k != -1) {
                damagers.insert(c.id);
                ret += min(base_health[k], c.health);
                base_health[k] -= min(base_health[k], c.health);
                if (ret >= upper_bound) {
                    return ret;
                }
            }
            else {
                int& dir = rng[w][iter][c.id];
                if (save_rng) {
                    dir = random_dir(c.x, c.y);
                }
                else {
                    if (dir < 0 || dir > 3 || !((1 << dir)&path_masks[c.x][c.y])) {
                        dir = random_dir(c.x, c.y);
                    }
                }
                new_creeps.emplace_back(c.id, c.x + dx[dir], c.y + dy[dir], c.health, c.spawn_time);
            }
        }
        creeps = new_creeps;
        
        F0(i, SZ(towers)) in_creeps[i].clear();
        F0(i, SZ(creeps)) {
            Creep& c = creeps[i];
            for (int j : in_tower[c.x][c.y]) in_creeps[j].push_back(i);
        }
        
        F0(ti,SZ(towers)) {
            Tower& t = towers[ti];
            int bestd = inf, bi = -1, bid = -1;
            for (int i : in_creeps[ti]) {
                Creep& creep = creeps[i];
                if (creep.health > 0 && t.in_range(creep.x, creep.y)) {
                    int d = d2(creep.x - t.x, creep.y - t.y);
                    if (d < bestd || (d == bestd && creep.id < bid)) {
                        bid = creep.id;
                        bestd = d;
                        bi = i;
                    }
                }
            }
            if (bi != -1) {
                creeps[bi].health -= t.damage();
            }
        }
        new_creeps.clear();
        for (Creep c : creeps) if (c.health > 0) new_creeps.push_back(c); else killed++;
        creeps = new_creeps;
    }
    return ret;
}

int calc_dmg(const vector<Creep>& creeps, int samples, set<int>& damagers, int upper_bound, bool save_rng = false) {
    killed = 0;
    damagers.clear();
    int total = 0; F0(w, samples) total += calc_dmg_single(creeps, w, damagers, upper_bound, save_rng); return total;
}

void add_tower(Tower t) {
    board[t.x][t.y] = 'T';
    for (int dx = -t.range(); dx <= t.range(); dx++) {
        for (int dy = -t.range(); dy <= t.range(); dy++) if (d2(dx, dy) <= sq(t.range())) {
            int xx = t.x + dx, yy = t.y + dy;
            if (on_path(xx, yy)) {
                total_damage[xx][yy] += t.damage();
                in_tower[xx][yy].push_back(SZ(towers));
            }
        }
    }
    towers.push_back(t);
}

void remove_tower() {
    Tower t = towers.back();
    board[t.x][t.y] = '#';
    for (int dx = -t.range(); dx <= t.range(); dx++) {
        for (int dy = -t.range(); dy <= t.range(); dy++) if (d2(dx, dy) <= sq(t.range())) {
            int xx = t.x + dx, yy = t.y + dy;
            if (on_path(xx, yy)) {
                total_damage[xx][yy] -= t.damage();
                in_tower[xx][yy].pop_back();
            }
        }
    }
    towers.pop_back();
}

int full_sim(vector<Creep> all, vector<pii>* creep_moves, int step0, int upper_bound, int& fake_money) {
    vector<int> base_health = baseHealth;
    int ret = 0;
    fake_money = 0;
    
    sort(all.begin(), all.end());
    int at = 0;
    
    vector<Creep> creeps;	
    vector<Creep> new_creeps;
    for (int step = step0; step <= SIMULATION_TIME; step++) {
        new_creeps.clear();
        
        // spawns
        while (at < SZ(all) && all[at].spawn_time <= step) {
            new_creeps.push_back(all[at]);
            at++;
        }
        
        // attacks/moves
        for (Creep c : creeps) {
            int k = base_dmg[c.x][c.y];
            if (k != -1) {
                int lost = min(base_health[k], c.health);
                ret += lost;
                base_health[k] -= lost;
                if (ret >= upper_bound) {
                    return ret;
                }
            }
            else {
                int t = c.spawn_time;
                if (step - t >= SZ(creep_moves[c.id])) {
                    cerr << step << " " << c.id << " " << c.spawn_time << " " << c.x << " " << c.y << endl;
                    cerr << "WTF" << endl;
                    while (1);
                }
                else
                    new_creeps.emplace_back(c.id, creep_moves[c.id][step - t].first, creep_moves[c.id][step - t].second, c.health, c.spawn_time);
            }
        }
        creeps = new_creeps;
        
        //if (creeps.empty()) continue;
        
        F0(i, SZ(towers)) in_creeps[i].clear();
        F0(i, SZ(creeps)) {
            Creep& c = creeps[i];
            for (int j : in_tower[c.x][c.y]) {
                in_creeps[j].push_back(i);
            }
        }
        
        F0(ti, SZ(towers)) {
            Tower& t = towers[ti];
            int bestd = inf, bi = -1, bid = -1;
            for (int i : in_creeps[ti]) {
                Creep& creep = creeps[i];
                if (creep.health > 0 && t.in_range(creep.x, creep.y)) {
                    int d = d2(creep.x - t.x, creep.y - t.y);
                    if (d < bestd || (d == bestd && creep.id < bid)) {
                        bid = creep.id;
                        bestd = d;
                        bi = i;
                    }
                }
            }
            if (bi != -1) {
                creeps[bi].health -= t.damage();
                if (creeps[bi].health <= 0) {
                    fake_money += creepMoney;
                }
            }
        }
        new_creeps.clear();
        for (Creep c : creeps) if (c.health > 0) new_creeps.push_back(c);
        creeps = new_creeps;
    }
    F0(i, SZ(base_health)) fake_money += base_health[i];
    return ret;
}

void generate_fake(int step) {
    int ids = max_id;
    if (ids < 400) ids = 1000;
    F0(i, ids + 1) {
        // dead creeps
        fake_creep_moves[i] = creep_moves[i];
        if (fake_creep_moves[i].empty()) {
            int j = my.nextInt(spawnCnt);
            int time = my.nextInt(SIMULATION_TIME - step + 1) + step + 1;
            //while (time < 1500) {
            //	time = my.nextInt(SIMULATION_TIME - step + 1) + step + 1;
            //}
            Creep c(i, spawnX[j], spawnY[j], creepHealth * (1 << ((time - 1) / 500)), time);
            fake_creeps.push_back(c);
            fake_creep_moves[c.id].push_back(pii(c.x, c.y));
        }
    }
    
    for (Creep c : fake_creeps) {
        int time = c.spawn_time > step ? c.spawn_time : step;
        while (base_dmg[c.x][c.y] == -1 && time < SIMULATION_TIME) {
            int dir = random_dir(c.x, c.y);
            c.x += dx[dir]; c.y += dy[dir];
            fake_creep_moves[c.id].push_back(pii(c.x, c.y));
            time++;
        }
    }
}

int samples;

struct PathDefense {
    vector<int> placeTowers(vector<int> creep, int money, vector<int> _baseHealth) {
        //cerr << endl << 4 << endl;
        double time_start = GetSeconds();
        baseHealth = _baseHealth;
        step++;
        
        if (step == 1) {
            samples = 8;
            F0(i, MAX_CREEP_COUNT + 1) creep_moves[i].clear();
            max_id = 0;
        }
        if (time_spent > 1.0) samples = min(samples, 4);
        if (time_spent > 2.0) samples = min(samples, 2);
        if (time_spent > 3.0) samples = min(samples, 1);
        
        vector<int> ret;
        vector<Creep> creeps;
        F0(i, SZ(creep) / 4) {
            int id = creep[4 * i];
            int cx = creep[4 * i + 3];
            int cy = creep[4 * i + 2];
            int hp = creep[4 * i + 1];
            if (creep_moves[id].empty()) first_move[id] = step;
            creeps.push_back(Creep(id, cx, cy, hp, first_move[id]));
            if (id > max_id) max_id = id;
            creep_moves[id].push_back(pii(cx, cy));
        }
        
        double tmp = GetSeconds();
        if (step > 0) {
            fake_creeps = creeps;
            generate_fake(step);
        }
        t_o[0] += GetSeconds() - tmp;
        
        set<int> damagers;
        int damage_taken = calc_dmg(creeps, samples, damagers, inf, true);
        int killed1 = killed;
        //int fake_money1, fake_money2;
        //int ignore = full_sim(fake_creeps, fake_creep_moves, step, 10000000, fake_money1); fake_money1 += money;
        int total_spent = 0, total_built = 0;
        if (damage_taken > 0) {
            cerr << "Problem at step " << step;
        }
        while (1) {
            if (!damage_taken) break;
            CL(0, health_sum);
            F0(i, SZ(creep) / 4) {
                int id = creep[4 * i];
                if (!damagers.count(id)) continue;
                int cx = creep[4 * i + 3];
                int cy = creep[4 * i + 2];
                int hp = creep[4 * i + 1];
                F0(j, 4) if ((1 << j)&path_masks[cx][cy]) {
                    int x = cx + dx[j], y = cy + dy[j];
                    health_sum[x][y] += 24 * hp; // bc[path_masks[cx][cy]];
                }
            }
            
            bool found = false;
            
            int bestIndex = -1, currIndex = -1, bestTaken = -1, bestKilled = -1;
            double bestRatio = -1.0;
            for (PossibleTower& pt : possible_towers) if (!pt.built && on_free(pt.x, pt.y) && pt.cost() <= money && !found) {
                if (pt.goodness * damage_taken <= bestRatio) break;
                currIndex++;
                bool can_help = false;
                for (int dx = -pt.range(); dx <= pt.range(); dx++)
                    for (int dy = -pt.range(); dy <= pt.range(); dy++) if (d2(dx, dy) <= sq(pt.range())) {
                        int x = pt.x + dx, y = pt.y + dy; if (on_path(x, y) && health_sum[x][y] > 0) can_help = true;
                    }
                
                double current_goodness = pt.goodness;
                
                bool doit = GetSeconds() - time_start + time_spent < 10.0 && step < 1500; 
                //doit = step < 1500;
                
                if (can_help || doit)
                    {
                    add_tower(Tower(pt.x, pt.y, pt.type));
                    int new_dmg = calc_dmg(creeps, samples, damagers, damage_taken);
                    if (new_dmg < damage_taken) {
                        
                        double ratio = (damage_taken - new_dmg) * current_goodness;
                        if (ratio > bestRatio) {
                            bestRatio = ratio; bestIndex = currIndex; bestTaken = new_dmg; bestKilled = killed;
                        }
                    }
                    remove_tower();
                    }
            }
            else currIndex++;
            if (bestIndex != -1) {
                PossibleTower& pt = possible_towers[bestIndex];
                
                add_tower(Tower(pt.x, pt.y, pt.type));
                
                bool bad = false;
                
                if (step >= 100 && bestKilled * creepMoney <= killed1 * creepMoney + samples * pt.cost()) {
                    //int ignore = full_sim(fake_creeps, fake_creep_moves, step, 10000000, fake_money2);
                    //fake_money2 += money - pt.cost();
                    //cerr << fake_money1 << " " << fake_money2 << endl;
                    //bad = fake_money1 > fake_money2;
                }
                
                if (bad) {
                    remove_tower();
                    // cerr << "Not worth $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << endl;
                }
                else{
                    //cerr << "Tower: " << step << " " << bestTaken << endl;
                    found = true;
                    damage_taken = bestTaken;
                    total_spent += pt.cost();
                    total_built++;
                    money -= pt.cost();
                    board[pt.x][pt.y] = 'T';
                    ret.push_back(pt.y); ret.push_back(pt.x); ret.push_back(pt.type);
                    pt.built = 1;
                }
            }
            if (!found) break;
        }
        if (damage_taken > 0 && money > 1000) {
            //cerr << endl;
            //while (1);
        }
        if (total_built || damage_taken) {
            cerr << "  " << total_built << " towers";
            cerr << (damage_taken > 0 ? " didn't solve " : " solved ") << "the problem" << endl;
        }
        /*
         if (damage_taken > 0) {
         while (total_built--) {
         remove_tower();
         }
         }*/
        
        if (step == SIMULATION_TIME) {
            set<int> S;
            sort(towers.begin(), towers.end(), [](Tower x, Tower y) { return pii(x.x,x.y) < pii(y.x,y.y); });
            for (auto x : towers) if (!S.count(x.type)) {
                S.insert(x.type);
                cerr << x.x << "," << x.y << "   - " << x.type << endl;
            }
            Report();
        }
        time_spent += GetSeconds() - time_start;
        //if (SZ(ret)) cerr << step << " " << SZ(ret) / 3 << endl;
        return ret;
    }
    
    int init(vector<string> _board, int _money, int _creepHealth, int _creepMoney, vector<int> t) {
        Init();
        double time_start = GetSeconds();
        step = 0;
        F0(i, n) F0(j, n) in_tower[i][j].clear();
        board = _board; money = _money; creepHealth = _creepHealth; creepMoney = _creepMoney;
        n = SZ(board);  ttn = SZ(t) / 3;
        F0(i, ttn) {
            tower_types[i] = TowerType(t[3 * i], t[3 * i + 1], t[3 * i + 2]);
        }
        F0(i, ttn) F0(j, ttn) {
            TowerType ti = tower_types[i], tj = tower_types[j];
            if (ti.cost <= tj.cost && ti.range >= tj.range && ti.damage >= tj.damage) {
                if (ti.cost < tj.cost || ti.range > tj.range || ti.damage > tj.damage || i < j) tower_types[j].dead = 1;
            }
        }
        
        baseCnt = 0; spawnCnt = 0;
        CL(0, total_damage);
        CL(-1, base_dmg);
        F0(i, n) F0(j, n) if (isdigit(board[i][j])) {
            baseCnt++;
            int baseId = board[i][j] - '0';
            baseX[baseId] = i;
            baseY[baseId] = j;
            F0(k, 4) {
                int x = i + dx[k], y = j + dy[k]; if (on_path(x, y)) base_dmg[x][y] = baseId;
            }
            health[baseId] = 1000;
        }
        else if (on_border(i, j) && on_path(i, j)) {
            spawnX[spawnCnt] = i;
            spawnY[spawnCnt] = j;
            spawnCnt++;
        }
        
        F0(b, baseCnt) {
            F0(dir0, 4) fill_from(from_base[b][dir0], dir0, baseX[b], baseY[b]);
        }
        F0(b, spawnCnt) {
            F0(dir0, 4) fill_from(from_spawn[b][dir0], dir0, spawnX[b], spawnY[b]);
        }
        
        F0(i, n) F0(j, n) {
            path_masks[i][j] = path_dist_sum[i][j] = 0;
            F0(k,4) path_dist[i][j][k] = 0;
            F0(b, baseCnt) F0(s, spawnCnt) F0(spawn_dir, 4) {
                int base_dir = (spawn_dir + 2) % 4;
                if (from_base[b][base_dir][i][j] && from_spawn[s][spawn_dir][i][j]) {
                    F0(k, 4) {
                        int x = i + dx[k], y = j + dy[k];
                        if (d2(x-baseX[b], y-baseY[b]) < d2(i-baseX[b], j-baseY[b]) && on_path(x, y) && from_base[b][base_dir][x][y] && from_spawn[s][spawn_dir][x][y]) {
                            path_masks[i][j] |= (1 << k);
                            int dist = abs(i - baseX[b]) + abs(j - baseY[b]); //dist = 1;
                            if (path_dist[i][j][k] == 0 || path_dist[i][j][k] > dist) path_dist[i][j][k] = dist;
                        }
                    }
                }
            }
            int prod = 1; F0(k, 4) if (path_dist[i][j][k]) prod *= path_dist[i][j][k];
            F0(k, 4) if (path_dist[i][j][k]) path_dist[i][j][k] = prod / path_dist[i][j][k];
            F0(k, 4) path_dist_sum[i][j] += path_dist[i][j][k];
        }
        
        towers.clear();
        possible_towers.clear();
        F0(x, n) F0(y, n) if (on_free(x, y)) {
            vector<PossibleTower> this_pos;
            F0(t, ttn) {
                TowerType tt = tower_types[t]; if (tt.dead) continue;
                PossibleTower pt;
                pt.x = x; pt.y = y; pt.type = t; pt.built = 0; pt.basic_goodness = 0.0;
                
                for (int dx = -tt.range; dx <= tt.range; dx++) {
                    for (int dy = -tt.range; dy <= tt.range; dy++) if (dx * dx + dy * dy <= tt.range * tt.range) {
                        if (on_path(x + dx, y + dy)) pt.basic_goodness += tt.damage;
                    }
                }
                
                pt.basic_goodness = pt.basic_goodness / tt.cost;
                pt.goodness = pt.basic_goodness;
                
                if (pt.goodness > 0) {
                    //possible_towers.push_back(pt);
                    this_pos.push_back(pt);
                }
            }
            sort(this_pos.begin(), this_pos.end());
            if (!this_pos.empty()) possible_towers.push_back(this_pos[0]);
        }
        sort(possible_towers.begin(), possible_towers.end());
        F0(i, ttn) if (!tower_types[i].dead) cerr << i << "->(" << tower_types[i].range << "," << tower_types[i].damage << ") = " << tower_types[i].cost << endl;
        //cerr << "Tower types:  " << ttn << endl;
        cerr << "Creep health: " << creepHealth << endl;
        cerr << "Creep money:  " << creepMoney << endl;
        //cerr << "Possible towers: " << SZ(possible_towers) << endl;
        time_spent = GetSeconds() - time_start;
        return 0;
    }
};


#include <fstream>

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



