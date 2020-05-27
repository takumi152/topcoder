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

//#define DEBUG

int n, k, p;
double c;
vector<vector<int> > clue;

double edgeExpected;
int edgeCount;

vector<unordered_set<int> > edge;
vector<unordered_set<int> > bestEdge;
vector<vector<int> > dist;

vector<vector<int> > lastDist;
queue<Pii> lastEdited;

vector<unordered_set<int> > lockedEdge;

void calcDist() {
  for (int i = 0; i < n; i++) {
    dist[i] = vector<int>(n, -1);
    queue<Pii> que;
    que.emplace(i, 0);
    while (!que.empty()) {
      auto now = que.front();
      que.pop();
      if (dist[i][now.first] != -1) continue;
      dist[i][now.first] = now.second;
      for (auto &x: edge[now.first]) {
        if (dist[i][x] == -1) que.emplace(x, now.second+1);
      }
    }
  }
}

void checkDist() {
  for (int i = 0; i < n; i++) {
    vector<int> distRef = vector<int>(n, -1);
    queue<Pii> que;
    que.emplace(i, 0);
    while (!que.empty()) {
      auto now = que.front();
      que.pop();
      if (distRef[now.first] != -1) continue;
      distRef[now.first] = now.second;
      for (auto &x: edge[now.first]) {
        if (distRef[x] == -1) que.emplace(x, now.second+1);
      }
    }
    for (int j = 0; j < n; j++) assert(distRef[j] == dist[i][j]);
  }
}

void calcDistPartial(int v1, int v2) {
  if (edge[v1].find(v2) != edge[v1].end()) { // added
    for (int i = 0; i < n; i++) {
      if (dist[i][v1] == -1 && dist[i][v2] == -1) continue; // unconnected to both
      else if (dist[i][v1] == -1) { // unconnected to v1 only
        queue<Pii> que;
        que.emplace(v1, dist[i][v2]+1);
        while (!que.empty()) {
          auto now = que.front();
          que.pop();
          if (dist[i][now.first] != -1) continue;
          dist[i][now.first] = now.second;
          lastEdited.emplace(i, now.first);
          for (auto &x: edge[now.first]) {
            if (dist[i][x] == -1) que.emplace(x, now.second+1);
          }
        }
      }
      else if (dist[i][v2] == -1) { // unconnected to v2 only
        queue<Pii> que;
        que.emplace(v2, dist[i][v1]+1);
        while (!que.empty()) {
          auto now = que.front();
          que.pop();
          if (dist[i][now.first] != -1) continue;
          dist[i][now.first] = now.second;
          lastEdited.emplace(i, now.first);
          for (auto &x: edge[now.first]) {
            if (dist[i][x] == -1) que.emplace(x, now.second+1);
          }
        }
      }
      else if (abs(dist[i][v1] - dist[i][v2]) <= 1) continue; // unaffected by addition
      else if (dist[i][v1] > dist[i][v2]) { // path to v1 becomes shorter
        queue<Pii> que;
        que.emplace(v1, dist[i][v2]+1);
        while (!que.empty()) {
          auto now = que.front();
          que.pop();
          if (now.second >= dist[i][now.first]) continue;
          dist[i][now.first] = now.second;
          lastEdited.emplace(i, now.first);
          for (auto &x: edge[now.first]) {
            if (now.second+1 < dist[i][x]) que.emplace(x, now.second+1);
          }
        }
      }
      else { // path to v2 becomes shorter
        queue<Pii> que;
        que.emplace(v2, dist[i][v1]+1);
        while (!que.empty()) {
          auto now = que.front();
          que.pop();
          if (now.second >= dist[i][now.first]) continue;
          dist[i][now.first] = now.second;
          lastEdited.emplace(i, now.first);
          for (auto &x: edge[now.first]) {
            if (now.second+1 < dist[i][x]) que.emplace(x, now.second+1);
          }
        }
      }
    }
  }
  else { // removed
    for (int i = 0; i < n; i++) {
      if (dist[i][v1] <= 0 && dist[i][v2] <= 0) continue; // unconnected to both, or have same distances
      else if (dist[i][v1] > dist[i][v2]) { // one of the shortest paths to v1 was removed
        queue<Pii> que;
        for (int j = 0; j < n; j++) {
          if (dist[i][j] > dist[i][v2]) {
            dist[i][j] = -1;
            lastEdited.emplace(i, j);
          }
          else if (dist[i][j] == dist[i][v2]) que.emplace(j, dist[i][v2]);
        }
        while (!que.empty()) {
          auto now = que.front();
          que.pop();
          if (dist[i][now.first] != -1 && dist[i][now.first] != dist[i][v2]) continue;
          dist[i][now.first] = now.second;
          for (auto &x: edge[now.first]) {
            if (dist[i][x] == -1) que.emplace(x, now.second+1);
          }
        }
      }
      else { // one of the shortest paths to v2 was removed
        queue<Pii> que;
        for (int j = 0; j < n; j++) {
          if (dist[i][j] > dist[i][v1]) {
            dist[i][j] = -1;
            lastEdited.emplace(i, j);
          }
          else if (dist[i][j] == dist[i][v1]) que.emplace(j, dist[i][v1]);
        }
        while (!que.empty()) {
          auto now = que.front();
          que.pop();
          if (dist[i][now.first] != -1 && dist[i][now.first] != dist[i][v1]) continue;
          dist[i][now.first] = now.second;
          for (auto &x: edge[now.first]) {
            if (dist[i][x] == -1) que.emplace(x, now.second+1);
          }
        }
      }
    }
  }
}

void commitDist() {
  while (!lastEdited.empty()) {
    auto now = lastEdited.front();
    lastEdited.pop();
    lastDist[now.first][now.second] = dist[now.first][now.second];
  }
}

void rollbackDist() {
  while (!lastEdited.empty()) {
    auto now = lastEdited.front();
    lastEdited.pop();
    dist[now.first][now.second] = lastDist[now.first][now.second];
  }
}

int calcScore() {
  int match = 0;
  for (int i = 0; i < p; i++) {
    if (dist[clue[i][0]][clue[i][1]] == clue[i][2]) match++;
  }
  return match;
}

int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);

  // input
  cin >> n;
  cin >> c;
  cin >> k;
  cin >> p;
  clue = vector<vector<int> >(p, vector<int>(3));
  for (int i = 0; i < p; i++) {
    cin >> clue[i][0] >> clue[i][1] >> clue[i][2];
  }

  // preprocessing
  sort(clue.begin(), clue.end());
  edgeExpected = c * (n * (n-1) / 2);
  edge = vector<unordered_set<int> >(n);
  lockedEdge = vector<unordered_set<int> >(n);
  edgeCount = 0;
  for (int i = 0; i < p; i++) {
    if (clue[i][2] == 1) {
      edge[clue[i][0]].insert(clue[i][1]);
      edge[clue[i][1]].insert(clue[i][0]);
      edgeCount++;
      lockedEdge[clue[i][0]].insert(clue[i][1]);
      lockedEdge[clue[i][1]].insert(clue[i][0]);
    }
    else {
      lockedEdge[clue[i][0]].insert(clue[i][1]);
      lockedEdge[clue[i][1]].insert(clue[i][0]);
    }
  }
  dist = vector<vector<int> >(n, vector<int>(n, -1));
  calcDist();
  for (int i = 0; i < p; i++) {
    if (clue[i][2] == -1) {
      for (int j = 0; j < n; j++) {
        if (dist[clue[i][0]][j] >= 0) {
          lockedEdge[clue[i][1]].insert(j);
          lockedEdge[j].insert(clue[i][1]);
        }
        if (dist[clue[i][1]][j] >= 0) {
          lockedEdge[clue[i][0]].insert(j);
          lockedEdge[j].insert(clue[i][0]);
        }
      }
    }
    else if (clue[i][2] >= 2) {
      for (int j = 0; j < n; j++) {
        if (dist[clue[i][0]][j] < clue[i][2] - 1 && dist[clue[i][0]][j] != -1) {
          lockedEdge[clue[i][1]].insert(j);
          lockedEdge[j].insert(clue[i][1]);
        }
        if (dist[clue[i][1]][j] < clue[i][2] - 1 && dist[clue[i][1]][j] != -1) {
          lockedEdge[clue[i][0]].insert(j);
          lockedEdge[j].insert(clue[i][0]);
        }
      }
    }
  }
  calcDist();
  bestEdge = edge;
  lastDist = dist;

  theRandom.x = ((ll) random_device()() << 32) | (ll) random_device()();

  // optimization
  int score = calcScore();
  int lastScore = score;
  int bestScore = score;
  int bestEdgeCount = edgeCount;

  double baseTemperature = 5e-1;
  double temperature = baseTemperature;
  double timeLimit = 9.900;
  int iterCount = 0;

  while (theTimer.time() < timeLimit) {
    int v1 = theRandom.nextUIntMod(n);
    int v2 = theRandom.nextUIntMod(n);
    if (v1 == v2) continue;
    if (lockedEdge[v1].find(v2) != lockedEdge[v1].end()) continue;

    if (edge[v1].find(v2) == edge[v1].end()) {
      if (theRandom.nextDouble() > pow(1.1, edgeExpected - edgeCount - 10)) continue;
      edge[v1].insert(v2);
      edge[v2].insert(v1);
      edgeCount++;
    }
    else {
      edge[v1].erase(v2);
      edge[v2].erase(v1);
      edgeCount--;
    }

    calcDistPartial(v1, v2);
    //checkDist();
    score = calcScore();

    #ifdef DEBUG
    if (iterCount % 1000 == 0) cerr << iterCount << " " << score << " " << lastScore << " " << bestScore << " " << temperature << " " << edgeCount << " " << edgeExpected << endl;
    #endif

    if (score >= lastScore) {
      lastScore = score;
      if (score > bestScore || (score == bestScore && abs(edgeExpected - edgeCount) < abs(edgeExpected - bestEdgeCount))) {
        bestEdge = edge;
        bestEdgeCount = edgeCount;
        bestScore = score;
      }
      commitDist();
    }
    else if (theRandom.nextDouble() < exp(double(score - lastScore) / temperature)) { // accept
      lastScore = score;
      commitDist();
    }
    else { // rollback
      if (edge[v1].find(v2) == edge[v1].end()) {
        edge[v1].insert(v2);
        edge[v2].insert(v1);
        edgeCount++;
      }
      else {
        edge[v1].erase(v2);
        edge[v2].erase(v1);
        edgeCount--;
      }
      rollbackDist();
    }
    //checkDist();

    iterCount++;
    temperature = baseTemperature * ((timeLimit - theTimer.time()) / timeLimit) + 0.1;
  }

  cerr << "iterCount     = " << iterCount << endl;
  cerr << "temperature   = " << temperature << endl;
  cerr << "bestScore     = " << bestScore << endl;
  cerr << "clueCount     = " << p << endl;
  cerr << "bestEdgeCount = " << bestEdgeCount << endl;
  cerr << "edgeExpected  = " << edgeExpected << endl;

  // postprocess & output
  vector<string> ans(n);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      if (edge[i].find(j) == edge[i].end()) ans[i] += '0';
      else ans[i] += '1';
    }
  }

  cout << n << endl;
  for (auto &x: ans) cout << x << endl;

  return 0;
}
