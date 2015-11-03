#include <cstdio>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <cstdlib>
#include <unordered_set>

using namespace std;


#define forn(i, n) for (int i = 0; i < (int)(n); i++)
typedef long long ll;
typedef pair<int, int> pii;
typedef vector<int> VI;
typedef vector<VI> VVI;
typedef vector<VVI> VVVI;
typedef vector<pii> VP;
typedef vector<string> VS;

const int di[] = {-1, 0, 1, 0};
const int dj[] = {0, -1, 0, 1};
#define sqr(x) ((x)*(x))
#define DBG(x) cerr << #x << ": " << x << endl;
#define log(args...) fprintf(stderr, "[turn %d] ", turn); fprintf(stderr, args); cerr << endl;
// #define log(args...)

template<class T>
ostream& operator<<(ostream& out, vector<T> v) {
    out << "[";
    forn(i, v.size()) {
        if (i) out << ", ";
        out << v[i];
    }
    out << "]";
    return out;
}

const int md = 123456791;
struct RND {
    int cur;
    
    RND() { cur = 1991; }
    RND(int x) { cur = x; }
    
    int next(int n) {
        cur = cur * 17 % md;
        return cur % n;
    }
    
    double nextDouble() {
        cur = cur * 17 % md;
        return double(cur) / md;
    }
};

struct Tower {
    int R, D, C;
    size_t id;
    
    bool operator != (const Tower& rhs) {
        return C != rhs.C || R != rhs.R || D != rhs.D;
    }
    
    friend ostream& operator<<(ostream& out, const Tower& t) {
        return out << "(R=" << t.R << ", D=" << t.D << ", C=" << t.C << ")";
    }
};

struct Creep {
    int id, h, c, r;
    int targetBase, pm;
    
    Creep() { pm = -1; }
    Creep(int a, int b, int z, int d): id(a), h(b), c(z), r(d), pm(-1) {}
};

struct StartingCreep {
    int turn, borderInd, id, h;
    
    StartingCreep(int a, int b, int c, int d): turn(a), borderInd(b), id(c), h(d) {}
};

struct BuiltTower {
    int r, c;
    Tower bt;
    Creep* target;
    
    BuiltTower() {}
    BuiltTower(int a, int b, const Tower& z): r(a), c(b), bt(z) {}
};

VP inRange[64][64][5];

struct TowersMap {
    VVVI firingTowers;
    vector<BuiltTower> builtTowers;
    
    void setSize(int n) {
        firingTowers = VVVI(n, VVI(n));
        builtTowers.clear();
    }
    
    void pushTower(const BuiltTower& bt) {
        builtTowers.push_back(bt);
        builtTowers.back().target = nullptr;
        for (const pii& c: inRange[bt.r][bt.c][bt.bt.R-1])
            firingTowers[c.first][c.second].push_back(builtTowers.size() - 1);
    }
    
    void popTower() {
        const BuiltTower& bt = builtTowers.back();
        for (const pii& c: inRange[bt.r][bt.c][bt.bt.R-1])
            firingTowers[c.first][c.second].pop_back();
        builtTowers.pop_back();
    }
    
    void clearCreeps() {
        for (BuiltTower& bt: builtTowers)
            bt.target = nullptr;
    }
    
    void addCreep(Creep& cr) {
        for (int btId: firingTowers[cr.r][cr.c]) {
            BuiltTower& bt = builtTowers[btId];
            pii d2(sqr(bt.r - cr.r) + sqr(bt.c - cr.c), cr.id);
            if (d2.first <= sqr(bt.bt.R))
                if (bt.target == nullptr ||
                    pii(sqr(bt.r - bt.target->r) + sqr(bt.c - bt.target->c), bt.target->id) > d2)
                    bt.target = &cr;
        }
    }
};


// given
VS ma;
int n;
int turn;
int startingCreepHealth, creepMoney, initMoney;
vector<Tower> towers;
int baseN;
VI baseHealth;
VP basePos;

// my
VVI allp[10], alld[10];
VVI baseDng;
VVI dir;
VI usedTowers;
VVI distBase;
unordered_map<int, pii> prevCreepPos;
VP buildingCells;
VI willDefend;
VP border;
unordered_set<int> creepsSeen;
TowersMap towersMap;

///////////////// CODE

int creepHealth(int t = -1) {
    if (t == -1) t = turn;
    return startingCreepHealth * (1 << (t / 500));
}

class PathDefense {
public:
    VI placeTowers(VI creep, int money, VI baseHealth);
    int init(VS board, int money, int creepHealth, int creepMoney, VI towerTypes);
};


vector<Creep> makeCreeps(const VI& creep) {
    vector<Creep> res;
    for (size_t i = 0; i < creep.size(); i += 4) {
        res.push_back(Creep{creep[i], creep[i+1], creep[i+2], creep[i+3]});
    }
    return res;
}

void traceCreep(const VVI& p, const Creep& cr, VVI& dng, int healthLeft) {
    int si = cr.r, sj = cr.c;
    while (true) {
        int z = p[si][sj];
        si += di[z], sj += dj[z];
        if (p[si][sj] == -1) break;
        dng[si][sj] += healthLeft; // * 7 + baseDng[si][sj];
        int cd = 0;
        forn(q, 4) if (q != (z ^ 2) && (dir[si][sj] & (1 << q))) cd++;
        healthLeft = healthLeft * 3 / (cd + 2);
    }
}

bool possibleGo(const Creep& cr, int baseId) {
    int pd = allp[baseId][cr.r][cr.c];
    if (pd == -1) return false;
    pii next(cr.r + di[pd], cr.c + dj[pd]);
    return prevCreepPos[cr.id] != next;
}

int gKILLED;
VVI traceCreeps(const vector<Creep>& creeps, TowersMap& tm, int curTurn, int& dmg) {
    dmg = 0;
    gKILLED = 0;
    vector<Creep> ghosts = creeps;
    int alive = ghosts.size();
    for (Creep& g: ghosts) {
        g.targetBase = 0;
        forn(b, baseN) {
            if (possibleGo(g, b))
                if (alld[b][g.r][g.c] < alld[g.targetBase][g.r][g.c])
                    g.targetBase = b;
        }
    }
    
    // log("Tracing creeps...");
    while (alive > 0 && curTurn < 2000) {
        curTurn++;
        for (Creep& g: ghosts)
            if (g.h > 0) {
                const VVI& p = allp[g.targetBase];
                int w = p[g.r][g.c];
                g.r += di[w], g.c += dj[w];
                if (p[g.r][g.c] == -1) {
                    // log("Creep %d reached base", g.id);
                    alive--;
                    int baseId = ma[g.r][g.c] - '0';
                    // if ((1000.0 - baseHealth[baseId]) / turn * 2000.0 < 1000 || turn < 1000)
                    if (baseHealth[baseId] > 0 && willDefend[baseId])
                        g.h = -g.h;
                    else
                        g.h = 0;
                }
            }
        
        tm.clearCreeps();
        for (Creep& g: ghosts)
            if (g.h > 0)
                tm.addCreep(g);
        
        for (const BuiltTower& t: tm.builtTowers) {
            if (t.target != nullptr) {
                // log("Creep %d at (%d, %d) with health %d is shot by tower with damage %d",
                //     t.target->id, t.target->r, t.target->c, t.target->h, t.bt.D);
                if (t.target->h > 0) {
                    t.target->h -= t.bt.D;
                    if (t.target->h <= 0) {
                        alive--;
                        gKILLED++;
                        t.target->h = 0;
                    }
                }
            }
        }
    }
    
    // log("Traced.")
    VVI curDng(n, VI(n, 0));
    bool traced = false;
    forn(ic, ghosts.size())
    if (ghosts[ic].h < 0) { // that means this creep dealt damage to base
        dmg -= ghosts[ic].h;
        // dmg += creepHealth() * 0.1;
        traceCreep(allp[ghosts[ic].targetBase], creeps[ic], curDng, -ghosts[ic].h);
        // if (turn >= 1555 && turn <= 1565) {
        //     fprintf(stderr, "creep at %d,%d has %d health, down to %d\n",
        //         creeps[ic].r, creeps[ic].c, creeps[ic].h, -ghosts[ic].h);
        // }
        traced = true;
    }
    
    return curDng;
}

int simulateCreeps(const vector<Creep>& creeps, TowersMap& tm, int curTurn,
                   double creepsRate, int turnsToTrace) {
    int dmg = 0;
    int gKILLED = 0;
    RND rnd;
    vector<Creep> ghosts = creeps;
    VI baseHealth_bckp = baseHealth;
    int alive = ghosts.size();
    for (Creep& g: ghosts) {
        g.targetBase = 0;
        forn(b, baseN) {
            if (possibleGo(g, b))
                if (alld[b][g.r][g.c] < alld[g.targetBase][g.r][g.c])
                    g.targetBase = b;
        }
    }
    
    // log("simulating creeps with creepsRate %.4f", creepsRate);
    while (curTurn < 2000 && turnsToTrace > 0) {
        curTurn++;
        turnsToTrace--;
        // cerr << RAND_MAX << endl;
        if (rnd.nextDouble() < creepsRate) {
            int ind = border.size() * rnd.nextDouble();
            if (ind >= border.size()) ind = border.size() - 1;
            const pii& c = border[ind];
            ghosts.emplace_back(-1, creepHealth(curTurn), c.second, c.first);
            Creep& g = ghosts.back();
            g.targetBase = 0;
            forn(b, baseN) {
                if (possibleGo(g, b))
                    if (alld[b][g.r][g.c] < alld[g.targetBase][g.r][g.c])
                        g.targetBase = b;
            }
            alive++;
        }
        if (alive == 0) continue;
        for (Creep& g: ghosts)
            if (g.h > 0) {
                const VVI& p = allp[g.targetBase];
                int w = p[g.r][g.c];
                g.r += di[w], g.c += dj[w];
                if (p[g.r][g.c] == -1) {
                    // log("Creep %d reached base", g.id);
                    alive--;
                    int baseId = ma[g.r][g.c] - '0';
                    // if ((1000.0 - baseHealth[baseId]) / turn * 2000.0 < 1000 || turn < 1000)
                    if (baseHealth[baseId] > 0 && willDefend[baseId]) {
                        // baseHealth[baseId] -= g.h * 0.7;
                        g.h = -g.h;
                    } else {
                        g.h = 0;
                    }
                }
            }
        
        tm.clearCreeps();
        for (Creep& g: ghosts)
            if (g.h > 0)
                tm.addCreep(g);
        
        for (const BuiltTower& t: tm.builtTowers) {
            if (t.target != nullptr) {
                // log("Creep %d at (%d, %d) with health %d is shot by tower with damage %d",
                //     t.target->id, t.target->r, t.target->c, t.target->h, t.bt.D);
                if (t.target->h > 0) {
                    t.target->h -= t.bt.D;
                    if (t.target->h <= 0) {
                        alive--;
                        gKILLED++;
                        t.target->h = 0;
                    }
                }
            }
        }
    }
    
    // log("Traced.")
    forn(ic, ghosts.size())
    if (ghosts[ic].h < 0) { // that means this creep dealt damage to base
        dmg -= ghosts[ic].h;
    }
    
    baseHealth = baseHealth_bckp;
    
    // log("%d, %d", gKILLED, dmg);
    return gKILLED * creepMoney - dmg;
}

double NORM = 1e-4;
int calcCellDng(int bdng) {
    ll res = 0;
    if (turn >= 1500) return NORM * bdng * 8 * (2000 - turn);
    res += bdng * 8 * 500;
    if (turn >= 1000) return NORM * (res + bdng * 4 * (1500 - turn));
    res += bdng * 4 * 500;
    if (turn >= 500) return NORM * (res + bdng * 2 * (1000 - turn));
    res += bdng * 2 * 500;
    return NORM * (res + bdng * (500 - turn));
}

void calcTowerScores(const Tower& t, int ti, int tj, const VVI& dng,
                     int& cntR, int& cntC, int& cntM) {
    cntR = cntC = cntM = 0;
    
    int base = 0;
    // int dealt = 0;
    for (const pii& c: inRange[ti][tj][t.R - 1]) {
        int ni = c.first, nj = c.second;
        cntR += dng[ni][nj] * 300;
        // dealt += t.D;
        base += calcCellDng(baseDng[ni][nj]);
        if (dng[ni][nj] > 0) {
            cntC++;
            cntM = max(cntM, dng[ni][nj]);
        }
    }
    if (cntR > 0) {
        cntR += base;
    }
}

BuiltTower getTowerToBuild(const VVI& curDng,
                           const VI& possibleTowers,
                           const vector<BuiltTower>& builtTowers,
                           vector<Creep>& creeps) {
    double bsc = 0;
    int bi = -1, bj = -1;
    Tower bt;
    bool nonzero = false;
    forn(i, n) forn(j, n) if (curDng[i][j] > 0) { nonzero = true; break; }
    if (!nonzero) return BuiltTower(bi, bj, bt);
    int cntR, cntC, cntM;
    
    for (const pii& cell: buildingCells) {
        int i = cell.first, j = cell.second;
        if (ma[i][j] == '#') {
            // calcTowerScores(i, j, curDng, cntR, cntC, cntM);
            for (int pt: possibleTowers) {
                const Tower& t = towers[pt];
                calcTowerScores(t, i, j, curDng, cntR, cntC, cntM);
                double tsc = t.D * cntR * 1.0 / (pow(t.C, 1.88) - distBase[i][j] / 33.0);
                if (cntM > 0) {
                    tsc *= (1.0 + 0.1 * t.D * cntC / cntM);
                    if (tsc > bsc) {
                        bsc = tsc;
                        bi = i;
                        bj = j;
                        bt = t;
                    }
                }
            }
        }
    }
    
    return BuiltTower(bi, bj, bt);
}

void moveCreeps(vector<Creep>& creeps) {
    for (Creep& cr: creeps) {
        cr.targetBase = 0;
        forn(b, baseN)
        if (alld[b][cr.r][cr.c] < alld[cr.targetBase][cr.r][cr.c])
            cr.targetBase = b;
    }
    
    for (Creep& cr: creeps) {
        const VVI& p = allp[cr.targetBase];
        int w = p[cr.r][cr.c];
        cr.r += di[w], cr.c += dj[w];
    }
}

void rndMoveCreeps(vector<Creep>& creeps, RND& rnd) {
    for (Creep& cr: creeps) {
        while (true) {
            int q = rnd.next(4);
            if ((dir[cr.r][cr.c] & (1 << q)) > 0 && q != cr.pm)
                { cr.r += di[q], cr.c += dj[q]; cr.pm = q ^ 2; break; }
        }
    }
}

double getExpectedMoney(const vector<Creep>& creeps, TowersMap& tm, int cTurn) {
    if (turn < 1000) return 0;
    double creepsRate = double(creepsSeen.size()) / turn;
    return simulateCreeps(creeps, tm, cTurn, creepsRate, 1000);
}

VI solve(vector<Creep>& creeps,
         int money,
         VI& baseHealth,
         int curTurn,
         int& dmg) {
    ::baseHealth = baseHealth;
    VI res;
    
    // log("solve for turn %d", curTurn);
    
    int built = 0;
    VVI curDng = traceCreeps(creeps, towersMap, curTurn, dmg);
    double wasMoney = getExpectedMoney(creeps, towersMap, curTurn);
    while (true) {
        VI possibleTowers;
        forn(q, towers.size())
        if (towers[q].C <= money) {
            possibleTowers.push_back(q);
        }
        
        if (possibleTowers.empty() || dmg == 0) break;
        BuiltTower toBuild = getTowerToBuild(curDng, possibleTowers, towersMap.builtTowers, creeps);
        if (toBuild.r == -1) break;
        
        if (turn > 1555 && dmg < toBuild.bt.C) {
            log("decided to skip tower building, dmg is %d, tower cost is %d", dmg, toBuild.bt.C);
            break;
        }
        
        // log("dmg %d, will build tower at (%d, %d)", dmg, toBuild.r, toBuild.c);
        towersMap.pushTower(toBuild);
        int dmgNew = 0;
        curDng = traceCreeps(creeps, towersMap, curTurn, dmgNew);
        double newMoney = getExpectedMoney(creeps, towersMap, curTurn);
        // log("money without tower: %.1f, money with tower: %.1f", wasMoney, newMoney);
        double kDmg = 5;
        if (turn > 1000 && newMoney - toBuild.bt.C - kDmg * dmgNew < wasMoney - kDmg * dmg) {
            log("decided to skip tower building because of economy:");
            log("Without tower: %.1f (dmg %d), with tower: %.1f (%.1f - %d, dmg (%d))",
                wasMoney, dmg, newMoney - toBuild.bt.C, newMoney, toBuild.bt.C, dmgNew);
            towersMap.popTower();
            break;
        }
        if (turn > 1555 && (3000 - turn) / 300.0 * (dmg - dmgNew) < toBuild.bt.C) {
            log("decided to skip tower building, dmg: %d->%d, tower cost is %d", dmg, dmgNew, toBuild.bt.C);
            towersMap.popTower();
            break;
        }
        
        money -= toBuild.bt.C;
        res.push_back(toBuild.c);
        res.push_back(toBuild.r);
        res.push_back(toBuild.bt.id);
        ma[toBuild.r][toBuild.c] = 'T';
        // cerr << "built " << toBuild.bt << " at " << toBuild.r << "," << toBuild.c << endl;
        built++;
        wasMoney = newMoney;
        dmg = dmgNew;
        if (dmg < creepHealth() * toBuild.bt.C * 0.123) break;
        // if (built > 3 - turn / 500) break;
    }
    
    return res;
}

void simpleSolve(vector<Creep>& creeps, int& money, const VI& baseHealth, int curTurn) {
    ::baseHealth = baseHealth;
    
    int built = 0, dmg = 0;
    VVI curDng = traceCreeps(creeps, towersMap, curTurn, dmg);
    while (true) {
        VI possibleTowers;
        forn(q, towers.size())
        if (towers[q].C <= money) {
            possibleTowers.push_back(q);
        }
        
        if (possibleTowers.empty() || dmg == 0) break;
        BuiltTower toBuild = getTowerToBuild(curDng, possibleTowers, towersMap.builtTowers, creeps);
        if (toBuild.r == -1) break;
        
        // if (turn > 1555 && dmg < toBuild.bt.C) {
        //     log("decided to skip tower building, dmg is %d, tower cost is %d", dmg, toBuild.bt.C);
        //     break;
        // }
        
        // log("dmg %d, will build tower at (%d, %d)", dmg, toBuild.r, toBuild.c);
        towersMap.pushTower(toBuild);
        int dmgNew = 0;
        curDng = traceCreeps(creeps, towersMap, curTurn, dmgNew);
        // double newMoney = getExpectedMoney(creeps, towersMap, curTurn);
        // log("money without tower: %.1f, money with tower: %.1f", wasMoney, newMoney);
        // double kDmg = 3;
        /*if (turn > 1000 && newMoney - toBuild.bt.C - kDmg * dmgNew < wasMoney - kDmg * dmg) {
         log("decided to skip tower building because of economy:");
         log("Without tower: %.1f (dmg %d), with tower: %.1f (%.1f - %d, dmg (%d))",
         wasMoney, dmg, newMoney - toBuild.bt.C, newMoney, toBuild.bt.C, dmgNew);
         towersMap.popTower();
         break;
         }*/
        // if (turn > 1555 && (3000 - turn) / 300.0 * (dmg - dmgNew) < toBuild.bt.C) {
        //     log("decided to skip tower building, dmg: %d->%d, tower cost is %d", dmg, dmgNew, toBuild.bt.C);
        //     towersMap.popTower();
        //     break;
        // }
        
        money -= toBuild.bt.C;
        // res.push_back(toBuild.c);
        // res.push_back(toBuild.r);
        // res.push_back(toBuild.bt.id);
        ma[toBuild.r][toBuild.c] = 'T';
        // cerr << "built " << toBuild.bt << " at " << toBuild.r << "," << toBuild.c << endl;
        built++;
        // wasMoney = newMoney;
        dmg = dmgNew;
        if (dmg < creepHealth() * toBuild.bt.C * 0.123) break;
        // if (built > 3 - turn / 500) break;
    }
}


VI PathDefense::placeTowers(VI creep, int money, VI baseHealth) {
    turn++;
    vector<Creep> creeps = makeCreeps(creep);
    int dmgNow = 0;
    // cerr << turn << ": " << baseHealth << endl;
    VI res = solve(creeps, money, baseHealth, turn, dmgNow);
    
    for (size_t i = 0; i < res.size(); i += 3) {
        // cerr << "build " << res[i+2] << " at " << res[i+1] << "," << res[i] << endl;
        usedTowers[res[i+2]]++;
        buildingCells.erase(find(buildingCells.begin(), buildingCells.end(), pii{res[i+1], res[i]}));
    }
    
    if (turn == 1999) {
        DBG(usedTowers);
    }
    
    for (const Creep& cr: creeps) {
        /*const pii& pr = prevCreepPos[cr.id];
         if (abs(pr.first - cr.r) + abs(pr.second - cr.c) == 1) {
         forn(q, 4)
         if (pr.first + di[q] == cr.r && pr.second + dj[q] == cr.c)
         dir[pr.first][pr.second] |= 1 << q;
         }*/
        prevCreepPos[cr.id] = pii(cr.r, cr.c);
        creepsSeen.insert(cr.id);
    }
    
    return res;
}

void bfs(int si, int sj, VVI& p, VVI& d) {
    p.assign(n, VI(n, -1));
    d.assign(n, VI(n, 1 << 30));
    VP q;
    q.emplace_back(si, sj);
    d[si][sj] = 0;
    size_t qb = 0;
    int ri = si, rj = sj;
    
    while (qb < q.size()) {
        si = q[qb].first, sj = q[qb].second;
        qb++;
        
        forn(w, 4) {
            int ni = si + di[w];
            int nj = sj + dj[w];
            if (ni >= 0 && ni < n && nj >= 0 && nj < n && ma[ni][nj] == '.' && p[ni][nj] == -1) {
                if (abs(ni - ri) + abs(nj - rj) <= d[si][sj] + 5) {
                    p[ni][nj] = w ^ 2;
                    d[ni][nj] = d[si][sj] + 1;
                    q.emplace_back(ni, nj);
                }
            }
        }
    }
}


void traceGhost(int pi, int pj, int i, int j, int pw) {
    if (pw == 0) return;
    if (ma[i][j] >= '0' && ma[i][j] <= '9') return;
    int cd = 0;
    baseDng[i][j] += pw;
    forn(q, 4)
    if (dir[i][j] & (1 << q)) {
        if (i + di[q] != pi || j + dj[q] != pj)
            cd++;
    }
    forn(q, 4)
    if (dir[i][j] & (1 << q)) {
        if (i + di[q] != pi || j + dj[q] != pj)
            traceGhost(i, j, i + di[q], j + dj[q], pw / cd);
    }
}

bool cmpCell(const pii& c1, const pii& c2) {
    return abs(c1.first - n / 2) + abs(c1.second - n / 2) >
    abs(c2.first - n / 2) + abs(c2.second - n / 2);
}

int simulateGame(const vector<StartingCreep>& allCreeps, bool R) {
    towersMap.setSize(n);
    size_t ic = 0;
    vector<Creep> creeps;
    VI baseHealth(baseN, 1000);
    int money = initMoney;
    VS ma_bckp = ma;
    RND rnd;
    forn (curTurn, 2000) {
        while (ic < allCreeps.size() && allCreeps[ic].turn == curTurn) {
            const auto& cr = allCreeps[ic++];
            creeps.emplace_back(cr.id, cr.h, border[cr.borderInd].second, border[cr.borderInd].first);
        }
        
        simpleSolve(creeps, money, baseHealth, curTurn + 1);
        
        if (R) rndMoveCreeps(creeps, rnd);
        else moveCreeps(creeps);
        
        for (size_t i = 0; i < creeps.size(); i++) {
            const Creep& cr = creeps[i];
            if (ma[cr.r][cr.c] >= '0' && ma[cr.r][cr.c] <= '9') {
                int baseId = ma[cr.r][cr.c] - '0';
                baseHealth[baseId] = max(baseHealth[baseId] - cr.h, 0);
                swap(creeps[i], creeps.back());
                creeps.pop_back();
                i--;
            }
        }
        
        towersMap.clearCreeps();
        for (Creep& cr: creeps)
            towersMap.addCreep(cr);
        
        for (const BuiltTower& t: towersMap.builtTowers) {
            if (t.target != nullptr) {
                // log("Creep %d at (%d, %d) with health %d is shot by tower with damage %d",
                //     t.target->id, t.target->r, t.target->c, t.target->h, t.bt.D);
                if (t.target->h > 0) {
                    t.target->h -= t.bt.D;
                    if (t.target->h <= 0) {
                        money += creepMoney;
                        t.target->h = 0;
                    }
                }
            }
        }
        
        for (size_t i = 0; i < creeps.size(); i++) {
            const Creep& cr = creeps[i];
            if (cr.h <= 0) {
                swap(creeps[i], creeps.back());
                creeps.pop_back();
                i--;
            }
        }
    }
    
    ma = ma_bckp;
    int resScore = money;
    // DBG(baseHealth);
    for (int bh: baseHealth) resScore += bh;
    // DBG(resScore);
    return resScore;
}

vector<StartingCreep> generateLevelCreeps(double hardLevel) {
    int cnt = 750 + 250 * hardLevel;
    vector<StartingCreep> res;
    res.reserve(cnt);
    forn(i, cnt) {
        int turn = rand() % 2000;
        res.emplace_back(turn, rand() % border.size(), i, creepHealth(turn));
    }
    
    int waves = hardLevel * 4; // 4, 8, 12
    int wi = 0;
    forn(w, waves) {
        int borderInd = rand() % border.size();
        int waveLen = 5 + hardLevel * 15;
        int waveStart = rand() % (2000 - waveLen);
        forn(i, waveLen) {
            if (wi >= res.size()) break;
            int tt = waveStart + rand() % waveLen;
            res[wi++] = StartingCreep(tt, borderInd, res[wi].id, creepHealth(tt));
        }
        if (wi >= res.size()) break;
    }
    
    VI bc(border.size(), 0);
    for (auto& cr: res)
        bc[cr.borderInd]++;
    DBG(bc);
    
    sort(res.begin(), res.end(), [](const StartingCreep& a, const StartingCreep& b) {return a.turn < b.turn; });
    
    return res;
}

void estimateDefendingTowers() {
    VI bbDng(baseN, 0), bi(baseN, 0);
    forn(b, baseN) {
        forn(q, 4) {
            int ni = basePos[b].first + di[q],
            nj = basePos[b].second + dj[q];
            bbDng[b] += baseDng[ni][nj];
        }
        bi[b] = b;
    }
    
    sort(bi.begin(), bi.end(), [bbDng](int i, int j) { return bbDng[i] < bbDng[j]; });
    
    vector<vector<StartingCreep>> scenarios = {
        generateLevelCreeps(1),
        generateLevelCreeps(1.5),
        generateLevelCreeps(2),
        generateLevelCreeps(2.5),
        generateLevelCreeps(3)
    };
    
    willDefend = VI(baseN, 0);
    int bc = 0, bestVal = 1e9;
    for (auto& sc: scenarios)
        bestVal = min(bestVal, simulateGame(sc, true));
    // int bc = 0, bestVal = simulateGame(futureCreeps1, false) +
    //                       simulateGame(futureCreeps2, false) +
    //                       simulateGame(futureCreeps3, false) +
    //                       simulateGame(futureCreeps1, true) +
    //                       simulateGame(futureCreeps2, true) +
    //                       simulateGame(futureCreeps3, true);
    cerr << willDefend << ": " << bestVal << endl;
    
    forn(qi, baseN) {
        willDefend[bi[qi]] = 1;
        int curVal = 1e9;
        for (auto& sc: scenarios)
            curVal = min(curVal, simulateGame(sc, true));
        
        cerr << willDefend << ": " << curVal << endl;
        
        if (curVal > bestVal) {
            bc = qi + 1;
            bestVal = curVal;
        }
    }
    
    willDefend = VI(baseN, 0);
    forn(qi, bc)
    willDefend[bi[qi]] = 1;
}

void preprocess() {
    srand(0);
    forn(q, n) {
        if (ma[q][0] == '.') border.emplace_back(q, 0);
        if (q != 0 && ma[0][q] == '.') border.emplace_back(0, q);
        if (ma[q][n-1] == '.') border.emplace_back(q, n-1);
        if (q != n-1 && ma[n-1][q] == '.') border.emplace_back(n-1, q);
    }
    random_shuffle(border.begin(), border.end());
    
    baseDng = VVI(n, VI(n));
    dir = VVI(n, VI(n));
    distBase = VVI(n, VI(n, 1 << 30));
    basePos = VP(10);
    forn(i, n) forn(j, n)
    if (ma[i][j] >= '0' && ma[i][j] <= '9') {
        int baseId = ma[i][j] - '0';
        VVI& p = allp[baseId];
        VVI& d = alld[baseId];
        baseN = max(baseN, baseId + 1);
        basePos[baseId] = pii(i, j);
        bfs(i, j, p, d);
        forn(ci, n) forn(cj, n) {
            if (p[ci][cj] != -1)
                dir[ci][cj] |= 1 << p[ci][cj];
            distBase[ci][cj] = min(distBase[ci][cj], abs(ci - i) + abs(cj - j));
        }
    }
    
    for (const pii& c: border)
        traceGhost(-1, -1, c.first, c.second, 1 << 11);
    
    DBG(baseN);
    // cerr << "Got baseDng:\n";
    // forn(i, n) {
    //     DBG(baseDng[i]);
    // }
    
    forn(i, n)
    forn(j, n)
    if (ma[i][j] == '#') {
        for (int ni = i - 5; ni <= i + 5; ni++)
            for (int nj = j - 5; nj <= j + 5; nj++)
                if (ni >= 0 && ni < n && nj >= 0 && nj < n && ma[ni][nj] == '.') {
                    int d2 = sqr(i - ni) + sqr(j - nj);
                    pii c(ni, nj);
                    forn(q, 5)
                    if (d2 <= sqr(q+1)) inRange[i][j][q].push_back(c);
                    // else break;
                }
        forn(q, 5)
        sort(inRange[i][j][q].begin(), inRange[i][j][q].end(), cmpCell);
    }
    
    forn(i, n)
    forn(j, n)
    if (ma[i][j] == '#')
        if (!inRange[i][j][4].empty())
            buildingCells.emplace_back(i, j);
    
    /*double minVal = 1e9;
     for (const Tower& t: towers) {
     double curVal = double(t.C) / creepMoney * startingCreepHealth / t.D;
     DBG(curVal);
     minVal = min(minVal, curVal);
     }*/
    
    estimateDefendingTowers();
    
    // willDefend = VI(baseN, 1);
    /*if (minVal > 44) {
     int bestBase = -1, bestBaseDng = 1e9;
     forn(b, baseN) {
     int sumBaseDng = 0;
     forn(q, 4) {
     int ni = basePos[b].first + di[q],
     nj = basePos[b].second + dj[q];
     sumBaseDng += baseDng[ni][nj];
     }
     if (sumBaseDng < bestBaseDng) {
     bestBaseDng = sumBaseDng;
     bestBase = b;
     }
     }
     forn(b, baseN)
     willDefend[b] = (b == bestBase);
     }*/
    DBG(willDefend);
    
    towersMap.setSize(n);
}

void fillTowers(const VI& towerTypes) {
    vector<Tower> ptowers;
    for (size_t i = 0; i < towerTypes.size(); i += 3) {
        ptowers.push_back(Tower{towerTypes[i],
            towerTypes[i+1],
            towerTypes[i+2],
            i / 3});
    }
    DBG(ptowers);
    
    vector<bool> bad(ptowers.size(), false);
    forn(i, ptowers.size())
    forn(j, ptowers.size()) {
        if (ptowers[j].R >= ptowers[i].R &&
            ptowers[j].D >= ptowers[i].D &&
            ptowers[j].C <= ptowers[i].C)
            if (i < j || ptowers[j] != ptowers[i]) {
                bad[i] = true;
                break;
            }
    }
    
    towers.clear();
    forn(i, ptowers.size())
    if (!bad[i])
        towers.push_back(ptowers[i]);
}

int PathDefense::init(VS board, int money, int creepHealth, int creepMoney, VI towerTypes) {
    ma = board;
    n = ma.size();
    startingCreepHealth = creepHealth;
    ::creepMoney = creepMoney;
    initMoney = money;
    DBG(money);
    DBG(creepMoney);
    DBG(creepHealth);
    
    fillTowers(towerTypes);
    DBG(towers);
    usedTowers.assign(towerTypes.size() / 3, 0);
    preprocess();
    turn = 0;
    
    return 0;
}




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
