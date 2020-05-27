#pragma GCC optimize ("O3")
#pragma GCC optimize ("unroll-loops")
#pragma GCC target ("avx")

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <utility>
#include <string>
#include <queue>
#include <stack>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <random>
#include <cmath>
#include <cassert>

#include <x86intrin.h>

struct xorshift64 {
  unsigned long long int x = 88172645463325252ULL;
  inline unsigned short nextUShort() {
    x = x ^ (x << 7);
    return x = x ^ (x >> 9);
  }
  inline unsigned short nextUShortMod(unsigned long long int mod) {
    x = x ^ (x << 7);
    x = x ^ (x >> 9);
    return ((x & 0x0000ffffffffffff) * mod) >> 48;
  }
  inline unsigned int nextUInt() {
    x = x ^ (x << 7);
    return x = x ^ (x >> 9);
  }
  inline unsigned int nextUIntMod(unsigned long long int mod) {
    x = x ^ (x << 7);
    x = x ^ (x >> 9);
    return ((x & 0x00000000ffffffff) * mod) >> 32;
  }
  inline unsigned long long int nextULL() {
    x = x ^ (x << 7);
    return x = x ^ (x >> 9);
  }
  inline double nextDouble() {
    x = x ^ (x << 7);
    x = x ^ (x >> 9);
    return (double)x * 5.42101086242752217e-20;
  }
};

struct timer {
  double t = 0.0;
  double lastStop = 0.0;
  bool stopped = false;
  timer() {
    restart();
  }
  inline void restart() {
    t = now();
    stopped = false;
  }
  inline void start() {
    if (stopped) {
      t += now() - lastStop;
      stopped = false;
    }
  }
  inline void stop() {
    if (!stopped) {
      lastStop = now();
      stopped = true;
    }
  }
  inline double time() {
    return now() - t;
  }
  inline double now() {
    unsigned long long l, h;
    __asm__ ("rdtsc" : "=a"(l), "=d"(h));
    #ifdef LOCAL
    return (double)(l | h << 32) * 2.857142857142857e-10; // 1 / 3.5e9, for local (Ryzen 9 3950X)
    #else
    return (double)(l | h << 32) * 3.5714285714285715e-10; // 1 / 2.8e9, for AWS EC2 C3 (Xeon E5-2680 v2)
    //return (double)(l | h << 32) * 3.4482758620689656e-10; // 1 / 2.9e9, for AWS EC2 C4 (Xeon E5-2666 v3)
    //return (double)(l | h << 32) * 3.333333333333333e-10; // 1 / 3.0e9, for AWS EC2 C5 (Xeon Platinum 8124M / Xeon Platinum 8275CL)
    #endif
  }
};

using namespace std;

typedef long long int ll;
typedef pair<int, int> Pii;

const ll mod = 1000000007;

struct unionfind {
  vector<int> group;
  vector<int> rank;

  unionfind() {
    unionfind(0);
  }

  unionfind(int n) {
    group = vector<int>(n);
    rank = vector<int>(n);
    for (int i = 0; i < n; i++) group[i] = i;
  }

  int root(int x) {
    if (group[x] == x) return x;
    return group[x] = root(group[x]);
  }

  void unite(int x, int y) {
    int rx = root(x);
    int ry = root(y);
    if (rank[rx] > rank[ry]) group[ry] = rx;
    else if (rank[ry] > rank[rx]) group[rx] = ry;
    else if (rx != ry) {
      group[ry] = rx;
      rank[rx]++;
    }
  }

  bool same(int x, int y) {
    int rx = root(x);
    int ry = root(y);
    return rx == ry;
  }
};

timer theTimer;
xorshift64 theRandom;
mt19937 theMersenne(1);

//#define DEBUG

int n, c, d, s;
vector<vector<vector<int> > > colorPalette;
vector<int> p;
vector<vector<vector<int> > > choreography;

vector<vector<int> > marginUsed;
vector<vector<int> > marginLimit;

vector<vector<vector<char> > > movement;
vector<vector<int> > flipCount;
unionfind colorGroup;
vector<unordered_map<int, int> > colorCount;

int targetColor = -1;

ll calcScore() {
  flipCount = vector<vector<int> >(n, vector<int>(n));
  ll score = 0;
  for (int i = 0; i < d; i++) {
    int px = choreography[i][0][0];
    int py = choreography[i][0][1];
    for (auto &x: movement[i]) {
      for (auto &y: x) {
        if (y == 'L') px--;
        else if (y == 'R') px++;
        else if (y == 'U') py--;
        else if (y == 'D') py++;
        else continue;
        if (px < 0 || px >= n || py < 0 || py >= n) score = (ll) 9e18; // out of bound
        else flipCount[py][px]++;
      }
    }
  }
  colorGroup = unionfind(n*n);
  for (int i = 0; i < n-1; i++) {
    for (int j = 0; j < n; j++) {
      if (colorPalette[i][j][flipCount[i][j]%c] == colorPalette[i+1][j][flipCount[i+1][j]%c]) colorGroup.unite(i*n+j, (i+1)*n+j);
    }
  }
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n-1; j++) {
      if (colorPalette[i][j][flipCount[i][j]%c] == colorPalette[i][j+1][flipCount[i][j+1]%c]) colorGroup.unite(i*n+j, i*n+(j+1));
    }
  }
  colorCount = vector<unordered_map<int, int> >(c);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      colorCount[colorPalette[i][j][flipCount[i][j]%c]][colorGroup.root(i*n+j)]++;
    }
  }
  for (int i = 0; i < c; i++) {
    score += 1000LL * colorCount[i].size() * colorCount[i].size();
    for (auto &x: colorCount[i]) {
      if (n <= 25) {
        score -= x.second * x.second;
        if (i == targetColor) score += x.second * x.second;
      }
      else score += x.second * x.second;
    }
  }
  return score;
}

ll calcScorePartial(int r, int t, vector<char> &movementBefore) {
  ll score = 0;
  int px = choreography[r][t][0];
  int py = choreography[r][t][1];
  for (auto &y: movementBefore) {
    if (y == 'L') px--;
    else if (y == 'R') px++;
    else if (y == 'U') py--;
    else if (y == 'D') py++;
    else continue;
    if (px < 0 || px >= n || py < 0 || py >= n) continue; // out of bound
    else flipCount[py][px]--;
  }
  px = choreography[r][t][0];
  py = choreography[r][t][1];
  for (auto &y: movement[r][t]) {
    if (y == 'L') px--;
    else if (y == 'R') px++;
    else if (y == 'U') py--;
    else if (y == 'D') py++;
    else continue;
    if (px < 0 || px >= n || py < 0 || py >= n) score = (ll) 9e18; // out of bound
    else flipCount[py][px]++;
  }
  colorGroup = unionfind(n*n);
  for (int i = 0; i < n-1; i++) {
    for (int j = 0; j < n; j++) {
      if (colorPalette[i][j][flipCount[i][j]%c] == colorPalette[i+1][j][flipCount[i+1][j]%c]) colorGroup.unite(i*n+j, (i+1)*n+j);
    }
  }
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n-1; j++) {
      if (colorPalette[i][j][flipCount[i][j]%c] == colorPalette[i][j+1][flipCount[i][j+1]%c]) colorGroup.unite(i*n+j, i*n+(j+1));
    }
  }
  colorCount = vector<unordered_map<int, int> >(c);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      colorCount[colorPalette[i][j][flipCount[i][j]%c]][colorGroup.root(i*n+j)]++;
    }
  }
  for (int i = 0; i < c; i++) {
    score += 1000LL * colorCount[i].size() * colorCount[i].size();
    for (auto &x: colorCount[i]) {
      if (n <= 25) {
        score -= x.second * x.second;
        if (i == targetColor) score += x.second * x.second;
      }
      else score += x.second * x.second;
    }
  }
  return score;
}

void revertMovement(int r, int t, vector<char> &movementBefore) {
  int px = choreography[r][t][0];
  int py = choreography[r][t][1];
  for (auto &y: movementBefore) {
    if (y == 'L') px--;
    else if (y == 'R') px++;
    else if (y == 'U') py--;
    else if (y == 'D') py++;
    else continue;
    if (px < 0 || px >= n || py < 0 || py >= n) continue; // out of bound
    else flipCount[py][px]++;
  }
  px = choreography[r][t][0];
  py = choreography[r][t][1];
  for (auto &y: movement[r][t]) {
    if (y == 'L') px--;
    else if (y == 'R') px++;
    else if (y == 'U') py--;
    else if (y == 'D') py++;
    else continue;
    if (px < 0 || px >= n || py < 0 || py >= n) continue; // out of bound
    else flipCount[py][px]--;
  }
}

int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);

  // input
  cin >> n >> c >> d >> s;
  colorPalette = vector<vector<vector<int> > >(n, vector<vector<int> >(n, vector<int>(c)));
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      for (int k = 0; k < c; k++) {
        char t;
        cin >> t;
        colorPalette[i][j][k] = t - '0';
      }
    }
  }
  p = vector<int>(d);
  choreography = vector<vector<vector<int> > >(d);
  for (int i = 0; i < d; i++) {
    cin >> p[i];
    choreography[i] = vector<vector<int> >(p[i], vector<int>(3));
    for (auto &x: choreography[i]) {
      for (auto &y: x) cin >> y;
    }
  }

  // preprocess
  movement = vector<vector<vector<char> > >(d);
  for (int i = 0; i < d; i++) {
    movement[i] = vector<vector<char> >(p[i]-1);
    for (int j = 0; j < p[i]-1; j++) {
      movement[i][j] = vector<char>(choreography[i][j+1][2] - choreography[i][j][2], '-');
      int px = choreography[i][j][0];
      int py = choreography[i][j][1];
      int t = 0;
      while (px != choreography[i][j+1][0] || py != choreography[i][j+1][1]) {
        if (px < choreography[i][j+1][0]) {
          movement[i][j][t] = 'R';
          px++;
        }
        else if (px > choreography[i][j+1][0]) {
          movement[i][j][t] = 'L';
          px--;
        }
        else if (py < choreography[i][j+1][1]) {
          movement[i][j][t] = 'D';
          py++;
        }
        else if (py > choreography[i][j+1][1]) {
          movement[i][j][t] = 'U';
          py--;
        }
        t++;
      }
      shuffle(movement[i][j].begin(), movement[i][j].begin() + abs(choreography[i][j+1][0] - choreography[i][j][0]) + abs(choreography[i][j+1][1] - choreography[i][j][1]), theMersenne);
    }
  }
  marginUsed = vector<vector<int> >(d);
  marginLimit = vector<vector<int> >(d);
  for (int i = 0; i < d; i++) {
    marginUsed[i] = vector<int>(p[i]-1);
    marginLimit[i] = vector<int>(p[i]-1);
    for (int j = 0; j < p[i]-1; j++) marginLimit[i][j] = ((choreography[i][j+1][2] - choreography[i][j][2]) - (abs(choreography[i][j+1][0] - choreography[i][j][0]) + abs(choreography[i][j+1][1] - choreography[i][j][1]))) / 2;
  }
  if (c == 2 && n % 2 == 1) {
    vector<int> firstColorCount(2);
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        firstColorCount[colorPalette[i][j][0]]++;
      }
    }
    if (firstColorCount[0] % 2 == 1) targetColor = 0;
    else targetColor = 1;
  }

  // optimization
  ll score = calcScore();
  ll lastScore = score;
  ll bestScore = score;

  double baseTemperature = 1e4;
  double temperature = baseTemperature;
  double decayRate = 2e-5;
  double timeLimit = 9.900;
  int iterCount = 0;

  double thresholdChangeTime = 1.000;
  double insertExtraMoveRollThreshold = 0.0;
  double removeExtraMoveRollThreshold = 0.0;

  while (theTimer.time() < timeLimit) {
    if (theTimer.time() > thresholdChangeTime) {
      insertExtraMoveRollThreshold = 0.2;
      removeExtraMoveRollThreshold = 0.4;
    }
    double roll = theRandom.nextDouble();
    if (roll < insertExtraMoveRollThreshold) {
      int r = theRandom.nextUIntMod(d);
      int t = theRandom.nextUIntMod(p[r]-1);
      if (marginUsed[r][t] == marginLimit[r][t]) continue;
      int u = theRandom.nextUIntMod(abs(choreography[r][t+1][0] - choreography[r][t][0]) + abs(choreography[r][t+1][1] - choreography[r][t][1]) + marginUsed[r][t] * 2 + 1);
      int v = theRandom.nextUIntMod(4);

      auto movementBefore = movement[r][t];
      if (v == 0) {
        movement[r][t].insert(movement[r][t].begin() + u, 'U');
        movement[r][t].insert(movement[r][t].begin() + u, 'D');
      }
      else if (v == 1) {
        movement[r][t].insert(movement[r][t].begin() + u, 'D');
        movement[r][t].insert(movement[r][t].begin() + u, 'U');
      }
      else if (v == 2) {
        movement[r][t].insert(movement[r][t].begin() + u, 'L');
        movement[r][t].insert(movement[r][t].begin() + u, 'R');
      }
      else {
        movement[r][t].insert(movement[r][t].begin() + u, 'R');
        movement[r][t].insert(movement[r][t].begin() + u, 'L');
      }
      movement[r][t].pop_back();
      movement[r][t].pop_back();

      score = calcScorePartial(r, t, movementBefore);

      #ifdef DEBUG
      if (iterCount % 10000 == 0) cerr << iterCount << " " << score << " " << lastScore << " " << bestScore << " " << temperature << endl;
      #endif

      if (score <= lastScore) {
        lastScore = score;
        marginUsed[r][t]++;
        if (score < bestScore) {
          bestScore = score;
        }
      }
      else if (theRandom.nextDouble() < exp(double(lastScore - score) / temperature)) { // accept
        lastScore = score;
        marginUsed[r][t]++;
      }
      else { // rollback
        revertMovement(r, t, movementBefore);
        movement[r][t] = movementBefore;
        score = lastScore;
      }
    }
    else if (roll < removeExtraMoveRollThreshold) {
      int r = theRandom.nextUIntMod(d);
      int t = theRandom.nextUIntMod(p[r]-1);
      int u1 = theRandom.nextUIntMod(choreography[r][t+1][2] - choreography[r][t][2]);
      int u2 = theRandom.nextUIntMod(choreography[r][t+1][2] - choreography[r][t][2]);
      if (!((movement[r][t][u1] == 'U' && movement[r][t][u2] == 'D') || (movement[r][t][u1] == 'D' && movement[r][t][u2] == 'U') || (movement[r][t][u1] == 'L' && movement[r][t][u2] == 'R') || (movement[r][t][u1] == 'R' && movement[r][t][u2] == 'L'))) continue;

      if (u1 > u2) swap(u1, u2);
      auto movementBefore = movement[r][t];
      movement[r][t].erase(movement[r][t].begin() + u2);
      movement[r][t].erase(movement[r][t].begin() + u1);
      movement[r][t].push_back('-');
      movement[r][t].push_back('-');

      score = calcScorePartial(r, t, movementBefore);

      #ifdef DEBUG
      if (iterCount % 10000 == 0) cerr << iterCount << " " << score << " " << lastScore << " " << bestScore << " " << temperature << endl;
      #endif

      if (score <= lastScore) {
        lastScore = score;
        marginUsed[r][t]--;
        if (score < bestScore) {
          bestScore = score;
        }
      }
      else if (theRandom.nextDouble() < exp(double(lastScore - score) / temperature)) { // accept
        lastScore = score;
        marginUsed[r][t]--;
      }
      else { // rollback
        revertMovement(r, t, movementBefore);
        movement[r][t] = movementBefore;
        score = lastScore;
      }
    }
    else {
      int r = theRandom.nextUIntMod(d);
      int t = theRandom.nextUIntMod(p[r]-1);
      if (abs(choreography[r][t+1][0] - choreography[r][t][0]) + abs(choreography[r][t+1][1] - choreography[r][t][1]) + marginUsed[r][t] * 2 < 2) continue;
      int u = theRandom.nextUIntMod(abs(choreography[r][t+1][0] - choreography[r][t][0]) + abs(choreography[r][t+1][1] - choreography[r][t][1]) + marginUsed[r][t] * 2 - 1);
      if (movement[r][t][u] == movement[r][t][u+1]) continue;

      auto movementBefore = movement[r][t];
      swap(movement[r][t][u], movement[r][t][u+1]);

      score = calcScorePartial(r, t, movementBefore);

      #ifdef DEBUG
      if (iterCount % 10000 == 0) cerr << iterCount << " " << score << " " << lastScore << " " << bestScore << " " << temperature << endl;
      #endif

      if (score <= lastScore) {
        lastScore = score;
        if (score < bestScore) {
          bestScore = score;
        }
      }
      else if (theRandom.nextDouble() < exp(double(lastScore - score) / temperature)) { // accept
        lastScore = score;
      }
      else { // rollback
        revertMovement(r, t, movementBefore);
        movement[r][t] = movementBefore;
        score = lastScore;
      }
    }

    iterCount++;
    temperature *= 1.0 - decayRate;
  }

  cerr << "iterCount   = " << iterCount << endl;
  cerr << "temperature = " << temperature << endl;
  cerr << "bestScore   = " << bestScore << endl;

  // postprocess & output
  vector<vector<char> > ans(s, vector<char>(d));
  for (int i = 0; i < d; i++) {
    int t = 0;
    for (auto &x: movement[i]) {
      for (auto &y: x) {
        ans[t][i] = y;
        t++;
      }
    }
  }

  cout << s << endl;
  for (auto &x: ans) {
    for (auto &y: x) cout << y;
    cout << endl;
  }

  return 0;
}
