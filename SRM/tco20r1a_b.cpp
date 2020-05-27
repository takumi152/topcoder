#pragma GCC optimize ("O3")
#pragma GCC optimize ("unroll-loops")
#pragma GCC target ("sse4.2")

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <utility>
#include <string>
#include <queue>
#include <stack>
#include <unordered_set>
#include <random>
#include <cmath>
#include <cassert>

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

using namespace std;

typedef long long int ll;
typedef pair<int, int> Pii;

const ll mod = 1000000007;

timer theTimer;
xorshift64 theRandom;

#define DEBUG

class ThreeNeighbors {
private:
  int calcScore(vector<string> &board) {
    int score = 0;
    for (int i = 0; i < 50; i++) {
      for (int j = 0; j < 50; j++) {
        if (board[i][j] == 'X') continue;
        int alive = 0;
        for (int k = max(0, i-1); k < min(50, i+2); k++) {
          for (int l = max(0, j-1); l < min(50, j+2); l++) {
            if (board[k][l] == 'X' && !(i == k && j == l)) alive++;
          }
        }
        if (alive == 3) score++;
      }
    }
    return score;
  }
public:
  vector<string> construct(int n) {
    // preprocessing
    vector<string> ans(50);
    for (int i = 0; i < 50; i++) {
      for (int j = 0; j < 50; j++) ans[i].push_back('.');
    }
    vector<string> best_ans = ans;

    // optimization
    int score = n;
    int lastScore = score;
    int bestScore = score;

    double baseTemperature = 1e1;
    double temperature = baseTemperature;
    double decayRate = 1e-3;
    double timeLimit = 1.900;
    int iterCount = 0;

    while (theTimer.time() < timeLimit) {
      int p1 = theRandom.nextUIntMod(50);
      int p2 = theRandom.nextUIntMod(50);

      if (ans[p1][p2] == '.') ans[p1][p2] = 'X';
      else ans[p1][p2] = '.';

      score = abs(n - calcScore(ans));

      #ifdef DEBUG
      if (iterCount % 10000 == 0) cerr << iterCount << " " << score << " " << lastScore << " " << bestScore << " " << temperature << " " << endl;
      #endif

      if (score <= lastScore) {
        lastScore = score;
        if (score < bestScore) {
          bestScore = score;
          best_ans = ans;
        }
      }
      else if (theRandom.nextDouble() < exp(double(lastScore - score) / temperature)) { // accept
        lastScore = score;
      }
      else { // rollback
        if (ans[p1][p2] == '.') ans[p1][p2] = 'X';
        else ans[p1][p2] = '.';
      }

      iterCount++;
      temperature *= 1.0 - decayRate;
    }

    cerr << "iterCount   = " << iterCount << endl;
    cerr << "temperature = " << temperature << endl;
    cerr << "bestScore   = " << bestScore << endl;

    ans = best_ans;

    // postprocess & output
    return ans;
  }
};

// driver main
int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);

  int n;
  cin >> n;

  ThreeNeighbors solver;
  auto ans = solver.construct(n);

  for (auto &x: ans) cout << x << endl;
}
