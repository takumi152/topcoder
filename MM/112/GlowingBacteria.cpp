#pragma GCC optimize ("O3")
#pragma GCC optimize ("unroll-loops")
#pragma GCC target ("sse4.2")

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <set>
#include <map>
#include <list>
#include <queue>
#include <functional>
#include <numeric>
#include <random>
#include <chrono>
#include <cstring>
#include <climits>
#include <cassert>

#include <x86intrin.h>

using namespace std;

typedef long long int ll;

struct xorshift64 {
  unsigned long long int x = 88172645463325252ULL;
  inline unsigned short nextUShort() {
    x = x ^ (x << 7);
    return x = x ^ (x >> 9);
  }
  inline unsigned int nextUShortMod(unsigned long long int mod) {
    x = x ^ (x << 7);
    x = x ^ (x >> 9);
    return ((x & 0x000000000000ffff) * mod) >> 48;
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
  timer() {
    restart();
  }
  inline void restart() {
    t = now();
  }
  inline double time() {
    return now() - t;
  }
  inline double now() {
    unsigned long long l, h;
    __asm__ ("rdtsc" : "=a"(l), "=d"(h));
    return (double)(l | h << 32) * 3.5714285714285715e-10; // 1 / 2.8e9
  }
};

//#define DEBUG

xorshift64 theRandom;
timer theTimer;

int n, c, d, k;
vector<vector<char> > target;

vector<vector<char> > colorAfter;
vector<vector<vector<char> > > colorHistogram;

vector<vector<char> > predicted;
vector<vector<char> > lastPredicted;
vector<vector<char> > bestPredicted;

vector<vector<int> > generatorSequence;

vector<vector<vector<pair<int, int> > > > neighbour;

void findNeighbour() {
  neighbour = vector<vector<vector<pair<int, int> > > >(n, vector<vector<pair<int, int> > >(n));
  for (int x = 0; x < n; x++) {
    for (int y = 0; y < n; y++) {
      for (int px = 0; px < n; px++) {
        for (int py = 0; py < n; py++) {
          int e = (x - px) * (x - px) + (y - py) * (y - py);
          if (e > 0 && e <= k) {
            neighbour[x][y].emplace_back(px, py);
          }
        }
      }
    }
  }
}

int recalculateHistogram() {
  colorAfter = predicted;
  colorHistogram = vector<vector<vector<char> > >(n, vector<vector<char> >(n, vector<char>(c, 0)));
  int score = 0;
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      for (auto &p: neighbour[i][j]) {
        colorHistogram[i][j][predicted[p.first][p.second]]++;
      }
      int mindex = 0;
      char minval = 0x7f;
      bool unclear = false;
      for (int k = 0; k < c; k++) {
        if (colorHistogram[i][j][k] == 0) continue;
        else if (colorHistogram[i][j][k] < minval) {
          mindex = k;
          minval = colorHistogram[i][j][k];
          unclear = false;
        }
        else if (colorHistogram[i][j][k] == minval) {
          unclear = true;
        }
      }
      if (unclear) mindex = predicted[i][j];
      colorAfter[i][j] = mindex;
      if (colorAfter[i][j] == target[i][j]) score++;
    }
  }
  return score;
}

int switchColor(int x, int y, char nc, int scoreNow) {
  int score = scoreNow;
  char oc = predicted[x][y];
  for (auto &p: neighbour[x][y]) {
    colorHistogram[p.first][p.second][oc]--;
    colorHistogram[p.first][p.second][nc]++;
    int mindex = 0;
    char minval = 0x7f;
    bool unclear = false;
    for (int k = 0; k < c; k++) {
      if (colorHistogram[p.first][p.second][k] == 0) continue;
      else if (colorHistogram[p.first][p.second][k] < minval) {
        mindex = k;
        minval = colorHistogram[p.first][p.second][k];
        unclear = false;
      }
      else if (colorHistogram[p.first][p.second][k] == minval) {
        unclear = true;
      }
    }
    if (unclear) mindex = predicted[p.first][p.second];
    if (colorAfter[p.first][p.second] == target[p.first][p.second]) score--;
    colorAfter[p.first][p.second] = mindex;
    if (colorAfter[p.first][p.second] == target[p.first][p.second]) score++;
  }
  predicted[x][y] = nc;
  int mindex = 0;
  char minval = 0x7f;
  bool unclear = false;
  for (int k = 0; k < c; k++) {
    if (colorHistogram[x][y][k] == 0) continue;
    else if (colorHistogram[x][y][k] < minval) {
      mindex = k;
      minval = colorHistogram[x][y][k];
      unclear = false;
    }
    else if (colorHistogram[x][y][k] == minval) {
      unclear = true;
    }
  }
  if (unclear) mindex = predicted[x][y];
  if (colorAfter[x][y] == target[x][y]) score--;
  colorAfter[x][y] = mindex;
  if (colorAfter[x][y] == target[x][y]) score++;
  return score;
}

int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);

  cin >> n;
  cin >> c;
  cin >> d;
  cin >> k;

  target = vector<vector<char> >(n, vector<char>(n));
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      cin >> target[i][j];
    }
  }

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      target[i][j] -= '0';
    }
  }

  findNeighbour();

  predicted = target;
  lastPredicted = predicted;
  bestPredicted = predicted;

  int score = recalculateHistogram();
  int lastScore = score;
  int bestScore = score;

  double baseTemperature = 0.3;
  double temperature = 0.3;

  int iterCount = 0;

  constexpr double timeLimit = 9.100;
  while (theTimer.time() < timeLimit) {
    int x = theRandom.nextUIntMod(n);
    int y = theRandom.nextUIntMod(n);
    char nc = theRandom.nextUIntMod(c);
    char oc = predicted[x][y];
    if (oc == nc) continue;

    score = switchColor(x, y, nc, score);
    if (iterCount % 100000 == 0) cerr << iterCount << " " << score << " " << lastScore << " " << bestScore << " " << temperature << " " << endl;

    if (score >= lastScore) {
      lastScore = score;
      if (score > bestScore) {
        bestPredicted = predicted;
        bestScore = score;
      }
    }
    else if (theRandom.nextDouble() < exp(double(score - lastScore) / temperature)) {
      lastScore = score;
    }
    else {
      score = switchColor(x, y, oc, score);
    }

    iterCount++;
    temperature = baseTemperature * ((timeLimit - theTimer.time()) / timeLimit);
  }

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      bestPredicted[i][j] += '0';
    }
  }

  cout << n * n << endl;
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      cout << bestPredicted[i][j] << endl;
    }
  }

  cerr << "iterCount   = " << iterCount << endl;
  cerr << "temperature = " << temperature << endl;
  cerr << "bestScore   = " << bestScore << endl;

  return 0;
}
