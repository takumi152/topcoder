#pragma GCC optimize ("O3")
#pragma GCC optimize ("unroll-loops")
#pragma GCC target ("sse4.2,avx")

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <queue>
#include <functional>
#include <numeric>
#include <random>
#include <chrono>
#include <cstring>
#include <climits>
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
    return ((x & 0x000000000000ffff) * mod) >> 16;
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

xorshift64 theRandom;
timer theTimer;

//#define DEBUG

double p;
int n;
vector<int> h, w;
vector<vector<string> > grid_in;

int total_in = 0;
vector<vector<int> > cardinality;
vector<vector<vector<int> > > inDistribution;

int h_out, w_out;
int h_max, w_max;
vector<string> grid_out;
vector<Pii> loc;

vector<string> grid_out_prev;

vector<string> grid_out_ans;
vector<Pii> loc_ans;
double ans_score = 1e308;

void greedyAssign() {
  cardinality = vector<vector<int> >(h_out, vector<int>(w_out));
  inDistribution = vector<vector<vector<int> > >(h_out, vector<vector<int> >(w_out, vector<int>(26)));
  grid_out_prev = grid_out;
  vector<int> assignOrder(n);
  for (int i = 0; i < n; i++) assignOrder[i] = i;
  if (h_out > w_out) {
    sort(assignOrder.rbegin(), assignOrder.rend(), [&](int &a, int &b){return Pii(w[a], h[a]) < Pii(w[b], h[b]);});
    for (int r = 0; r < n; r++) {
      int k = assignOrder[r];
      loc[k] = Pii(-1, -1);
      bool good = true;
      for (int s = 0; s < h_out - h[k] + 1; s++) {
        for (int t = 0; t < w_out - w[k] + 1; t++) {
          good = true;
          for (int i = 0; i < h[k]; i++) {
            for (int j = 0; j < w[k]; j++) {
              if (cardinality[i+s][j+t] > 0) {
                good = false;
                break;
              }
            }
            if (!good) break;
          }
          if (good) {
            loc[k] = Pii(s, t);
            for (int i = 0; i < h[k]; i++) {
              for (int j = 0; j < w[k]; j++) {
                cardinality[i+loc[k].first][j+loc[k].second]++;
                inDistribution[i+loc[k].first][j+loc[k].second][grid_in[k][i][j]-'A']++;
                grid_out[i+loc[k].first][j+loc[k].second] = grid_in[k][i][j];
              }
            }
            break;
          }
        }
        if (good) break;
      }
    }
  }
  else {
    sort(assignOrder.rbegin(), assignOrder.rend(), [&](int &a, int &b){return Pii(h[a], w[a]) < Pii(h[b], w[b]);});
    for (int r = 0; r < n; r++) {
      int k = assignOrder[r];
      loc[k] = Pii(-1, -1);
      bool good = true;
      for (int t = 0; t < w_out - w[k] + 1; t++) {
        for (int s = 0; s < h_out - h[k] + 1; s++) {
          good = true;
          for (int i = 0; i < h[k]; i++) {
            for (int j = 0; j < w[k]; j++) {
              if (cardinality[i+s][j+t] > 0) {
                good = false;
                break;
              }
            }
            if (!good) break;
          }
          if (good) {
            loc[k] = Pii(s, t);
            for (int j = 0; j < w[k]; j++) {
              for (int i = 0; i < h[k]; i++) {
                cardinality[i+loc[k].first][j+loc[k].second]++;
                inDistribution[i+loc[k].first][j+loc[k].second][grid_in[k][i][j]-'A']++;
                grid_out[i+loc[k].first][j+loc[k].second] = grid_in[k][i][j];
              }
            }
            break;
          }
        }
        if (good) break;
      }
    }
  }

  for (int r = 0; r < n; r++) {
    int k = assignOrder[r];
    if (loc[k].first == -1) {
      int bestScore = 1000000007;
      Pii best_loc = Pii(0, 0);
      for (int s = 0; s < h_out - h[k] + 1; s++) {
        for (int t = 0; t < w_out - w[k] + 1; t++) {
          loc[k] = Pii(s, t);
          int score = 0;
          for (int i = 0; i < h[k]; i++) {
            for (int j = 0; j < w[k]; j++) {
              cardinality[i+loc[k].first][j+loc[k].second]++;
              inDistribution[i+loc[k].first][j+loc[k].second][grid_in[k][i][j]-'A']++;
              score += abs(grid_in[k][i][j] - grid_out[i+loc[k].first][j+loc[k].second]);
              int median = (cardinality[i+loc[k].first][j+loc[k].second] + 1) / 2;
              int counted = 0;
              char nextchar = 'A';
              for (; nextchar <= 'Z'; nextchar++) {
                counted += inDistribution[i+loc[k].first][j+loc[k].second][nextchar-'A'];
                if (counted >= median) break;
              }
              for (char c = 'A'; c <= 'Z'; c++) {
                score += (abs(c - nextchar) - abs(c - grid_out[i+loc[k].first][j+loc[k].second])) * inDistribution[i+loc[k].first][j+loc[k].second][c-'A'];
              }
              grid_out_prev[i+loc[k].first][j+loc[k].second] = grid_out[i+loc[k].first][j+loc[k].second];
              grid_out[i+loc[k].first][j+loc[k].second] = nextchar;
            }
          }
          for (int i = 0; i < h[k]; i++) {
            for (int j = 0; j < w[k]; j++) {
              cardinality[i+loc[k].first][j+loc[k].second]--;
              inDistribution[i+loc[k].first][j+loc[k].second][grid_in[k][i][j]-'A']--;
              grid_out[i+loc[k].first][j+loc[k].second] = grid_out_prev[i+loc[k].first][j+loc[k].second];
            }
          }
          if (score < bestScore) {
            bestScore = score;
            best_loc = loc[k];
            if (bestScore <= 0) break;
          }
        }
        if (bestScore <= 0) break;
      }
      loc[k] = best_loc;
      for (int i = 0; i < h[k]; i++) {
        for (int j = 0; j < w[k]; j++) {
          cardinality[i+loc[k].first][j+loc[k].second]++;
          inDistribution[i+loc[k].first][j+loc[k].second][grid_in[k][i][j]-'A']++;
          int median = (cardinality[i+loc[k].first][j+loc[k].second] + 1) / 2;
          int counted = 0;
          char nextchar = 'A';
          for (; nextchar <= 'Z'; nextchar++) {
            counted += inDistribution[i+loc[k].first][j+loc[k].second][nextchar-'A'];
            if (counted >= median) break;
          }
          grid_out_prev[i+loc[k].first][j+loc[k].second] = grid_out[i+loc[k].first][j+loc[k].second];
          grid_out[i+loc[k].first][j+loc[k].second] = nextchar;
        }
      }
    }
  }
}

int calcScore() {
  int score = 0;
  cardinality = vector<vector<int> >(h_out, vector<int>(w_out));
  inDistribution = vector<vector<vector<int> > >(h_out, vector<vector<int> >(w_out, vector<int>(26)));
  grid_out_prev = grid_out;
  for (int k = 0; k < n; k++) {
    for (int i = 0; i < h[k]; i++) {
      for (int j = 0; j < w[k]; j++) {
        score += abs(grid_in[k][i][j] - grid_out[i+loc[k].first][j+loc[k].second]);
        cardinality[i+loc[k].first][j+loc[k].second]++;
        inDistribution[i+loc[k].first][j+loc[k].second][grid_in[k][i][j]-'A']++;
      }
    }
  }
  return score;
}

int updateGrid(int px, int py, char c) {
  int scoreDelta = 0;
  for (char k = 'A'; k <= 'Z'; k++) {
    scoreDelta += (abs(k - c) - abs(k - grid_out[px][py])) * inDistribution[px][py][k-'A'];
  }
  grid_out[px][py] = c;
  return scoreDelta;
}

int moveGrid(int g, int px, int py) {
  int scoreDelta = 0;
  for (int i = 0; i < h[g]; i++) {
    for (int j = 0; j < w[g]; j++) {
      cardinality[i+loc[g].first][j+loc[g].second]--;
      inDistribution[i+loc[g].first][j+loc[g].second][grid_in[g][i][j]-'A']--;
      scoreDelta -= abs(grid_in[g][i][j] - grid_out[i+loc[g].first][j+loc[g].second]);
    }
  }
  loc[g] = Pii(px, py);
  for (int i = 0; i < h[g]; i++) {
    for (int j = 0; j < w[g]; j++) {
      cardinality[i+loc[g].first][j+loc[g].second]++;
      inDistribution[i+loc[g].first][j+loc[g].second][grid_in[g][i][j]-'A']++;
      if (cardinality[i+loc[g].first][j+loc[g].second] == 1) grid_out[i+loc[g].first][j+loc[g].second] = grid_in[g][i][j];
      else scoreDelta += abs(grid_in[g][i][j] - grid_out[i+loc[g].first][j+loc[g].second]);
    }
  }
  return scoreDelta;
}

int moveGridOptimal(int g, int px, int py) {
  int scoreDelta = 0;
  for (int i = 0; i < h[g]; i++) {
    for (int j = 0; j < w[g]; j++) {
      cardinality[i+loc[g].first][j+loc[g].second]--;
      inDistribution[i+loc[g].first][j+loc[g].second][grid_in[g][i][j]-'A']--;
      scoreDelta -= abs(grid_in[g][i][j] - grid_out[i+loc[g].first][j+loc[g].second]);
    }
  }
  loc[g] = Pii(px, py);
  for (int i = 0; i < h[g]; i++) {
    for (int j = 0; j < w[g]; j++) {
      cardinality[i+loc[g].first][j+loc[g].second]++;
      inDistribution[i+loc[g].first][j+loc[g].second][grid_in[g][i][j]-'A']++;
      scoreDelta += abs(grid_in[g][i][j] - grid_out[i+loc[g].first][j+loc[g].second]);
      int median = (cardinality[i+loc[g].first][j+loc[g].second] + 1) / 2;
      int counted = 0;
      char nextchar = 'A';
      for (; nextchar <= 'Z'; nextchar++) {
        counted += inDistribution[i+loc[g].first][j+loc[g].second][nextchar-'A'];
        if (counted >= median) break;
      }
      for (char k = 'A'; k <= 'Z'; k++) {
        scoreDelta += (abs(k - nextchar) - abs(k - grid_out[i+loc[g].first][j+loc[g].second])) * inDistribution[i+loc[g].first][j+loc[g].second][k-'A'];
      }
      grid_out_prev[i+loc[g].first][j+loc[g].second] = grid_out[i+loc[g].first][j+loc[g].second];
      grid_out[i+loc[g].first][j+loc[g].second] = nextchar;
    }
  }
  return scoreDelta;
}

int revertOptimalMove(int g, int ox, int oy) {
  int scoreDelta = 0;
  for (int i = 0; i < h[g]; i++) {
    for (int j = 0; j < w[g]; j++) {
      cardinality[i+loc[g].first][j+loc[g].second]--;
      inDistribution[i+loc[g].first][j+loc[g].second][grid_in[g][i][j]-'A']--;
      scoreDelta -= abs(grid_in[g][i][j] - grid_out[i+loc[g].first][j+loc[g].second]);
      for (char k = 'A'; k <= 'Z'; k++) {
        scoreDelta += (abs(k - grid_out_prev[i+loc[g].first][j+loc[g].second]) - abs(k - grid_out[i+loc[g].first][j+loc[g].second])) * inDistribution[i+loc[g].first][j+loc[g].second][k-'A'];
      }
      grid_out[i+loc[g].first][j+loc[g].second] = grid_out_prev[i+loc[g].first][j+loc[g].second];
    }
  }
  loc[g] = Pii(ox, oy);
  for (int i = 0; i < h[g]; i++) {
    for (int j = 0; j < w[g]; j++) {
      cardinality[i+loc[g].first][j+loc[g].second]++;
      inDistribution[i+loc[g].first][j+loc[g].second][grid_in[g][i][j]-'A']++;
      scoreDelta += abs(grid_in[g][i][j] - grid_out[i+loc[g].first][j+loc[g].second]);
    }
  }
  return scoreDelta;
}

double optimizeGreedy() {
  grid_out = vector<string>();
  for (int i = 0; i < h_out; i++) {
    grid_out.push_back("");
    for (int j = 0; j < w_out; j++) {
      char c = theRandom.nextUIntMod(26) + 'A';
      grid_out[i].push_back(c);
    }
  }
  cardinality = vector<vector<int> >(h_out, vector<int>(w_out));
  inDistribution = vector<vector<vector<int> > >(h_out, vector<vector<int> >(w_out, vector<int>(26)));
  grid_out_prev = grid_out;

  loc = vector<Pii>(n);
  for (int i = 0; i < n; i++) {
    int px = theRandom.nextUIntMod(h_out - h[i] + 1);
    int py = theRandom.nextUIntMod(w_out - w[i] + 1);
    loc[i] = Pii(px, py);
  }

  greedyAssign();

  int score = calcScore();
  int bestScore = score;

  double compressionScore = (double) (h_out * w_out) / (double) total_in;
  double lossinessScore = (double) bestScore / ((double) total_in * 12.5);
  double finalScore = compressionScore * p + lossinessScore * (1 - p);

  return finalScore;
}

double optimize1sec() {
  grid_out = vector<string>();
  for (int i = 0; i < h_out; i++) {
    grid_out.push_back("");
    for (int j = 0; j < w_out; j++) {
      char c = theRandom.nextUIntMod(26) + 'A';
      grid_out[i].push_back(c);
    }
  }
  cardinality = vector<vector<int> >(h_out, vector<int>(w_out));
  inDistribution = vector<vector<vector<int> > >(h_out, vector<vector<int> >(w_out, vector<int>(26)));
  grid_out_prev = grid_out;

  loc = vector<Pii>(n);
  for (int i = 0; i < n; i++) {
    int px = theRandom.nextUIntMod(h_out - h[i] + 1);
    int py = theRandom.nextUIntMod(w_out - w[i] + 1);
    loc[i] = Pii(px, py);
  }

  greedyAssign();

  int score = calcScore();
  int lastScore = score;
  int bestScore = score;
  vector<string> best_grid_out = grid_out;
  vector<Pii> best_loc = loc;

  double baseTemperature = 5e0;
  double temperature = baseTemperature;
  double decayRate = 5e-6;
  double timeLimit = theTimer.time() + 0.500;
  int iterCount = 0;

  while (theTimer.time() < timeLimit) {
    double roll = theRandom.nextDouble();
    if (roll < 0.5) {
      int px = theRandom.nextUIntMod(h_out);
      int py = theRandom.nextUIntMod(w_out);
      char c = theRandom.nextUIntMod(26) + 'A';
      if (grid_out[px][py] == c) continue;

      char cb = grid_out[px][py];

      score += updateGrid(px, py, c);

      #ifdef DEBUG
      if (iterCount % 100000 == 0) cerr << iterCount << " " << score << " " << lastScore << " " << bestScore << " " << temperature << " " << endl;
      #endif

      if (score <= lastScore) {
        lastScore = score;
        if (score < bestScore) {
          bestScore = score;
          best_grid_out = grid_out;
          best_loc = loc;
        }
      }
      else if (theRandom.nextDouble() < exp(double(lastScore - score) / temperature)) { // accept
        lastScore = score;
      }
      else { // rollback
        score += updateGrid(px, py, cb);
      }
    }
    else {
      int g = theRandom.nextUIntMod(n);
      int px = theRandom.nextUIntMod(h_out - h[g] + 1);
      int py = theRandom.nextUIntMod(w_out - w[g] + 1);
      if (px == loc[g].first && py == loc[g].second) continue;

      int ox = loc[g].first;
      int oy = loc[g].second;

      score += moveGrid(g, px, py);

      #ifdef DEBUG
      if (iterCount % 100000 == 0) cerr << iterCount << " " << score << " " << lastScore << " " << bestScore << " " << temperature << " " << endl;
      #endif

      if (score <= lastScore) {
        lastScore = score;
        if (score < bestScore) {
          bestScore = score;
          best_grid_out = grid_out;
          best_loc = loc;
        }
      }
      else if (theRandom.nextDouble() < exp(double(lastScore - score) / temperature)) { // accept
        lastScore = score;
      }
      else { // rollback
        score += moveGrid(g, ox, oy);
      }
    }

    iterCount++;
    temperature *= 1.0 - decayRate;
    if (bestScore == 0) break;
  }

  grid_out = best_grid_out;
  loc = best_loc;

  double compressionScore = (double) (h_out * w_out) / (double) total_in;
  double lossinessScore = (double) bestScore / ((double) total_in * 12.5);
  double finalScore = compressionScore * p + lossinessScore * (1 - p);

  return finalScore;
}

double optimizeHalf() {
  grid_out = vector<string>();
  for (int i = 0; i < h_out; i++) {
    grid_out.push_back("");
    for (int j = 0; j < w_out; j++) {
      char c = theRandom.nextUIntMod(26) + 'A';
      grid_out[i].push_back(c);
    }
  }
  cardinality = vector<vector<int> >(h_out, vector<int>(w_out));
  inDistribution = vector<vector<vector<int> > >(h_out, vector<vector<int> >(w_out, vector<int>(26)));
  grid_out_prev = grid_out;

  loc = vector<Pii>(n);
  for (int i = 0; i < n; i++) {
    int px = theRandom.nextUIntMod(h_out - h[i] + 1);
    int py = theRandom.nextUIntMod(w_out - w[i] + 1);
    loc[i] = Pii(px, py);
  }

  greedyAssign();

  int score = calcScore();
  int lastScore = score;
  int bestScore = score;
  vector<string> best_grid_out = grid_out;
  vector<Pii> best_loc = loc;

  double baseTemperature = min(1e1, 5e-2 * score);
  double temperature = baseTemperature;
  double decayRate = 5e-7;
  double timeLimit = theTimer.time() + (9.900 - theTimer.time()) / 2;
  int iterCount = 0;

  while (theTimer.time() < timeLimit) {
    double roll = theRandom.nextDouble();
    if (roll < 0.5) {
      int px = theRandom.nextUIntMod(h_out);
      int py = theRandom.nextUIntMod(w_out);
      char c = theRandom.nextUIntMod(26) + 'A';
      if (grid_out[px][py] == c) continue;

      char cb = grid_out[px][py];

      score += updateGrid(px, py, c);

      #ifdef DEBUG
      if (iterCount % 100000 == 0) cerr << iterCount << " " << score << " " << lastScore << " " << bestScore << " " << temperature << " " << endl;
      #endif

      if (score <= lastScore) {
        lastScore = score;
        if (score < bestScore) {
          bestScore = score;
          best_grid_out = grid_out;
          best_loc = loc;
        }
      }
      else if (theRandom.nextDouble() < exp(double(lastScore - score) / temperature)) { // accept
        lastScore = score;
      }
      else { // rollback
        score += updateGrid(px, py, cb);
      }
    }
    else {
      int g = theRandom.nextUIntMod(n);
      int px = theRandom.nextUIntMod(h_out - h[g] + 1);
      int py = theRandom.nextUIntMod(w_out - w[g] + 1);
      if (px == loc[g].first && py == loc[g].second) continue;

      int ox = loc[g].first;
      int oy = loc[g].second;

      score += moveGrid(g, px, py);

      #ifdef DEBUG
      if (iterCount % 100000 == 0) cerr << iterCount << " " << score << " " << lastScore << " " << bestScore << " " << temperature << " " << endl;
      #endif

      if (score <= lastScore) {
        lastScore = score;
        if (score < bestScore) {
          bestScore = score;
          best_grid_out = grid_out;
          best_loc = loc;
        }
      }
      else if (theRandom.nextDouble() < exp(double(lastScore - score) / temperature)) { // accept
        lastScore = score;
      }
      else { // rollback
        score += moveGrid(g, ox, oy);
      }
    }

    iterCount++;
    temperature *= 1.0 - decayRate;
    if (bestScore == 0) break;
  }

  grid_out = best_grid_out;
  loc = best_loc;

  cerr << "iterCount   = " << iterCount << endl;
  cerr << "temperature = " << temperature << endl;
  cerr << "bestScore   = " << bestScore << endl;

  double compressionScore = (double) (h_out * w_out) / (double) total_in;
  double lossinessScore = (double) bestScore / ((double) total_in * 12.5);
  double finalScore = compressionScore * p + lossinessScore * (1 - p);

  return finalScore;
}

double optimizeFull() {
  grid_out = vector<string>();
  for (int i = 0; i < h_out; i++) {
    grid_out.push_back("");
    for (int j = 0; j < w_out; j++) {
      char c = theRandom.nextUIntMod(26) + 'A';
      grid_out[i].push_back(c);
    }
  }
  cardinality = vector<vector<int> >(h_out, vector<int>(w_out));
  inDistribution = vector<vector<vector<int> > >(h_out, vector<vector<int> >(w_out, vector<int>(26)));
  grid_out_prev = grid_out;

  loc = vector<Pii>(n);
  for (int i = 0; i < n; i++) {
    int px = theRandom.nextUIntMod(h_out - h[i] + 1);
    int py = theRandom.nextUIntMod(w_out - w[i] + 1);
    loc[i] = Pii(px, py);
  }

  greedyAssign();

  int score = calcScore();
  int lastScore = score;
  int bestScore = score;
  vector<string> best_grid_out = grid_out;
  vector<Pii> best_loc = loc;

  double baseTemperature = min(1e1, 5e-2 * score);
  double temperature = baseTemperature;
  double decayRate = 5e-7;
  double timeLimit = 9.900;
  int iterCount = 0;

  while (theTimer.time() < timeLimit) {
    double roll = theRandom.nextDouble();
    if (roll < 0.5) {
      int px = theRandom.nextUIntMod(h_out);
      int py = theRandom.nextUIntMod(w_out);
      char c = theRandom.nextUIntMod(26) + 'A';
      if (grid_out[px][py] == c) continue;

      char cb = grid_out[px][py];

      score += updateGrid(px, py, c);

      #ifdef DEBUG
      if (iterCount % 100000 == 0) cerr << iterCount << " " << score << " " << lastScore << " " << bestScore << " " << temperature << " " << endl;
      #endif

      if (score <= lastScore) {
        lastScore = score;
        if (score < bestScore) {
          bestScore = score;
          best_grid_out = grid_out;
          best_loc = loc;
        }
      }
      else if (theRandom.nextDouble() < exp(double(lastScore - score) / temperature)) { // accept
        lastScore = score;
      }
      else { // rollback
        score += updateGrid(px, py, cb);
      }
    }
    else {
      int g = theRandom.nextUIntMod(n);
      int px = theRandom.nextUIntMod(h_out - h[g] + 1);
      int py = theRandom.nextUIntMod(w_out - w[g] + 1);
      if (px == loc[g].first && py == loc[g].second) continue;

      int ox = loc[g].first;
      int oy = loc[g].second;

      score += moveGrid(g, px, py);

      #ifdef DEBUG
      if (iterCount % 100000 == 0) cerr << iterCount << " " << score << " " << lastScore << " " << bestScore << " " << temperature << " " << endl;
      #endif

      if (score <= lastScore) {
        lastScore = score;
        if (score < bestScore) {
          bestScore = score;
          best_grid_out = grid_out;
          best_loc = loc;
        }
      }
      else if (theRandom.nextDouble() < exp(double(lastScore - score) / temperature)) { // accept
        lastScore = score;
      }
      else { // rollback
        score += moveGrid(g, ox, oy);
      }
    }

    iterCount++;
    temperature *= 1.0 - decayRate;
    if (bestScore == 0) break;
  }

  grid_out = best_grid_out;
  loc = best_loc;

  cerr << "iterCount   = " << iterCount << endl;
  cerr << "temperature = " << temperature << endl;
  cerr << "bestScore   = " << bestScore << endl;

  double compressionScore = (double) (h_out * w_out) / (double) total_in;
  double lossinessScore = (double) bestScore / ((double) total_in * 12.5);
  double finalScore = compressionScore * p + lossinessScore * (1 - p);

  return finalScore;
}

int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);

  cin >> p;
  cin >> n;
  h = vector<int>(n);
  w = vector<int>(n);
  grid_in = vector<vector<string> >(n);
  for (int i = 0; i < n; i++) {
    cin >> h[i];
    grid_in[i] = vector<string>(h[i]);
    for (int j = 0; j < h[i]; j++) cin >> grid_in[i][j];
    w[i] = grid_in[i][0].length();
  }

  h_max = h[0];
  w_max = w[0];
  for (int i = 0; i < n; i++) {
    h_max = max(h_max, h[i]);
    w_max = max(w_max, w[i]);
    total_in += h[i] * w[i];
  }
  h_out = h_max;
  w_out = w_max;

  vector<pair<double, Pii> > scoreList;
  const double phi = (1.0 + sqrt(5.0)) / 2.0;

  // vertical
  w_out = w_max + 2;
  double low = h_max - 4;
  double high = max(h_max, (total_in * 2) / w_max);
  double midlow = max((double) h_max, ((phi * low + high) / (phi + 1.0)));
  double midhigh = ((phi * high + low) / (phi + 1.0));

  h_out = midlow;
  double score_midlow = optimizeGreedy();
  h_out = midhigh;
  double score_midhigh = optimizeGreedy();
  while (midlow+1 < midhigh) {
    if (score_midlow < 0) score_midlow = optimizeGreedy();
    if (score_midhigh < 0) score_midhigh = optimizeGreedy();
    if (score_midlow > score_midhigh) {
      low = midlow;
      midlow = midhigh;
      midhigh = ((phi * high + low) / (phi + 1.0));
      score_midlow = score_midhigh;
      h_out = midhigh;
      score_midhigh = -1.0;
    }
    else {
      high = midhigh;
      midhigh = midlow;
      midlow = max((double) h_max, ((phi * low + high) / (phi + 1.0)));
      score_midhigh = score_midlow;
      h_out = midlow;
      score_midlow = -1.0;
    }
  }

  for (int i = max(h_max, (int)((midlow + midhigh) / 2) - 10); i <= (int)((midlow + midhigh) / 2) + 10; i++) {
    h_out = i;
    double score_test = optimizeGreedy();
    scoreList.emplace_back(score_test, Pii(h_out, w_out));
  }

  // horizontal
  h_out = h_max + 2;
  low = h_max - 4;
  high = max(w_max, (total_in * 2) / h_max);
  midlow = max((double) w_max, ((phi * low + high) / (phi + 1.0)));
  midhigh = ((phi * high + low) / (phi + 1.0));

  w_out = midlow;
  score_midlow = optimizeGreedy();
  w_out = midhigh;
  score_midhigh = optimizeGreedy();
  while (midlow+1 < midhigh) {
    if (score_midlow < 0) score_midlow = optimizeGreedy();
    if (score_midhigh < 0) score_midhigh = optimizeGreedy();
    if (score_midlow > score_midhigh) {
      low = midlow;
      midlow = midhigh;
      midhigh = ((phi * high + low) / (phi + 1.0));
      score_midlow = score_midhigh;
      w_out = midhigh;
      score_midhigh = -1.0;
    }
    else {
      high = midhigh;
      midhigh = midlow;
      midlow = max((double) w_max, ((phi * low + high) / (phi + 1.0)));
      score_midhigh = score_midlow;
      w_out = midlow;
      score_midlow = -1.0;
    }
  }

  for (int i = max(w_max, (int)((midlow + midhigh) / 2) - 10); i <= (int)((midlow + midhigh) / 2) + 10; i++) {
    w_out = i;
    double score_test = optimizeGreedy();
    scoreList.emplace_back(score_test, Pii(h_out, w_out));
  }

  // minimum
  h_out = h_max;
  w_out = w_max;
  double score_minimum = optimizeGreedy();
  scoreList.emplace_back(score_minimum, Pii(h_out, w_out));

  sort(scoreList.begin(), scoreList.end());

  h_out = scoreList[0].second.first;
  w_out = scoreList[0].second.second;
  cerr << "h_out       = " << h_out << endl;
  cerr << "w_out       = " << w_out << endl;
  ans_score = optimizeHalf();
  cerr << "finalscore  = " << ans_score << endl;
  grid_out_ans = grid_out;
  loc_ans = loc;
  for (int i = 1; i < (int) scoreList.size(); i++) {
    h_out = scoreList[i].second.first;
    w_out = scoreList[i].second.second;
    cerr << "h_out       = " << h_out << endl;
    cerr << "w_out       = " << w_out << endl;
    double finalScore = optimizeFull();
    cerr << "finalscore  = " << finalScore << endl;
    if (finalScore < ans_score) {
      ans_score = finalScore;
      grid_out_ans = grid_out;
      loc_ans = loc;
    }
    if (theTimer.time() > 9.000) break;
  }

  cerr << "ans_score   = " << ans_score << endl;

  cout << grid_out_ans.size() << endl;
  for (auto &x: grid_out_ans) cout << x << endl;
  for (auto &x: loc_ans) cout << x.first << " " << x.second << endl;

  return 0;
}
