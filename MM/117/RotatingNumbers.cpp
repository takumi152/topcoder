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

timer theTimer;
xorshift64 theRandom;

//#define DEBUG

int n, p;
vector<vector<short> > grid;

vector<vector<short> > grid_after;
vector<pair<short, short> > after_position;
vector<pair<short, short> > target_position;
deque<vector<short> > operation;
deque<vector<short> > best_operation;

vector<int> penaltyTable = {0, 0, 1, 2, 5, 8, 11, 14, 18, 22, 27, 31, 36, 41, 46, 52, 58, 64, 70, 76, 82, 89, 96, 103, 110, 117, 125, 132, 140, 148, 156, 164, 172, 181, 189, 198, 207, 216, 225, 234, 243};

vector<int> greedyIncLimit = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 11, 9, 8, 7, 6, 5, 4, 4, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1};


int calcScore() {
  int score = 0;
  for (int i = 0; i < n*n; i++) {
    score += abs(after_position[i].first - target_position[i].first) + abs(after_position[i].second - target_position[i].second);
  }
  return score * p;
}

int calcPenalty() {
  int penalty = 0;
  for (auto &x: operation) penalty += penaltyTable[x[0]];
  return penalty;
}

int calcScoreDeltaLast(short s, short r, short c, char dir) {
  int scoreDelta = 0;
  if (dir == 'L') {
    for (int i = 0; i < s / 2; i++) {
      for (int j = i; j < s - i - 1; j++) {
        scoreDelta -= (abs((r+i)     - target_position[grid_after[r+i][c+j]].first)             + abs((c+j)     - target_position[grid_after[r+i][c+j]].second))
                    - (abs((r+s-j-1) - target_position[grid_after[r+i][c+j]].first)             + abs((c+i)     - target_position[grid_after[r+i][c+j]].second))
                    + (abs((r+s-j-1) - target_position[grid_after[r+s-j-1][c+i]].first)         + abs((c+i)     - target_position[grid_after[r+s-j-1][c+i]].second))
                    - (abs((r+s-i-1) - target_position[grid_after[r+s-j-1][c+i]].first)         + abs((c+s-j-1) - target_position[grid_after[r+s-j-1][c+i]].second))
                    + (abs((r+s-i-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].first)     + abs((c+s-j-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].second))
                    - (abs((r+j)     - target_position[grid_after[r+s-i-1][c+s-j-1]].first)     + abs((c+s-i-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].second))
                    + (abs((r+j)     - target_position[grid_after[r+j][c+s-i-1]].first)         + abs((c+s-i-1) - target_position[grid_after[r+j][c+s-i-1]].second))
                    - (abs((r+i)     - target_position[grid_after[r+j][c+s-i-1]].first)         + abs((c+j)     - target_position[grid_after[r+j][c+s-i-1]].second));
      }
    }
  }
  else if (dir == 'R') {
    for (int i = 0; i < s / 2; i++) {
      for (int j = i; j < s - i - 1; j++) {
        scoreDelta -= (abs((r+i)     - target_position[grid_after[r+i][c+j]].first)             + abs((c+j)     - target_position[grid_after[r+i][c+j]].second))
                    - (abs((r+j)     - target_position[grid_after[r+i][c+j]].first)             + abs((c+s-i-1) - target_position[grid_after[r+i][c+j]].second))
                    + (abs((r+j)     - target_position[grid_after[r+j][c+s-i-1]].first)         + abs((c+s-i-1) - target_position[grid_after[r+j][c+s-i-1]].second))
                    - (abs((r+s-i-1) - target_position[grid_after[r+j][c+s-i-1]].first)         + abs((c+s-j-1) - target_position[grid_after[r+j][c+s-i-1]].second))
                    + (abs((r+s-i-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].first)     + abs((c+s-j-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].second))
                    - (abs((r+s-j-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].first)     + abs((c+i)     - target_position[grid_after[r+s-i-1][c+s-j-1]].second))
                    + (abs((r+s-j-1) - target_position[grid_after[r+s-j-1][c+i]].first)         + abs((c+i)     - target_position[grid_after[r+s-j-1][c+i]].second))
                    - (abs((r+i)     - target_position[grid_after[r+s-j-1][c+i]].first)         + abs((c+j)     - target_position[grid_after[r+s-j-1][c+i]].second));
      }
    }
  }
  else {
    for (int i = 0; i < s / 2; i++) {
      for (int j = i; j < s - i - 1; j++) {
        scoreDelta -= (abs((r+i)     - target_position[grid_after[r+i][c+j]].first)             + abs((c+j)     - target_position[grid_after[r+i][c+j]].second))
                    - (abs((r+i)     - target_position[grid_after[r+s-i-1][c+s-j-1]].first)     + abs((c+j)     - target_position[grid_after[r+s-i-1][c+s-j-1]].second))
                    + (abs((r+s-i-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].first)     + abs((c+s-j-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].second))
                    - (abs((r+s-i-1) - target_position[grid_after[r+i][c+j]].first)             + abs((c+s-j-1) - target_position[grid_after[r+i][c+j]].second))
                    + (abs((r+j)     - target_position[grid_after[r+j][c+s-i-1]].first)         + abs((c+s-i-1) - target_position[grid_after[r+j][c+s-i-1]].second))
                    - (abs((r+s-j-1) - target_position[grid_after[r+j][c+s-i-1]].first)         + abs((c+i)     - target_position[grid_after[r+j][c+s-i-1]].second))
                    + (abs((r+s-j-1) - target_position[grid_after[r+s-j-1][c+i]].first)         + abs((c+i)     - target_position[grid_after[r+s-j-1][c+i]].second))
                    - (abs((r+j)     - target_position[grid_after[r+s-j-1][c+i]].first)         + abs((c+s-i-1) - target_position[grid_after[r+s-j-1][c+i]].second));
      }
    }
  }
  return scoreDelta * p;
}

int calcScoreDeltaFirst(short s, short r, short c, char dir) {
  int scoreDelta = 0;
  if (dir == 'L') {
    for (int i = 0; i < s / 2; i++) {
      for (int j = i; j < s - i - 1; j++) {
        scoreDelta -= (abs(after_position[grid[r+i][c+j]].first         - target_position[grid[r+i][c+j]].first)         + abs(after_position[grid[r+i][c+j]].second         - target_position[grid[r+i][c+j]].second))
                    - (abs(after_position[grid[r+s-j-1][c+i]].first     - target_position[grid[r+i][c+j]].first)         + abs(after_position[grid[r+s-j-1][c+i]].second     - target_position[grid[r+i][c+j]].second))
                    + (abs(after_position[grid[r+s-j-1][c+i]].first     - target_position[grid[r+s-j-1][c+i]].first)     + abs(after_position[grid[r+s-j-1][c+i]].second     - target_position[grid[r+s-j-1][c+i]].second))
                    - (abs(after_position[grid[r+s-i-1][c+s-j-1]].first - target_position[grid[r+s-j-1][c+i]].first)     + abs(after_position[grid[r+s-i-1][c+s-j-1]].second - target_position[grid[r+s-j-1][c+i]].second))
                    + (abs(after_position[grid[r+s-i-1][c+s-j-1]].first - target_position[grid[r+s-i-1][c+s-j-1]].first) + abs(after_position[grid[r+s-i-1][c+s-j-1]].second - target_position[grid[r+s-i-1][c+s-j-1]].second))
                    - (abs(after_position[grid[r+j][c+s-i-1]].first     - target_position[grid[r+s-i-1][c+s-j-1]].first) + abs(after_position[grid[r+j][c+s-i-1]].second     - target_position[grid[r+s-i-1][c+s-j-1]].second))
                    + (abs(after_position[grid[r+j][c+s-i-1]].first     - target_position[grid[r+j][c+s-i-1]].first)     + abs(after_position[grid[r+j][c+s-i-1]].second     - target_position[grid[r+j][c+s-i-1]].second))
                    - (abs(after_position[grid[r+i][c+j]].first         - target_position[grid[r+j][c+s-i-1]].first)     + abs(after_position[grid[r+i][c+j]].second         - target_position[grid[r+j][c+s-i-1]].second));
      }
    }
  }
  else if (dir == 'R') {
    for (int i = 0; i < s / 2; i++) {
      for (int j = i; j < s - i - 1; j++) {
        scoreDelta -= (abs(after_position[grid[r+i][c+j]].first         - target_position[grid[r+i][c+j]].first)         + abs(after_position[grid[r+i][c+j]].second         - target_position[grid[r+i][c+j]].second))
                    - (abs(after_position[grid[r+j][c+s-i-1]].first     - target_position[grid[r+i][c+j]].first)         + abs(after_position[grid[r+j][c+s-i-1]].second     - target_position[grid[r+i][c+j]].second))
                    + (abs(after_position[grid[r+j][c+s-i-1]].first     - target_position[grid[r+j][c+s-i-1]].first)     + abs(after_position[grid[r+j][c+s-i-1]].second     - target_position[grid[r+j][c+s-i-1]].second))
                    - (abs(after_position[grid[r+s-i-1][c+s-j-1]].first - target_position[grid[r+j][c+s-i-1]].first)     + abs(after_position[grid[r+s-i-1][c+s-j-1]].second - target_position[grid[r+j][c+s-i-1]].second))
                    + (abs(after_position[grid[r+s-i-1][c+s-j-1]].first - target_position[grid[r+s-i-1][c+s-j-1]].first) + abs(after_position[grid[r+s-i-1][c+s-j-1]].second - target_position[grid[r+s-i-1][c+s-j-1]].second))
                    - (abs(after_position[grid[r+s-j-1][c+i]].first     - target_position[grid[r+s-i-1][c+s-j-1]].first) + abs(after_position[grid[r+s-j-1][c+i]].second     - target_position[grid[r+s-i-1][c+s-j-1]].second))
                    + (abs(after_position[grid[r+s-j-1][c+i]].first     - target_position[grid[r+s-j-1][c+i]].first)     + abs(after_position[grid[r+s-j-1][c+i]].second     - target_position[grid[r+s-j-1][c+i]].second))
                    - (abs(after_position[grid[r+i][c+j]].first         - target_position[grid[r+s-j-1][c+i]].first)     + abs(after_position[grid[r+i][c+j]].second         - target_position[grid[r+s-j-1][c+i]].second));
      }
    }
  }
  else {
    for (int i = 0; i < s / 2; i++) {
      for (int j = i; j < s - i - 1; j++) {
        scoreDelta -= (abs(after_position[grid[r+i][c+j]].first         - target_position[grid[r+i][c+j]].first)             + abs(after_position[grid[r+i][c+j]].second         - target_position[grid[r+i][c+j]].second))
                    - (abs(after_position[grid[r+i][c+j]].first         - target_position[grid[r+s-i-1][c+s-j-1]].first)     + abs(after_position[grid[r+i][c+j]].second         - target_position[grid[r+s-i-1][c+s-j-1]].second))
                    + (abs(after_position[grid[r+s-i-1][c+s-j-1]].first - target_position[grid[r+s-i-1][c+s-j-1]].first)     + abs(after_position[grid[r+s-i-1][c+s-j-1]].second - target_position[grid[r+s-i-1][c+s-j-1]].second))
                    - (abs(after_position[grid[r+s-i-1][c+s-j-1]].first - target_position[grid[r+i][c+j]].first)             + abs(after_position[grid[r+s-i-1][c+s-j-1]].second - target_position[grid[r+i][c+j]].second))
                    + (abs(after_position[grid[r+j][c+s-i-1]].first     - target_position[grid[r+j][c+s-i-1]].first)         + abs(after_position[grid[r+j][c+s-i-1]].second     - target_position[grid[r+j][c+s-i-1]].second))
                    - (abs(after_position[grid[r+s-j-1][c+i]].first     - target_position[grid[r+j][c+s-i-1]].first)         + abs(after_position[grid[r+s-j-1][c+i]].second     - target_position[grid[r+j][c+s-i-1]].second))
                    + (abs(after_position[grid[r+s-j-1][c+i]].first     - target_position[grid[r+s-j-1][c+i]].first)         + abs(after_position[grid[r+s-j-1][c+i]].second     - target_position[grid[r+s-j-1][c+i]].second))
                    - (abs(after_position[grid[r+j][c+s-i-1]].first     - target_position[grid[r+s-j-1][c+i]].first)         + abs(after_position[grid[r+j][c+s-i-1]].second     - target_position[grid[r+s-j-1][c+i]].second));
      }
    }
  }
  return scoreDelta * p;
}

int calcScoreSquared() {
  int score = 0;
  for (int i = 0; i < n*n; i++) {
    score += (abs(after_position[i].first - target_position[i].first) + abs(after_position[i].second - target_position[i].second)) * (abs(after_position[i].first - target_position[i].first) + abs(after_position[i].second - target_position[i].second));
  }
  return score * p;
}

int calcScoreDeltaLastSquared(short s, short r, short c, char dir) {
  int scoreDelta = 0;
  if (dir == 'L') {
    for (int i = 0; i < s / 2; i++) {
      for (int j = i; j < s - i - 1; j++) {
        scoreDelta -= (abs((r+i)     - target_position[grid_after[r+i][c+j]].first)             + abs((c+j)     - target_position[grid_after[r+i][c+j]].second))
                    * (abs((r+i)     - target_position[grid_after[r+i][c+j]].first)             + abs((c+j)     - target_position[grid_after[r+i][c+j]].second))
                    - (abs((r+s-j-1) - target_position[grid_after[r+i][c+j]].first)             + abs((c+i)     - target_position[grid_after[r+i][c+j]].second))
                    * (abs((r+s-j-1) - target_position[grid_after[r+i][c+j]].first)             + abs((c+i)     - target_position[grid_after[r+i][c+j]].second))
                    + (abs((r+s-j-1) - target_position[grid_after[r+s-j-1][c+i]].first)         + abs((c+i)     - target_position[grid_after[r+s-j-1][c+i]].second))
                    * (abs((r+s-j-1) - target_position[grid_after[r+s-j-1][c+i]].first)         + abs((c+i)     - target_position[grid_after[r+s-j-1][c+i]].second))
                    - (abs((r+s-i-1) - target_position[grid_after[r+s-j-1][c+i]].first)         + abs((c+s-j-1) - target_position[grid_after[r+s-j-1][c+i]].second))
                    * (abs((r+s-i-1) - target_position[grid_after[r+s-j-1][c+i]].first)         + abs((c+s-j-1) - target_position[grid_after[r+s-j-1][c+i]].second))
                    + (abs((r+s-i-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].first)     + abs((c+s-j-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].second))
                    * (abs((r+s-i-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].first)     + abs((c+s-j-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].second))
                    - (abs((r+j)     - target_position[grid_after[r+s-i-1][c+s-j-1]].first)     + abs((c+s-i-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].second))
                    * (abs((r+j)     - target_position[grid_after[r+s-i-1][c+s-j-1]].first)     + abs((c+s-i-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].second))
                    + (abs((r+j)     - target_position[grid_after[r+j][c+s-i-1]].first)         + abs((c+s-i-1) - target_position[grid_after[r+j][c+s-i-1]].second))
                    * (abs((r+j)     - target_position[grid_after[r+j][c+s-i-1]].first)         + abs((c+s-i-1) - target_position[grid_after[r+j][c+s-i-1]].second))
                    - (abs((r+i)     - target_position[grid_after[r+j][c+s-i-1]].first)         + abs((c+j)     - target_position[grid_after[r+j][c+s-i-1]].second))
                    * (abs((r+i)     - target_position[grid_after[r+j][c+s-i-1]].first)         + abs((c+j)     - target_position[grid_after[r+j][c+s-i-1]].second));
      }
    }
  }
  else if (dir == 'R') {
    for (int i = 0; i < s / 2; i++) {
      for (int j = i; j < s - i - 1; j++) {
        scoreDelta -= (abs((r+i)     - target_position[grid_after[r+i][c+j]].first)             + abs((c+j)     - target_position[grid_after[r+i][c+j]].second))
                    * (abs((r+i)     - target_position[grid_after[r+i][c+j]].first)             + abs((c+j)     - target_position[grid_after[r+i][c+j]].second))
                    - (abs((r+j)     - target_position[grid_after[r+i][c+j]].first)             + abs((c+s-i-1) - target_position[grid_after[r+i][c+j]].second))
                    * (abs((r+j)     - target_position[grid_after[r+i][c+j]].first)             + abs((c+s-i-1) - target_position[grid_after[r+i][c+j]].second))
                    + (abs((r+j)     - target_position[grid_after[r+j][c+s-i-1]].first)         + abs((c+s-i-1) - target_position[grid_after[r+j][c+s-i-1]].second))
                    * (abs((r+j)     - target_position[grid_after[r+j][c+s-i-1]].first)         + abs((c+s-i-1) - target_position[grid_after[r+j][c+s-i-1]].second))
                    - (abs((r+s-i-1) - target_position[grid_after[r+j][c+s-i-1]].first)         + abs((c+s-j-1) - target_position[grid_after[r+j][c+s-i-1]].second))
                    * (abs((r+s-i-1) - target_position[grid_after[r+j][c+s-i-1]].first)         + abs((c+s-j-1) - target_position[grid_after[r+j][c+s-i-1]].second))
                    + (abs((r+s-i-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].first)     + abs((c+s-j-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].second))
                    * (abs((r+s-i-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].first)     + abs((c+s-j-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].second))
                    - (abs((r+s-j-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].first)     + abs((c+i)     - target_position[grid_after[r+s-i-1][c+s-j-1]].second))
                    * (abs((r+s-j-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].first)     + abs((c+i)     - target_position[grid_after[r+s-i-1][c+s-j-1]].second))
                    + (abs((r+s-j-1) - target_position[grid_after[r+s-j-1][c+i]].first)         + abs((c+i)     - target_position[grid_after[r+s-j-1][c+i]].second))
                    * (abs((r+s-j-1) - target_position[grid_after[r+s-j-1][c+i]].first)         + abs((c+i)     - target_position[grid_after[r+s-j-1][c+i]].second))
                    - (abs((r+i)     - target_position[grid_after[r+s-j-1][c+i]].first)         + abs((c+j)     - target_position[grid_after[r+s-j-1][c+i]].second))
                    * (abs((r+i)     - target_position[grid_after[r+s-j-1][c+i]].first)         + abs((c+j)     - target_position[grid_after[r+s-j-1][c+i]].second));
      }
    }
  }
  else {
    for (int i = 0; i < s / 2; i++) {
      for (int j = i; j < s - i - 1; j++) {
        scoreDelta -= (abs((r+i)     - target_position[grid_after[r+i][c+j]].first)             + abs((c+j)     - target_position[grid_after[r+i][c+j]].second))
                    * (abs((r+i)     - target_position[grid_after[r+i][c+j]].first)             + abs((c+j)     - target_position[grid_after[r+i][c+j]].second))
                    - (abs((r+i)     - target_position[grid_after[r+s-i-1][c+s-j-1]].first)     + abs((c+j)     - target_position[grid_after[r+s-i-1][c+s-j-1]].second))
                    * (abs((r+i)     - target_position[grid_after[r+s-i-1][c+s-j-1]].first)     + abs((c+j)     - target_position[grid_after[r+s-i-1][c+s-j-1]].second))
                    + (abs((r+s-i-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].first)     + abs((c+s-j-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].second))
                    * (abs((r+s-i-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].first)     + abs((c+s-j-1) - target_position[grid_after[r+s-i-1][c+s-j-1]].second))
                    - (abs((r+s-i-1) - target_position[grid_after[r+i][c+j]].first)             + abs((c+s-j-1) - target_position[grid_after[r+i][c+j]].second))
                    * (abs((r+s-i-1) - target_position[grid_after[r+i][c+j]].first)             + abs((c+s-j-1) - target_position[grid_after[r+i][c+j]].second))
                    + (abs((r+j)     - target_position[grid_after[r+j][c+s-i-1]].first)         + abs((c+s-i-1) - target_position[grid_after[r+j][c+s-i-1]].second))
                    * (abs((r+j)     - target_position[grid_after[r+j][c+s-i-1]].first)         + abs((c+s-i-1) - target_position[grid_after[r+j][c+s-i-1]].second))
                    - (abs((r+s-j-1) - target_position[grid_after[r+j][c+s-i-1]].first)         + abs((c+i)     - target_position[grid_after[r+j][c+s-i-1]].second))
                    * (abs((r+s-j-1) - target_position[grid_after[r+j][c+s-i-1]].first)         + abs((c+i)     - target_position[grid_after[r+j][c+s-i-1]].second))
                    + (abs((r+s-j-1) - target_position[grid_after[r+s-j-1][c+i]].first)         + abs((c+i)     - target_position[grid_after[r+s-j-1][c+i]].second))
                    * (abs((r+s-j-1) - target_position[grid_after[r+s-j-1][c+i]].first)         + abs((c+i)     - target_position[grid_after[r+s-j-1][c+i]].second))
                    - (abs((r+j)     - target_position[grid_after[r+s-j-1][c+i]].first)         + abs((c+s-i-1) - target_position[grid_after[r+s-j-1][c+i]].second))
                    * (abs((r+j)     - target_position[grid_after[r+s-j-1][c+i]].first)         + abs((c+s-i-1) - target_position[grid_after[r+s-j-1][c+i]].second));
      }
    }
  }
  return scoreDelta * p;
}

void commitRotationLast(short s, short r, short c, char dir) {
  if (dir == 'L') {
    for (int i = 0; i < s / 2; i++) {
      for (int j = i; j < s - i - 1; j++) {
        int temp_num = grid_after[r+i][c+j];
        grid_after[r+i][c+j] = grid_after[r+j][c+s-i-1];
        grid_after[r+j][c+s-i-1] = grid_after[r+s-i-1][c+s-j-1];
        grid_after[r+s-i-1][c+s-j-1] = grid_after[r+s-j-1][c+i];
        grid_after[r+s-j-1][c+i] = temp_num;

        after_position[grid_after[r+i][c+j]] = pair<short, short>(r+i, c+j);
        after_position[grid_after[r+j][c+s-i-1]] = pair<short, short>(r+j, c+s-i-1);
        after_position[grid_after[r+s-i-1][c+s-j-1]] = pair<short, short>(r+s-i-1, c+s-j-1);
        after_position[grid_after[r+s-j-1][c+i]] = pair<short, short>(r+s-j-1, c+i);
      }
    }
  }
  else if (dir == 'R') {
    for (int i = 0; i < s / 2; i++) {
      for (int j = i; j < s - i - 1; j++) {
        int temp_num = grid_after[r+i][c+j];
        grid_after[r+i][c+j] = grid_after[r+s-j-1][c+i];
        grid_after[r+s-j-1][c+i] = grid_after[r+s-i-1][c+s-j-1];
        grid_after[r+s-i-1][c+s-j-1] = grid_after[r+j][c+s-i-1];
        grid_after[r+j][c+s-i-1] = temp_num;

        after_position[grid_after[r+i][c+j]] = pair<short, short>(r+i, c+j);
        after_position[grid_after[r+j][c+s-i-1]] = pair<short, short>(r+j, c+s-i-1);
        after_position[grid_after[r+s-i-1][c+s-j-1]] = pair<short, short>(r+s-i-1, c+s-j-1);
        after_position[grid_after[r+s-j-1][c+i]] = pair<short, short>(r+s-j-1, c+i);
      }
    }
  }
  else {
    for (int i = 0; i < s / 2; i++) {
      for (int j = i; j < s - i - 1; j++) {
        int temp_num = grid_after[r+i][c+j];
        grid_after[r+i][c+j] = grid_after[r+j][c+s-i-1];
        grid_after[r+j][c+s-i-1] = grid_after[r+s-i-1][c+s-j-1];
        grid_after[r+s-i-1][c+s-j-1] = grid_after[r+s-j-1][c+i];
        grid_after[r+s-j-1][c+i] = temp_num;

        after_position[grid_after[r+i][c+j]] = pair<short, short>(r+i, c+j);
        after_position[grid_after[r+j][c+s-i-1]] = pair<short, short>(r+j, c+s-i-1);
        after_position[grid_after[r+s-i-1][c+s-j-1]] = pair<short, short>(r+s-i-1, c+s-j-1);
        after_position[grid_after[r+s-j-1][c+i]] = pair<short, short>(r+s-j-1, c+i);
      }
    }
    for (int i = 0; i < s / 2; i++) {
      for (int j = i; j < s - i - 1; j++) {
        int temp_num = grid_after[r+i][c+j];
        grid_after[r+i][c+j] = grid_after[r+j][c+s-i-1];
        grid_after[r+j][c+s-i-1] = grid_after[r+s-i-1][c+s-j-1];
        grid_after[r+s-i-1][c+s-j-1] = grid_after[r+s-j-1][c+i];
        grid_after[r+s-j-1][c+i] = temp_num;

        after_position[grid_after[r+i][c+j]] = pair<short, short>(r+i, c+j);
        after_position[grid_after[r+j][c+s-i-1]] = pair<short, short>(r+j, c+s-i-1);
        after_position[grid_after[r+s-i-1][c+s-j-1]] = pair<short, short>(r+s-i-1, c+s-j-1);
        after_position[grid_after[r+s-j-1][c+i]] = pair<short, short>(r+s-j-1, c+i);
      }
    }
  }
}

int calcScoreDeltaFirstSquared(short s, short r, short c, char dir) {
  int scoreDelta = 0;
  if (dir == 'L') {
    for (int i = 0; i < s / 2; i++) {
      for (int j = i; j < s - i - 1; j++) {
        scoreDelta -= (abs(after_position[grid[r+i][c+j]].first         - target_position[grid[r+i][c+j]].first)         + abs(after_position[grid[r+i][c+j]].second         - target_position[grid[r+i][c+j]].second))
                    * (abs(after_position[grid[r+i][c+j]].first         - target_position[grid[r+i][c+j]].first)         + abs(after_position[grid[r+i][c+j]].second         - target_position[grid[r+i][c+j]].second))
                    - (abs(after_position[grid[r+s-j-1][c+i]].first     - target_position[grid[r+i][c+j]].first)         + abs(after_position[grid[r+s-j-1][c+i]].second     - target_position[grid[r+i][c+j]].second))
                    * (abs(after_position[grid[r+s-j-1][c+i]].first     - target_position[grid[r+i][c+j]].first)         + abs(after_position[grid[r+s-j-1][c+i]].second     - target_position[grid[r+i][c+j]].second))
                    + (abs(after_position[grid[r+s-j-1][c+i]].first     - target_position[grid[r+s-j-1][c+i]].first)     + abs(after_position[grid[r+s-j-1][c+i]].second     - target_position[grid[r+s-j-1][c+i]].second))
                    * (abs(after_position[grid[r+s-j-1][c+i]].first     - target_position[grid[r+s-j-1][c+i]].first)     + abs(after_position[grid[r+s-j-1][c+i]].second     - target_position[grid[r+s-j-1][c+i]].second))
                    - (abs(after_position[grid[r+s-i-1][c+s-j-1]].first - target_position[grid[r+s-j-1][c+i]].first)     + abs(after_position[grid[r+s-i-1][c+s-j-1]].second - target_position[grid[r+s-j-1][c+i]].second))
                    * (abs(after_position[grid[r+s-i-1][c+s-j-1]].first - target_position[grid[r+s-j-1][c+i]].first)     + abs(after_position[grid[r+s-i-1][c+s-j-1]].second - target_position[grid[r+s-j-1][c+i]].second))
                    + (abs(after_position[grid[r+s-i-1][c+s-j-1]].first - target_position[grid[r+s-i-1][c+s-j-1]].first) + abs(after_position[grid[r+s-i-1][c+s-j-1]].second - target_position[grid[r+s-i-1][c+s-j-1]].second))
                    * (abs(after_position[grid[r+s-i-1][c+s-j-1]].first - target_position[grid[r+s-i-1][c+s-j-1]].first) + abs(after_position[grid[r+s-i-1][c+s-j-1]].second - target_position[grid[r+s-i-1][c+s-j-1]].second))
                    - (abs(after_position[grid[r+j][c+s-i-1]].first     - target_position[grid[r+s-i-1][c+s-j-1]].first) + abs(after_position[grid[r+j][c+s-i-1]].second     - target_position[grid[r+s-i-1][c+s-j-1]].second))
                    * (abs(after_position[grid[r+j][c+s-i-1]].first     - target_position[grid[r+s-i-1][c+s-j-1]].first) + abs(after_position[grid[r+j][c+s-i-1]].second     - target_position[grid[r+s-i-1][c+s-j-1]].second))
                    + (abs(after_position[grid[r+j][c+s-i-1]].first     - target_position[grid[r+j][c+s-i-1]].first)     + abs(after_position[grid[r+j][c+s-i-1]].second     - target_position[grid[r+j][c+s-i-1]].second))
                    * (abs(after_position[grid[r+j][c+s-i-1]].first     - target_position[grid[r+j][c+s-i-1]].first)     + abs(after_position[grid[r+j][c+s-i-1]].second     - target_position[grid[r+j][c+s-i-1]].second))
                    - (abs(after_position[grid[r+i][c+j]].first         - target_position[grid[r+j][c+s-i-1]].first)     + abs(after_position[grid[r+i][c+j]].second         - target_position[grid[r+j][c+s-i-1]].second))
                    * (abs(after_position[grid[r+i][c+j]].first         - target_position[grid[r+j][c+s-i-1]].first)     + abs(after_position[grid[r+i][c+j]].second         - target_position[grid[r+j][c+s-i-1]].second));
      }
    }
  }
  else if (dir == 'R') {
    for (int i = 0; i < s / 2; i++) {
      for (int j = i; j < s - i - 1; j++) {
        scoreDelta -= (abs(after_position[grid[r+i][c+j]].first         - target_position[grid[r+i][c+j]].first)         + abs(after_position[grid[r+i][c+j]].second         - target_position[grid[r+i][c+j]].second))
                    * (abs(after_position[grid[r+i][c+j]].first         - target_position[grid[r+i][c+j]].first)         + abs(after_position[grid[r+i][c+j]].second         - target_position[grid[r+i][c+j]].second))
                    - (abs(after_position[grid[r+j][c+s-i-1]].first     - target_position[grid[r+i][c+j]].first)         + abs(after_position[grid[r+j][c+s-i-1]].second     - target_position[grid[r+i][c+j]].second))
                    * (abs(after_position[grid[r+j][c+s-i-1]].first     - target_position[grid[r+i][c+j]].first)         + abs(after_position[grid[r+j][c+s-i-1]].second     - target_position[grid[r+i][c+j]].second))
                    + (abs(after_position[grid[r+j][c+s-i-1]].first     - target_position[grid[r+j][c+s-i-1]].first)     + abs(after_position[grid[r+j][c+s-i-1]].second     - target_position[grid[r+j][c+s-i-1]].second))
                    * (abs(after_position[grid[r+j][c+s-i-1]].first     - target_position[grid[r+j][c+s-i-1]].first)     + abs(after_position[grid[r+j][c+s-i-1]].second     - target_position[grid[r+j][c+s-i-1]].second))
                    - (abs(after_position[grid[r+s-i-1][c+s-j-1]].first - target_position[grid[r+j][c+s-i-1]].first)     + abs(after_position[grid[r+s-i-1][c+s-j-1]].second - target_position[grid[r+j][c+s-i-1]].second))
                    * (abs(after_position[grid[r+s-i-1][c+s-j-1]].first - target_position[grid[r+j][c+s-i-1]].first)     + abs(after_position[grid[r+s-i-1][c+s-j-1]].second - target_position[grid[r+j][c+s-i-1]].second))
                    + (abs(after_position[grid[r+s-i-1][c+s-j-1]].first - target_position[grid[r+s-i-1][c+s-j-1]].first) + abs(after_position[grid[r+s-i-1][c+s-j-1]].second - target_position[grid[r+s-i-1][c+s-j-1]].second))
                    * (abs(after_position[grid[r+s-i-1][c+s-j-1]].first - target_position[grid[r+s-i-1][c+s-j-1]].first) + abs(after_position[grid[r+s-i-1][c+s-j-1]].second - target_position[grid[r+s-i-1][c+s-j-1]].second))
                    - (abs(after_position[grid[r+s-j-1][c+i]].first     - target_position[grid[r+s-i-1][c+s-j-1]].first) + abs(after_position[grid[r+s-j-1][c+i]].second     - target_position[grid[r+s-i-1][c+s-j-1]].second))
                    * (abs(after_position[grid[r+s-j-1][c+i]].first     - target_position[grid[r+s-i-1][c+s-j-1]].first) + abs(after_position[grid[r+s-j-1][c+i]].second     - target_position[grid[r+s-i-1][c+s-j-1]].second))
                    + (abs(after_position[grid[r+s-j-1][c+i]].first     - target_position[grid[r+s-j-1][c+i]].first)     + abs(after_position[grid[r+s-j-1][c+i]].second     - target_position[grid[r+s-j-1][c+i]].second))
                    * (abs(after_position[grid[r+s-j-1][c+i]].first     - target_position[grid[r+s-j-1][c+i]].first)     + abs(after_position[grid[r+s-j-1][c+i]].second     - target_position[grid[r+s-j-1][c+i]].second))
                    - (abs(after_position[grid[r+i][c+j]].first         - target_position[grid[r+s-j-1][c+i]].first)     + abs(after_position[grid[r+i][c+j]].second         - target_position[grid[r+s-j-1][c+i]].second))
                    * (abs(after_position[grid[r+i][c+j]].first         - target_position[grid[r+s-j-1][c+i]].first)     + abs(after_position[grid[r+i][c+j]].second         - target_position[grid[r+s-j-1][c+i]].second));
      }
    }
  }
  else {
    for (int i = 0; i < s / 2; i++) {
      for (int j = i; j < s - i - 1; j++) {
        scoreDelta -= (abs(after_position[grid[r+i][c+j]].first         - target_position[grid[r+i][c+j]].first)             + abs(after_position[grid[r+i][c+j]].second         - target_position[grid[r+i][c+j]].second))
                    * (abs(after_position[grid[r+i][c+j]].first         - target_position[grid[r+i][c+j]].first)             + abs(after_position[grid[r+i][c+j]].second         - target_position[grid[r+i][c+j]].second))
                    - (abs(after_position[grid[r+i][c+j]].first         - target_position[grid[r+s-i-1][c+s-j-1]].first)     + abs(after_position[grid[r+i][c+j]].second         - target_position[grid[r+s-i-1][c+s-j-1]].second))
                    * (abs(after_position[grid[r+i][c+j]].first         - target_position[grid[r+s-i-1][c+s-j-1]].first)     + abs(after_position[grid[r+i][c+j]].second         - target_position[grid[r+s-i-1][c+s-j-1]].second))
                    + (abs(after_position[grid[r+s-i-1][c+s-j-1]].first - target_position[grid[r+s-i-1][c+s-j-1]].first)     + abs(after_position[grid[r+s-i-1][c+s-j-1]].second - target_position[grid[r+s-i-1][c+s-j-1]].second))
                    * (abs(after_position[grid[r+s-i-1][c+s-j-1]].first - target_position[grid[r+s-i-1][c+s-j-1]].first)     + abs(after_position[grid[r+s-i-1][c+s-j-1]].second - target_position[grid[r+s-i-1][c+s-j-1]].second))
                    - (abs(after_position[grid[r+s-i-1][c+s-j-1]].first - target_position[grid[r+i][c+j]].first)             + abs(after_position[grid[r+s-i-1][c+s-j-1]].second - target_position[grid[r+i][c+j]].second))
                    * (abs(after_position[grid[r+s-i-1][c+s-j-1]].first - target_position[grid[r+i][c+j]].first)             + abs(after_position[grid[r+s-i-1][c+s-j-1]].second - target_position[grid[r+i][c+j]].second))
                    + (abs(after_position[grid[r+j][c+s-i-1]].first     - target_position[grid[r+j][c+s-i-1]].first)         + abs(after_position[grid[r+j][c+s-i-1]].second     - target_position[grid[r+j][c+s-i-1]].second))
                    * (abs(after_position[grid[r+j][c+s-i-1]].first     - target_position[grid[r+j][c+s-i-1]].first)         + abs(after_position[grid[r+j][c+s-i-1]].second     - target_position[grid[r+j][c+s-i-1]].second))
                    - (abs(after_position[grid[r+s-j-1][c+i]].first     - target_position[grid[r+j][c+s-i-1]].first)         + abs(after_position[grid[r+s-j-1][c+i]].second     - target_position[grid[r+j][c+s-i-1]].second))
                    * (abs(after_position[grid[r+s-j-1][c+i]].first     - target_position[grid[r+j][c+s-i-1]].first)         + abs(after_position[grid[r+s-j-1][c+i]].second     - target_position[grid[r+j][c+s-i-1]].second))
                    + (abs(after_position[grid[r+s-j-1][c+i]].first     - target_position[grid[r+s-j-1][c+i]].first)         + abs(after_position[grid[r+s-j-1][c+i]].second     - target_position[grid[r+s-j-1][c+i]].second))
                    * (abs(after_position[grid[r+s-j-1][c+i]].first     - target_position[grid[r+s-j-1][c+i]].first)         + abs(after_position[grid[r+s-j-1][c+i]].second     - target_position[grid[r+s-j-1][c+i]].second))
                    - (abs(after_position[grid[r+j][c+s-i-1]].first     - target_position[grid[r+s-j-1][c+i]].first)         + abs(after_position[grid[r+j][c+s-i-1]].second     - target_position[grid[r+s-j-1][c+i]].second))
                    * (abs(after_position[grid[r+j][c+s-i-1]].first     - target_position[grid[r+s-j-1][c+i]].first)         + abs(after_position[grid[r+j][c+s-i-1]].second     - target_position[grid[r+s-j-1][c+i]].second));
      }
    }
  }
  return scoreDelta * p;
}

void commitRotationFirst(short s, short r, short c, char dir) {
  if (dir == 'L') {
    for (int i = 0; i < s / 2; i++) {
      for (int j = i; j < s - i - 1; j++) {
        int temp_num = grid_after[after_position[grid[r+i][c+j]].first][after_position[grid[r+i][c+j]].second];
        grid_after[after_position[grid[r+i][c+j]].first][after_position[grid[r+i][c+j]].second] = grid_after[after_position[grid[r+j][c+s-i-1]].first][after_position[grid[r+j][c+s-i-1]].second];
        grid_after[after_position[grid[r+j][c+s-i-1]].first][after_position[grid[r+j][c+s-i-1]].second] = grid_after[after_position[grid[r+s-i-1][c+s-j-1]].first][after_position[grid[r+s-i-1][c+s-j-1]].second];
        grid_after[after_position[grid[r+s-i-1][c+s-j-1]].first][after_position[grid[r+s-i-1][c+s-j-1]].second] = grid_after[after_position[grid[r+s-j-1][c+i]].first][after_position[grid[r+s-j-1][c+i]].second];
        grid_after[after_position[grid[r+s-j-1][c+i]].first][after_position[grid[r+s-j-1][c+i]].second] = temp_num;

        auto temp_pos = pair<short, short>(after_position[grid[r+i][c+j]].first, after_position[grid[r+i][c+j]].second);
        after_position[grid[r+i][c+j]] = pair<short, short>(after_position[grid[r+s-j-1][c+i]].first, after_position[grid[r+s-j-1][c+i]].second);
        after_position[grid[r+s-j-1][c+i]] = pair<short, short>(after_position[grid[r+s-i-1][c+s-j-1]].first, after_position[grid[r+s-i-1][c+s-j-1]].second);
        after_position[grid[r+s-i-1][c+s-j-1]] = pair<short, short>(after_position[grid[r+j][c+s-i-1]].first, after_position[grid[r+j][c+s-i-1]].second);
        after_position[grid[r+j][c+s-i-1]] = temp_pos;
      }
    }
  }
  else if (dir == 'R') {
    for (int i = 0; i < s / 2; i++) {
      for (int j = i; j < s - i - 1; j++) {
        int temp_num = grid_after[after_position[grid[r+i][c+j]].first][after_position[grid[r+i][c+j]].second];
        grid_after[after_position[grid[r+i][c+j]].first][after_position[grid[r+i][c+j]].second] = grid_after[after_position[grid[r+s-j-1][c+i]].first][after_position[grid[r+s-j-1][c+i]].second];
        grid_after[after_position[grid[r+s-j-1][c+i]].first][after_position[grid[r+s-j-1][c+i]].second] = grid_after[after_position[grid[r+s-i-1][c+s-j-1]].first][after_position[grid[r+s-i-1][c+s-j-1]].second];
        grid_after[after_position[grid[r+s-i-1][c+s-j-1]].first][after_position[grid[r+s-i-1][c+s-j-1]].second] = grid_after[after_position[grid[r+j][c+s-i-1]].first][after_position[grid[r+j][c+s-i-1]].second];
        grid_after[after_position[grid[r+j][c+s-i-1]].first][after_position[grid[r+j][c+s-i-1]].second] = temp_num;

        auto temp_pos = pair<short, short>(after_position[grid[r+i][c+j]].first, after_position[grid[r+i][c+j]].second);
        after_position[grid[r+i][c+j]] = pair<short, short>(after_position[grid[r+j][c+s-i-1]].first, after_position[grid[r+j][c+s-i-1]].second);
        after_position[grid[r+j][c+s-i-1]] = pair<short, short>(after_position[grid[r+s-i-1][c+s-j-1]].first, after_position[grid[r+s-i-1][c+s-j-1]].second);
        after_position[grid[r+s-i-1][c+s-j-1]] = pair<short, short>(after_position[grid[r+s-j-1][c+i]].first, after_position[grid[r+s-j-1][c+i]].second);
        after_position[grid[r+s-j-1][c+i]] = temp_pos;
      }
    }
  }
  else {
    for (int i = 0; i < s / 2; i++) {
      for (int j = i; j < s - i - 1; j++) {
        int temp_num = grid_after[after_position[grid[r+i][c+j]].first][after_position[grid[r+i][c+j]].second];
        grid_after[after_position[grid[r+i][c+j]].first][after_position[grid[r+i][c+j]].second] = grid_after[after_position[grid[r+j][c+s-i-1]].first][after_position[grid[r+j][c+s-i-1]].second];
        grid_after[after_position[grid[r+j][c+s-i-1]].first][after_position[grid[r+j][c+s-i-1]].second] = grid_after[after_position[grid[r+s-i-1][c+s-j-1]].first][after_position[grid[r+s-i-1][c+s-j-1]].second];
        grid_after[after_position[grid[r+s-i-1][c+s-j-1]].first][after_position[grid[r+s-i-1][c+s-j-1]].second] = grid_after[after_position[grid[r+s-j-1][c+i]].first][after_position[grid[r+s-j-1][c+i]].second];
        grid_after[after_position[grid[r+s-j-1][c+i]].first][after_position[grid[r+s-j-1][c+i]].second] = temp_num;

        auto temp_pos = pair<short, short>(after_position[grid[r+i][c+j]].first, after_position[grid[r+i][c+j]].second);
        after_position[grid[r+i][c+j]] = pair<short, short>(after_position[grid[r+s-j-1][c+i]].first, after_position[grid[r+s-j-1][c+i]].second);
        after_position[grid[r+s-j-1][c+i]] = pair<short, short>(after_position[grid[r+s-i-1][c+s-j-1]].first, after_position[grid[r+s-i-1][c+s-j-1]].second);
        after_position[grid[r+s-i-1][c+s-j-1]] = pair<short, short>(after_position[grid[r+j][c+s-i-1]].first, after_position[grid[r+j][c+s-i-1]].second);
        after_position[grid[r+j][c+s-i-1]] = temp_pos;
      }
    }
    for (int i = 0; i < s / 2; i++) {
      for (int j = i; j < s - i - 1; j++) {
        int temp_num = grid_after[after_position[grid[r+i][c+j]].first][after_position[grid[r+i][c+j]].second];
        grid_after[after_position[grid[r+i][c+j]].first][after_position[grid[r+i][c+j]].second] = grid_after[after_position[grid[r+j][c+s-i-1]].first][after_position[grid[r+j][c+s-i-1]].second];
        grid_after[after_position[grid[r+j][c+s-i-1]].first][after_position[grid[r+j][c+s-i-1]].second] = grid_after[after_position[grid[r+s-i-1][c+s-j-1]].first][after_position[grid[r+s-i-1][c+s-j-1]].second];
        grid_after[after_position[grid[r+s-i-1][c+s-j-1]].first][after_position[grid[r+s-i-1][c+s-j-1]].second] = grid_after[after_position[grid[r+s-j-1][c+i]].first][after_position[grid[r+s-j-1][c+i]].second];
        grid_after[after_position[grid[r+s-j-1][c+i]].first][after_position[grid[r+s-j-1][c+i]].second] = temp_num;

        auto temp_pos = pair<short, short>(after_position[grid[r+i][c+j]].first, after_position[grid[r+i][c+j]].second);
        after_position[grid[r+i][c+j]] = pair<short, short>(after_position[grid[r+s-j-1][c+i]].first, after_position[grid[r+s-j-1][c+i]].second);
        after_position[grid[r+s-j-1][c+i]] = pair<short, short>(after_position[grid[r+s-i-1][c+s-j-1]].first, after_position[grid[r+s-i-1][c+s-j-1]].second);
        after_position[grid[r+s-i-1][c+s-j-1]] = pair<short, short>(after_position[grid[r+j][c+s-i-1]].first, after_position[grid[r+j][c+s-i-1]].second);
        after_position[grid[r+j][c+s-i-1]] = temp_pos;
      }
    }
  }
}

void greedySolve(int inc_limit) {
  int s_limit = n;
  while (true) {
    short best_s = 0;
    short best_r = 0;
    short best_c = 0;
    char best_dir = 'L';
    bool best_first = false;
    int bestScore = 1000000007;
    for (short s = 2; s <= s_limit; s++) {
      for (short r = 0; r + s - 1 < n; r++) {
        for (short c = 0; c + s - 1 < n; c++) {
          int scoreDelta_L = calcScoreDeltaLastSquared(s, r, c, 'L') + penaltyTable[s];
          if (scoreDelta_L < bestScore) {
            best_s = s;
            best_r = r;
            best_c = c;
            best_dir = 'L';
            bestScore = scoreDelta_L;
          }
          int scoreDelta_R = calcScoreDeltaLastSquared(s, r, c, 'R') + penaltyTable[s];
          if (scoreDelta_R < bestScore) {
            best_s = s;
            best_r = r;
            best_c = c;
            best_dir = 'R';
            bestScore = scoreDelta_R;
          }
          int scoreDelta_F = calcScoreDeltaLastSquared(s, r, c, 'F') + penaltyTable[s] * 2;
          if (scoreDelta_F < bestScore) {
            best_s = s;
            best_r = r;
            best_c = c;
            best_dir = 'F';
            bestScore = scoreDelta_F;
          }
        }
      }
    }
    for (short s = 2; s <= s_limit; s++) {
      for (short r = 0; r + s - 1 < n; r++) {
        for (short c = 0; c + s - 1 < n; c++) {
          int scoreDelta_L = calcScoreDeltaFirstSquared(s, r, c, 'L') + penaltyTable[s];
          if (scoreDelta_L < bestScore) {
            best_s = s;
            best_r = r;
            best_c = c;
            best_dir = 'L';
            best_first = true;
            bestScore = scoreDelta_L;
          }
          int scoreDelta_R = calcScoreDeltaFirstSquared(s, r, c, 'R') + penaltyTable[s];
          if (scoreDelta_R < bestScore) {
            best_s = s;
            best_r = r;
            best_c = c;
            best_dir = 'R';
            best_first = true;
            bestScore = scoreDelta_R;
          }
          int scoreDelta_F = calcScoreDeltaFirstSquared(s, r, c, 'F') + penaltyTable[s] * 2;
          if (scoreDelta_F < bestScore) {
            best_s = s;
            best_r = r;
            best_c = c;
            best_dir = 'F';
            best_first = true;
            bestScore = scoreDelta_F;
          }
        }
      }
    }
    if (bestScore <= 0) {
      if (best_first) {
        commitRotationFirst(best_s, best_r, best_c, best_dir);
        operation.push_front({best_s, best_r, best_c, best_dir});
      }
      else {
        commitRotationLast(best_s, best_r, best_c, best_dir);
        operation.push_back({best_s, best_r, best_c, best_dir});
      }
    }
    else break;
    s_limit = min(n, best_s + inc_limit);
  }
}

void greedySolveLinear(int inc_limit) {
  int s_limit = n;
  while (true) {
    short best_s = 0;
    short best_r = 0;
    short best_c = 0;
    char best_dir = 'L';
    bool best_first = false;
    int bestScore = 1000000007;
    for (short s = 2; s <= s_limit; s++) {
      for (short r = 0; r + s - 1 < n; r++) {
        for (short c = 0; c + s - 1 < n; c++) {
          int scoreDelta_L = calcScoreDeltaLast(s, r, c, 'L') + penaltyTable[s];
          if (scoreDelta_L < bestScore) {
            best_s = s;
            best_r = r;
            best_c = c;
            best_dir = 'L';
            bestScore = scoreDelta_L;
          }
          int scoreDelta_R = calcScoreDeltaLast(s, r, c, 'R') + penaltyTable[s];
          if (scoreDelta_R < bestScore) {
            best_s = s;
            best_r = r;
            best_c = c;
            best_dir = 'R';
            bestScore = scoreDelta_R;
          }
          int scoreDelta_F = calcScoreDeltaLast(s, r, c, 'F') + penaltyTable[s] * 2;
          if (scoreDelta_F < bestScore) {
            best_s = s;
            best_r = r;
            best_c = c;
            best_dir = 'F';
            bestScore = scoreDelta_F;
          }
        }
      }
    }
    for (short s = 2; s <= s_limit; s++) {
      for (short r = 0; r + s - 1 < n; r++) {
        for (short c = 0; c + s - 1 < n; c++) {
          int scoreDelta_L = calcScoreDeltaFirst(s, r, c, 'L') + penaltyTable[s];
          if (scoreDelta_L < bestScore) {
            best_s = s;
            best_r = r;
            best_c = c;
            best_dir = 'L';
            best_first = true;
            bestScore = scoreDelta_L;
          }
          int scoreDelta_R = calcScoreDeltaFirst(s, r, c, 'R') + penaltyTable[s];
          if (scoreDelta_R < bestScore) {
            best_s = s;
            best_r = r;
            best_c = c;
            best_dir = 'R';
            best_first = true;
            bestScore = scoreDelta_R;
          }
          int scoreDelta_F = calcScoreDeltaFirst(s, r, c, 'F') + penaltyTable[s] * 2;
          if (scoreDelta_F < bestScore) {
            best_s = s;
            best_r = r;
            best_c = c;
            best_dir = 'F';
            best_first = true;
            bestScore = scoreDelta_F;
          }
        }
      }
    }
    if (bestScore <= 0) {
      if (best_first) {
        commitRotationFirst(best_s, best_r, best_c, best_dir);
        operation.push_front({best_s, best_r, best_c, best_dir});
      }
      else {
        commitRotationLast(best_s, best_r, best_c, best_dir);
        operation.push_back({best_s, best_r, best_c, best_dir});
      }
    }
    else break;
    s_limit = min(n, best_s + inc_limit);
  }
}

bool greedyStepLinear(int &s_limit) {
  int inc_limit = greedyIncLimit[n];
  short best_s = 0;
  short best_r = 0;
  short best_c = 0;
  char best_dir = 'L';
  bool best_first = false;
  int bestScore = 1000000007;
  for (short s = 2; s <= s_limit; s++) {
    for (short r = 0; r + s - 1 < n; r++) {
      for (short c = 0; c + s - 1 < n; c++) {
        int scoreDelta_L = calcScoreDeltaLast(s, r, c, 'L') + penaltyTable[s];
        if (scoreDelta_L < bestScore) {
          best_s = s;
          best_r = r;
          best_c = c;
          best_dir = 'L';
          bestScore = scoreDelta_L;
        }
        int scoreDelta_R = calcScoreDeltaLast(s, r, c, 'R') + penaltyTable[s];
        if (scoreDelta_R < bestScore) {
          best_s = s;
          best_r = r;
          best_c = c;
          best_dir = 'R';
          bestScore = scoreDelta_R;
        }
        int scoreDelta_F = calcScoreDeltaLast(s, r, c, 'F') + penaltyTable[s] * 2;
        if (scoreDelta_F < bestScore) {
          best_s = s;
          best_r = r;
          best_c = c;
          best_dir = 'F';
          bestScore = scoreDelta_F;
        }
      }
    }
  }
  for (short s = 2; s <= s_limit; s++) {
    for (short r = 0; r + s - 1 < n; r++) {
      for (short c = 0; c + s - 1 < n; c++) {
        int scoreDelta_L = calcScoreDeltaFirst(s, r, c, 'L') + penaltyTable[s];
        if (scoreDelta_L < bestScore) {
          best_s = s;
          best_r = r;
          best_c = c;
          best_dir = 'L';
          best_first = true;
          bestScore = scoreDelta_L;
        }
        int scoreDelta_R = calcScoreDeltaFirst(s, r, c, 'R') + penaltyTable[s];
        if (scoreDelta_R < bestScore) {
          best_s = s;
          best_r = r;
          best_c = c;
          best_dir = 'R';
          best_first = true;
          bestScore = scoreDelta_R;
        }
        int scoreDelta_F = calcScoreDeltaFirst(s, r, c, 'F') + penaltyTable[s] * 2;
        if (scoreDelta_F < bestScore) {
          best_s = s;
          best_r = r;
          best_c = c;
          best_dir = 'F';
          best_first = true;
          bestScore = scoreDelta_F;
        }
      }
    }
  }
  if (bestScore <= 0) {
    if (best_first) {
      commitRotationFirst(best_s, best_r, best_c, best_dir);
      operation.push_front({best_s, best_r, best_c, best_dir});
    }
    else {
      commitRotationLast(best_s, best_r, best_c, best_dir);
      operation.push_back({best_s, best_r, best_c, best_dir});
    }
    s_limit = min(n, best_s + inc_limit);
    return true;
  }
  else return false;
}

void greedySolve2() {
  for (int i = 0; i < n - 2; i++) {
    for (int j = 0; j < n - 2; j++) {
      while (after_position[i*n+j] != target_position[i*n+j]) {
        int score_L = 1000000007;
        int score_R = 1000000007;
        if (after_position[i*n+j].first >= target_position[i*n+j].first) {
          if (after_position[i*n+j].second < target_position[i*n+j].second) { // bottom left
            if (after_position[i*n+j].first > target_position[i*n+j].first+1) score_L = calcScoreDeltaLastSquared(2, after_position[i*n+j].first-1, after_position[i*n+j].second, 'L');
            if (after_position[i*n+j].first < n-1) score_R = calcScoreDeltaLastSquared(2, after_position[i*n+j].first, after_position[i*n+j].second, 'R');
            if (score_L < score_R) {
              operation.push_back({2, (short)(after_position[i*n+j].first-1), (short)(after_position[i*n+j].second), 'L'});
              commitRotationLast(2, after_position[i*n+j].first-1, after_position[i*n+j].second, 'L');
            }
            else {
              operation.push_back({2, (short)(after_position[i*n+j].first), (short)(after_position[i*n+j].second), 'R'});
              commitRotationLast(2, after_position[i*n+j].first, after_position[i*n+j].second, 'R');
            }
          }
          else if (after_position[i*n+j].second > target_position[i*n+j].second) { // bottom right
            if (after_position[i*n+j].first < n-1) score_L = calcScoreDeltaLastSquared(2, after_position[i*n+j].first, after_position[i*n+j].second-1, 'L');
            if (after_position[i*n+j].first > target_position[i*n+j].first) score_R = calcScoreDeltaLastSquared(2, after_position[i*n+j].first-1, after_position[i*n+j].second-1, 'R');
            if (score_L < score_R) {
              operation.push_back({2, (short)(after_position[i*n+j].first), (short)(after_position[i*n+j].second-1), 'L'});
              commitRotationLast(2, (short)(after_position[i*n+j].first), (short)(after_position[i*n+j].second-1), 'L');
            }
            else {
              operation.push_back({2, (short)(after_position[i*n+j].first-1), (short)(after_position[i*n+j].second-1), 'R'});
              commitRotationLast(2, (short)(after_position[i*n+j].first-1), (short)(after_position[i*n+j].second-1), 'R');
            }
          }
          else { // bottom center
            if (after_position[i*n+j].second > 0 && after_position[i*n+j].first > target_position[i*n+j].first+1) score_L = calcScoreDeltaLastSquared(2, (short)(after_position[i*n+j].first-1), (short)(after_position[i*n+j].second-1), 'L');
            if (after_position[i*n+j].second < n-1) score_R = calcScoreDeltaLastSquared(2, (short)(after_position[i*n+j].first-1), (short)(after_position[i*n+j].second), 'R');
            if (score_L < score_R) {
              operation.push_back({2, (short)(after_position[i*n+j].first-1), (short)(after_position[i*n+j].second-1), 'L'});
              commitRotationLast(2, (short)(after_position[i*n+j].first-1), (short)(after_position[i*n+j].second-1), 'L');
            }
            else {
              operation.push_back({2, (short)(after_position[i*n+j].first-1), (short)(after_position[i*n+j].second), 'R'});
              commitRotationLast(2, (short)(after_position[i*n+j].first-1), (short)(after_position[i*n+j].second), 'R');
            }
          }
        }
        else { // top
          if (after_position[i*n+j].second < n-1) score_L = calcScoreDeltaLastSquared(2, (short)(after_position[i*n+j].first), after_position[i*n+j].second, 'L');
          if (after_position[i*n+j].second == n-1) score_R = calcScoreDeltaLastSquared(2, (short)(after_position[i*n+j].first), after_position[i*n+j].second-1, 'R');
          if (score_L < score_R) {
            operation.push_back({2, (short)(after_position[i*n+j].first), (short)(after_position[i*n+j].second), 'L'});
            commitRotationLast(2, (short)(after_position[i*n+j].first), (short)(after_position[i*n+j].second), 'L');
          }
          else {
            operation.push_back({2, (short)(after_position[i*n+j].first), (short)(after_position[i*n+j].second-1), 'R'});
            commitRotationLast(2, (short)(after_position[i*n+j].first), (short)(after_position[i*n+j].second-1), 'R');
          }
        }
      }
    }
  }
  for (int i = 0; i < n - 2; i++) {
    if (i < n - 3) {
      while (after_position[i*n+(n-1)].first > i+1 || after_position[i*n+(n-1)].second < n-2) {
        if (after_position[i*n+(n-1)].second < n-2) {
          operation.push_back({2, (short) (n-2), (short)(after_position[i*n+(n-1)].second), 'R'});
          commitRotationLast(2, (short) (n-2), (short)(after_position[i*n+(n-1)].second), 'R');
        }
        else {
          operation.push_back({2, (short) (after_position[i*n+(n-1)].first-1), (short) (n-2), 'R'});
          commitRotationLast(2, (short) (after_position[i*n+(n-1)].first-1), (short) (n-2), 'R');
        }
      }
      while (after_position[i*n+(n-2)].first > i+2 || after_position[i*n+(n-2)].second < n-2) {
        if (after_position[i*n+(n-2)].second < n-2) {
          operation.push_back({2, (short) (n-2), (short)(after_position[i*n+(n-2)].second), 'R'});
          commitRotationLast(2, (short) (n-2), (short)(after_position[i*n+(n-2)].second), 'R');
        }
        else {
          operation.push_back({2, (short) (after_position[i*n+(n-2)].first-1), (short) (n-2), 'R'});
          commitRotationLast(2, (short) (after_position[i*n+(n-2)].first-1), (short) (n-2), 'R');
        }
      }
    }
    else {
      while (after_position[i*n+(n-1)].second < n-1) {
        if (after_position[i*n+(n-1)].first < n-2) {
          operation.push_back({2, (short) (after_position[i*n+(n-1)].first), (short) (n-2), 'R'});
          commitRotationLast(2, (short) (after_position[i*n+(n-1)].first), (short) (n-2), 'R');
        }
        else {
          operation.push_back({2, (short) (n-2), (short)(after_position[i*n+(n-1)].second), 'R'});
          commitRotationLast(2, (short) (n-2), (short)(after_position[i*n+(n-1)].second), 'R');
        }
      }
      while (after_position[i*n+(n-2)].second < n-2) {
        operation.push_back({2, (short) (n-2), (short)(after_position[i*n+(n-2)].second), 'R'});
        commitRotationLast(2, (short) (n-2), (short)(after_position[i*n+(n-2)].second), 'R');
      }
    }
    queue<pair<vector<vector<short> >, vector<vector<short> > > > que;
    que.emplace(vector<vector<short> >({{grid_after[i][n-2], grid_after[i][n-1]}, {grid_after[i+1][n-2], grid_after[i+1][n-1]}, {grid_after[i+2][n-2], grid_after[i+2][n-1]}}), vector<vector<short> >());
    while (!que.empty()) {
      auto now = que.front();
      que.pop();
      if (now.first[0][0] == i*n+(n-2) && now.first[0][1] == i*n+(n-1)) {
        for (auto &x: now.second) {
          operation.push_back({x[0], x[1], x[2], (char) x[3]});
          commitRotationLast(x[0], x[1], x[2], (char) x[3]);
        }
        break;
      }
      auto next = now.second;
      next.push_back({2, (short) i, (short) (n-2), 'L'});
      que.emplace(vector<vector<short> >({{now.first[0][1], now.first[1][1]}, {now.first[0][0], now.first[1][0]}, {now.first[2][0], now.first[2][1]}}), next);
      next = now.second;
      next.push_back({2, (short) i, (short) (n-2), 'R'});
      que.emplace(vector<vector<short> >({{now.first[1][0], now.first[0][0]}, {now.first[1][1], now.first[0][1]}, {now.first[2][0], now.first[2][1]}}), next);
      next = now.second;
      next.push_back({2, (short) (i+1), (short) (n-2), 'L'});
      que.emplace(vector<vector<short> >({{now.first[0][0], now.first[0][1]}, {now.first[1][1], now.first[2][1]}, {now.first[1][0], now.first[2][0]}}), next);
      next = now.second;
      next.push_back({2, (short) (i+1), (short) (n-2), 'R'});
      que.emplace(vector<vector<short> >({{now.first[0][0], now.first[0][1]}, {now.first[2][0], now.first[1][0]}, {now.first[2][1], now.first[1][1]}}), next);
    }
  }
  for (int j = 0; j < n - 2; j++) {
    while (after_position[(n-2)*n+j].second > j+1) {
      if (after_position[(n-2)*n+j].first > n-1) {
        operation.push_back({2, (short) (n-2), (short)(after_position[(n-2)*n+j].second-1), 'L'});
        commitRotationLast(2, (short) (n-2), (short)(after_position[(n-2)*n+j].second-1), 'L');
      }
      else {
        operation.push_back({2, (short) (n-2), (short)(after_position[(n-2)*n+j].second-1), 'R'});
        commitRotationLast(2, (short) (n-2), (short)(after_position[(n-2)*n+j].second-1), 'R');
      }
    }
    while (after_position[(n-1)*n+j].second > j+2) {
      if (after_position[(n-1)*n+j].first > n-1) {
        operation.push_back({2, (short) (n-2), (short)(after_position[(n-1)*n+j].second-1), 'L'});
        commitRotationLast(2, (short) (n-2), (short)(after_position[(n-1)*n+j].second-1), 'L');
      }
      else {
        operation.push_back({2, (short) (n-2), (short)(after_position[(n-1)*n+j].second-1), 'R'});
        commitRotationLast(2, (short) (n-2), (short)(after_position[(n-1)*n+j].second-1), 'R');
      }
    }
    queue<pair<vector<vector<short> >, vector<vector<short> > > > que;
    que.emplace(vector<vector<short> >({{grid_after[n-2][j], grid_after[n-2][j+1], grid_after[n-2][j+2]}, {grid_after[n-1][j], grid_after[n-1][j+1], grid_after[n-1][j+2]}}), vector<vector<short> >());
    while (!que.empty()) {
      auto now = que.front();
      que.pop();
      if (now.first[0][0] == (n-2)*n+j && now.first[1][0] == (n-1)*n+j) {
        for (auto &x: now.second) {
          operation.push_back({x[0], x[1], x[2], (char) x[3]});
          commitRotationLast(x[0], x[1], x[2], (char) x[3]);
        }
        break;
      }
      auto next = now.second;
      next.push_back({2, (short) (n-2), (short) j, 'L'});
      que.emplace(vector<vector<short> >({{now.first[0][1], now.first[1][1], now.first[0][2]}, {now.first[0][0], now.first[1][0], now.first[1][2]}}), next);
      next = now.second;
      next.push_back({2, (short) (n-2), (short) j, 'R'});
      que.emplace(vector<vector<short> >({{now.first[1][0], now.first[0][0], now.first[0][2]}, {now.first[1][1], now.first[0][1], now.first[1][2]}}), next);
      next = now.second;
      next.push_back({2, (short) (n-2), (short) (j+1), 'L'});
      que.emplace(vector<vector<short> >({{now.first[0][0], now.first[0][2], now.first[1][2]}, {now.first[1][0], now.first[0][1], now.first[1][1]}}), next);
      next = now.second;
      next.push_back({2, (short) (n-2), (short) (j+1), 'R'});
      que.emplace(vector<vector<short> >({{now.first[0][0], now.first[1][1], now.first[0][1]}, {now.first[1][0], now.first[1][2], now.first[0][2]}}), next);
    }
  }
}

void beamSolve(int beamWidth) {
  using stateContainer = priority_queue<tuple<int, vector<vector<short> >, vector<vector<short> > >, vector<tuple<int, vector<vector<short> >, vector<vector<short> > > >, greater<tuple<int, vector<vector<short> >, vector<vector<short> > > > >;
  vector<stateContainer> state(3);
  int bestScore = 1000000007;
  vector<vector<short> > bestState;
  state[0].emplace(calcScore(), vector<vector<short> >(), grid_after);
  for (int k = 0; k < 101; k++) {
    if (k > bestScore) break;
    for (int t = 0; t < beamWidth; t++) {
      if (state[0].empty()) break;
      auto now = state[0].top();
      state[0].pop();
      if (get<0>(now) < bestScore) {
        bestScore = get<0>(now);
        bestState = get<1>(now);
      }
      grid_after = get<2>(now);
      for (short s = 2; s <= 3; s++) {
        for (short r = 0; r + s - 1 < n; r++) {
          for (short c = 0; c + s - 1 < n; c++) {
            if (k == 0) {
              int score_L = get<0>(now) + calcScoreDeltaLast(s, r, c, 'L') + penaltyTable[s];
              auto next_L = get<1>(now);
              next_L.push_back({s, r, c, 'L'});
              commitRotationLast(s, r, c, 'L');
              state[1].emplace(score_L, next_L, grid_after);
              commitRotationLast(s, r, c, 'R');
              int score_R = get<0>(now) + calcScoreDeltaLast(s, r, c, 'R') + penaltyTable[s];
              auto next_R = get<1>(now);
              next_R.push_back({s, r, c, 'R'});
              commitRotationLast(s, r, c, 'R');
              state[1].emplace(score_R, next_R, grid_after);
              commitRotationLast(s, r, c, 'L');
              int score_F = get<0>(now) + calcScoreDeltaLast(s, r, c, 'F') + penaltyTable[s] * 2;
              auto next_F = get<1>(now);
              next_F.push_back({s, r, c, 'F'});
              commitRotationLast(s, r, c, 'F');
              state[1].emplace(score_F, next_F, grid_after);
              commitRotationLast(s, r, c, 'F');
            }
            else if (k == 1) {
              if (get<1>(now)[k-1][0] != s || get<1>(now)[k-1][1] != r || get<1>(now)[k-1][2] != c) {
                int score_L = get<0>(now) + calcScoreDeltaLast(s, r, c, 'L') + penaltyTable[s];
                auto next_L = get<1>(now);
                next_L.push_back({s, r, c, 'L'});
                commitRotationLast(s, r, c, 'L');
                state[1].emplace(score_L, next_L, grid_after);
                commitRotationLast(s, r, c, 'R');
                int score_R = get<0>(now) + calcScoreDeltaLast(s, r, c, 'R') + penaltyTable[s];
                auto next_R = get<1>(now);
                next_R.push_back({s, r, c, 'R'});
                commitRotationLast(s, r, c, 'R');
                state[1].emplace(score_R, next_R, grid_after);
                commitRotationLast(s, r, c, 'L');
                int score_F = get<0>(now) + calcScoreDeltaLast(s, r, c, 'F') + penaltyTable[s] * 2;
                auto next_F = get<1>(now);
                next_F.push_back({s, r, c, 'F'});
                commitRotationLast(s, r, c, 'F');
                state[1].emplace(score_F, next_F, grid_after);
                commitRotationLast(s, r, c, 'F');
              }
            }
            else if ((get<1>(now)[k-1][0] != s || get<1>(now)[k-1][1] != r || get<1>(now)[k-1][2] != c) && (get<1>(now)[k-2][0] != s || get<1>(now)[k-2][1] != r || get<1>(now)[k-2][2] != c)) {
              int score_L = get<0>(now) + calcScoreDeltaLast(s, r, c, 'L') + penaltyTable[s];
              auto next_L = get<1>(now);
              next_L.push_back({s, r, c, 'L'});
              commitRotationLast(s, r, c, 'L');
              state[1].emplace(score_L, next_L, grid_after);
              commitRotationLast(s, r, c, 'R');
              int score_R = get<0>(now) + calcScoreDeltaLast(s, r, c, 'R') + penaltyTable[s];
              auto next_R = get<1>(now);
              next_R.push_back({s, r, c, 'R'});
              commitRotationLast(s, r, c, 'R');
              state[1].emplace(score_R, next_R, grid_after);
              commitRotationLast(s, r, c, 'L');
              int score_F = get<0>(now) + calcScoreDeltaLast(s, r, c, 'F') + penaltyTable[s] * 2;
              auto next_F = get<1>(now);
              next_F.push_back({s, r, c, 'F'});
              commitRotationLast(s, r, c, 'F');
              state[1].emplace(score_F, next_F, grid_after);
              commitRotationLast(s, r, c, 'F');
            }
          }
        }
      }
      if (theTimer.time() > 8.000) break;
    }
    state[0] = state[1];
    state[1] = stateContainer();
  }
  grid_after = grid;
  after_position = vector<pair<short, short> >(n*n);
  target_position = vector<pair<short, short> >(n*n);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      after_position[grid[i][j]] = pair<short, short>(i, j);
      target_position[i*n+j] = pair<short, short>(i, j);
    }
  }
  for (auto &x: bestState) {
    commitRotationLast(x[0], x[1], x[2], x[3]);
    operation.push_back({x[0], x[1], x[2], x[3]});
  }
}

int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);

  // input
  cin >> n >> p;
  grid = vector<vector<short> >(n, vector<short>(n));
  for (auto &x: grid) {
    for (auto &y: x) {
      cin >> y;
      y--;
    }
  }

  // preprocess
  grid_after = grid;
  after_position = vector<pair<short, short> >(n*n);
  target_position = vector<pair<short, short> >(n*n);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      after_position[grid[i][j]] = pair<short, short>(i, j);
      target_position[i*n+j] = pair<short, short>(i, j);
    }
  }

  if (n <= 6 || n >= 19) {
    if (n == 4) beamSolve(2048);
    else if (n == 5) beamSolve(512);
    else if (n == 6) beamSolve(128);
    if (n <= 37) {
      auto prevOperation = operation;

      greedySolve(greedyIncLimit[n]);
      int score3 = calcScore();
      int penalty3 = 0;
      for (auto &x: operation) {
        if (x[3] == 'F') penalty3 += penaltyTable[x[0]] * 2;
        else penalty3 += penaltyTable[x[0]];
      }
      auto operation3 = operation;
      greedySolve2();
      int score5 = calcScore();
      int penalty5 = 0;
      for (auto &x: operation) {
        if (x[3] == 'F') penalty5 += penaltyTable[x[0]] * 2;
        else penalty5 += penaltyTable[x[0]];
      }
      auto operation5 = operation;
      greedySolve(greedyIncLimit[n]);
      int score1 = calcScore();
      int penalty1 = 0;
      for (auto &x: operation) {
        if (x[3] == 'F') penalty1 += penaltyTable[x[0]] * 2;
        else penalty1 += penaltyTable[x[0]];
      }
      auto operation1 = operation;

      grid_after = grid;
      after_position = vector<pair<short, short> >(n*n);
      target_position = vector<pair<short, short> >(n*n);
      for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
          after_position[grid[i][j]] = pair<short, short>(i, j);
          target_position[i*n+j] = pair<short, short>(i, j);
        }
      }
      operation = prevOperation;
      for (auto &x: operation) {
        commitRotationLast(x[0], x[1], x[2], x[3]);
      }

      greedySolveLinear(greedyIncLimit[n]);
      int score4 = calcScore();
      int penalty4 = 0;
      for (auto &x: operation) {
        if (x[3] == 'F') penalty4 += penaltyTable[x[0]] * 2;
        else penalty4 += penaltyTable[x[0]];
      }
      auto operation4 = operation;
      greedySolve2();
      int score6 = calcScore();
      int penalty6 = 0;
      for (auto &x: operation) {
        if (x[3] == 'F') penalty6 += penaltyTable[x[0]] * 2;
        else penalty6 += penaltyTable[x[0]];
      }
      auto operation6 = operation;
      greedySolveLinear(greedyIncLimit[n]);
      int score2 = calcScore();
      int penalty2 = 0;
      for (auto &x: operation) {
        if (x[3] == 'F') penalty2 += penaltyTable[x[0]] * 2;
        else penalty2 += penaltyTable[x[0]];
      }
      auto operation2 = operation;

      grid_after = grid;
      after_position = vector<pair<short, short> >(n*n);
      target_position = vector<pair<short, short> >(n*n);
      for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
          after_position[grid[i][j]] = pair<short, short>(i, j);
          target_position[i*n+j] = pair<short, short>(i, j);
        }
      }

      int bestScore = score1 + penalty1;
      operation = operation1;
      if (score2 + penalty2 < bestScore) {
        bestScore = score2 + penalty2;
        operation = operation2;
      }
      if (score3 + penalty3 < bestScore) {
        bestScore = score3 + penalty3;
        operation = operation3;
      }
      if (score4 + penalty4 < bestScore) {
        bestScore = score4 + penalty4;
        operation = operation4;
      }
      if (score5 + penalty5 < bestScore) {
        bestScore = score5 + penalty5;
        operation = operation5;
      }
      if (score6 + penalty6 < bestScore) {
        bestScore = score6 + penalty6;
        operation = operation6;
      }
      best_operation = operation;
    }
    else {
      if (p == 1) greedySolve(1);
      else greedySolveLinear(1);
      greedySolve2();
      greedySolveLinear(1);
      best_operation = operation;
    }
  }
  else {
    int s_limit = n;
    auto prevOperation = operation;
    int greedyBest = calcScore();
    while (true) {
      grid_after = grid;
      after_position = vector<pair<short, short> >(n*n);
      target_position = vector<pair<short, short> >(n*n);
      for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
          after_position[grid[i][j]] = pair<short, short>(i, j);
          target_position[i*n+j] = pair<short, short>(i, j);
        }
      }
      operation = prevOperation;
      for (auto &x: operation) {
        commitRotationLast(x[0], x[1], x[2], x[3]);
      }
      if (greedyStepLinear(s_limit)) {
        prevOperation = operation;
        int greedyScore = calcScore() + calcPenalty();
        if (greedyScore < greedyBest) {
          greedyBest = greedyScore;
          best_operation = operation;
        }
        greedySolve2();
        int greedyScore2 = calcScore() + calcPenalty();
        if (greedyScore2 < greedyBest) {
          greedyBest = greedyScore2;
          best_operation = operation;
        }
      }
      else break;
    }
  }

  grid_after = grid;
  after_position = vector<pair<short, short> >(n*n);
  target_position = vector<pair<short, short> >(n*n);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      after_position[grid[i][j]] = pair<short, short>(i, j);
      target_position[i*n+j] = pair<short, short>(i, j);
    }
  }
  operation = best_operation;
  for (auto &x: operation) {
    commitRotationLast(x[0], x[1], x[2], x[3]);
  }

  int penalty = 0;
  for (auto &x: operation) {
    if (x[3] == 'F') penalty += penaltyTable[x[0]] * 2;
    else penalty += penaltyTable[x[0]];
  }

  cerr << "timeGreedy  = " << theTimer.time() << endl;

  // optimization
  int score = calcScore() + penalty;
  int lastScore = score;
  int bestScore = score;

  double baseTemperature = 1e0;
  double temperature = baseTemperature;
  double decayRate = 1e-7;
  double timeLimit = 9.900;
  int iterCount = 0;

  while (theTimer.time() < timeLimit) {
    double roll = theRandom.nextDouble();
    if (roll < 0.2) {
      short s = theRandom.nextUShortMod(min(10, n-1)) + 2;
      short r = theRandom.nextUShortMod(n-s+1);
      short c = theRandom.nextUShortMod(n-s+1);
      char dir = !(theRandom.nextUIntMod(4)) ? 'F' : (theRandom.nextUIntMod(2) ? 'L' : 'R');

      if (dir == 'F') score += calcScoreDeltaLast(s, r, c, dir) + penaltyTable[s] * 2;
      else score += calcScoreDeltaLast(s, r, c, dir) + penaltyTable[s];

      #ifdef DEBUG
      if (iterCount % 100000 == 0) cerr << iterCount << " " << score << " " << lastScore << " " << bestScore << " " << temperature << " " << operation.size() << endl;
      #endif

      if (score <= lastScore) {
        commitRotationLast(s, r, c, dir);
        operation.push_back({s, r, c, dir});
        lastScore = score;
        if (score < bestScore) {
          bestScore = score;
          best_operation = operation;
        }
      }
      else if (theRandom.nextDouble() < exp(double(lastScore - score) / temperature)) { // accept
        commitRotationLast(s, r, c, dir);
        operation.push_back({s, r, c, dir});
        lastScore = score;
      }
      else { // rollback
        score = lastScore;
      }
    }
    else if (roll < 0.5) {
      if (operation.size() == 0) continue;
      auto now = operation.back();
      short s = now[0];
      short r = now[1];
      short c = now[2];
      char dir = now[3];
      if (dir == 'L') dir = 'R';
      else if (dir == 'R') dir = 'L';

      if (dir == 'F') score += calcScoreDeltaLast(s, r, c, dir) - penaltyTable[s] * 2;
      else score += calcScoreDeltaLast(s, r, c, dir) - penaltyTable[s];

      #ifdef DEBUG
      if (iterCount % 100000 == 0) cerr << iterCount << " " << score << " " << lastScore << " " << bestScore << " " << temperature << " " << operation.size() << endl;
      #endif

      if (score <= lastScore) {
        commitRotationLast(s, r, c, dir);
        operation.pop_back();
        lastScore = score;
        if (score < bestScore) {
          bestScore = score;
          best_operation = operation;
        }
      }
      else if (theRandom.nextDouble() < exp(double(lastScore - score) / temperature)) { // accept
        commitRotationLast(s, r, c, dir);
        operation.pop_back();
        lastScore = score;
      }
      else { // rollback
        score = lastScore;
      }
    }
    else if (roll < 0.7) {
      short s = theRandom.nextUShortMod(min(10, n-1)) + 2;
      short r = theRandom.nextUShortMod(n-s+1);
      short c = theRandom.nextUShortMod(n-s+1);
      char dir = !(theRandom.nextUIntMod(4)) ? 'F' : (theRandom.nextUIntMod(2) ? 'L' : 'R');

      if (dir == 'F') score += calcScoreDeltaFirst(s, r, c, dir) + penaltyTable[s] * 2;
      else score += calcScoreDeltaFirst(s, r, c, dir) + penaltyTable[s];

      #ifdef DEBUG
      if (iterCount % 100000 == 0) cerr << iterCount << " " << score << " " << lastScore << " " << bestScore << " " << temperature << " " << operation.size() << endl;
      #endif

      if (score <= lastScore) {
        commitRotationFirst(s, r, c, dir);
        operation.push_front({s, r, c, dir});
        lastScore = score;
        if (score < bestScore) {
          bestScore = score;
          best_operation = operation;
        }
      }
      else if (theRandom.nextDouble() < exp(double(lastScore - score) / temperature)) { // accept
        commitRotationFirst(s, r, c, dir);
        operation.push_front({s, r, c, dir});
        lastScore = score;
      }
      else { // rollback
        score = lastScore;
      }
    }
    else {
      if (operation.size() == 0) continue;
      auto now = operation.front();
      short s = now[0];
      short r = now[1];
      short c = now[2];
      char dir = now[3];
      if (dir == 'L') dir = 'R';
      else if (dir == 'R') dir = 'L';

      if (dir == 'F') score += calcScoreDeltaFirst(s, r, c, dir) - penaltyTable[s] * 2;
      else score += calcScoreDeltaFirst(s, r, c, dir) - penaltyTable[s];

      #ifdef DEBUG
      if (iterCount % 100000 == 0) cerr << iterCount << " " << score << " " << lastScore << " " << bestScore << " " << temperature << " " << operation.size() << endl;
      #endif

      if (score <= lastScore) {
        commitRotationFirst(s, r, c, dir);
        operation.pop_front();
        lastScore = score;
        if (score < bestScore) {
          bestScore = score;
          best_operation = operation;
        }
      }
      else if (theRandom.nextDouble() < exp(double(lastScore - score) / temperature)) { // accept
        commitRotationFirst(s, r, c, dir);
        operation.pop_front();
        lastScore = score;
      }
      else { // rollback
        score = lastScore;
      }
    }

    iterCount++;
    temperature *= 1.0 - decayRate;
  }

  operation = best_operation;

  cerr << "iterCount   = " << iterCount << endl;
  cerr << "temperature = " << temperature << endl;
  cerr << "bestScore   = " << bestScore << endl;

  // postprocess & output
  int operationCount = 0;
  for (auto &x: operation) {
    if (x[3] == 'F') operationCount += 2;
    else operationCount++;
  }
  cout << operationCount << endl;
  for (auto &x: operation) {
    if (x[3] == 'F') {
      cout << x[1] << " " << x[2] << " " << x[0] << " " << 'L' << endl;
      cout << x[1] << " " << x[2] << " " << x[0] << " " << 'L' << endl;
    }
    else cout << x[1] << " " << x[2] << " " << x[0] << " " << (char)(x[3]) << endl;
  }

  return 0;
}
