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
#include <set>
#include <cmath>

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

int n, v;
string s;

vector<int> snake;

vector<vector<int> > board;
vector<vector<int> > placement;
vector<vector<int> > bestPlacement;

vector<int> p1, p2, p3;

const vector<vector<int> > scoretable = {{0,  0,   0,    0,     0},
                                         {1,  1,   1,    1,     1},
                                         {2,  4,   8,   16,    32},
                                         {3,  9,  27,   81,   243},
                                         {4, 16,  64,  256,  1024},
                                         {5, 25, 125,  625,  3125},
                                         {6, 36, 216, 1296,  7776},
                                         {7, 49, 343, 2401, 16807},
                                         {8, 64, 512, 4096, 32768},
                                         {9, 81, 729, 6561, 59049}};

void makePlacement() {
  int p = 0;
  int r = 1;
  int c = 1;
  for (int i = 0; i < p1.size(); i++) {
    for (int j = 0; j < p1[i]; j++) {
      placement[r][c] = p;
      c++;
      p++;
    }
    placement[r][c] = p;
    r++;
    p++;
    for (int j = 0; j < p1[i]; j++) {
      placement[r][c] = p;
      c--;
      p++;
    }
    placement[r][c] = p;
    r++;
    p++;
  }
  if ((n+1)%4 == 0) {
    for (int j = 0; j < n-2; j++) {
      placement[r][c] = p;
      c++;
      p++;
    }
  }
  else {
    for (int j = 0; j < n-2; j += 2) {
      placement[r][c] = p;
      r++;
      p++;
      placement[r][c] = p;
      c++;
      p++;
      placement[r][c] = p;
      r--;
      p++;
      if (j >= n-3) break;
      placement[r][c] = p;
      c++;
      p++;
    }
  }
  placement[r][c] = p;
  r--;
  p++;
  for (int i = p1.size()-1; i >= 0; i--) {
    for (int j = n-3; j > p1[i]; j--) {
      placement[r][c] = p;
      c--;
      p++;
    }
    placement[r][c] = p;
    r--;
    p++;
    for (int j = n-3; j > p1[i]; j--) {
      placement[r][c] = p;
      c++;
      p++;
    }
    placement[r][c] = p;
    if (i > 0) r--;
    else c++;
    p++;
  }
  for (int i = 0; i < n/2; i++) {
    placement[r][c] = p;
    r++;
    p++;
  }
  for (int i = 0; i < p2.size(); i++) {
    for (int j = 0; j < p2[i]; j++) {
      placement[r][c] = p;
      r++;
      p++;
    }
    placement[r][c] = p;
    c--;
    p++;
    for (int j = 0; j < p2[i]; j++) {
      placement[r][c] = p;
      r--;
      p++;
    }
    placement[r][c] = p;
    c--;
    p++;
  }
  if ((n+1)%4 == 0) {
    for (int i = 0; i < n/2-1; i++) {
      placement[r][c] = p;
      r++;
      p++;
    }
  }
  else {
    for (int j = 0; j < n/2-1; j += 2) {
      placement[r][c] = p;
      c--;
      p++;
      placement[r][c] = p;
      r++;
      p++;
      placement[r][c] = p;
      c++;
      p++;
      if (j >= n/2-2) break;
      placement[r][c] = p;
      r++;
      p++;
    }
  }
  placement[r][c] = p;
  c++;
  p++;
  for (int i = p2.size()-1; i >= 0; i--) {
    for (int j = n/2-2; j > p2[i]; j--) {
      placement[r][c] = p;
      r--;
      p++;
    }
    placement[r][c] = p;
    c++;
    p++;
    for (int j = n/2-2; j > p2[i]; j--) {
      placement[r][c] = p;
      r++;
      p++;
    }
    placement[r][c] = p;
    if (i > 0) c++;
    else r++;
    p++;
  }
  for (int i = 0; i < n/2; i++) {
    placement[r][c] = p;
    c--;
    p++;
  }
  for (int i = 0; i < p3.size(); i++) {
    for (int j = 0; j < p3[i]; j++) {
      placement[r][c] = p;
      c--;
      p++;
    }
    placement[r][c] = p;
    r--;
    p++;
    for (int j = 0; j < p3[i]; j++) {
      placement[r][c] = p;
      c++;
      p++;
    }
    placement[r][c] = p;
    r--;
    p++;
  }
  if ((n+1)%4 == 0) {
    for (int i = 0; i < n/2-1; i++) {
      placement[r][c] = p;
      c--;
      p++;
    }
  }
  else {
    for (int j = 0; j < n/2-1; j += 2) {
      placement[r][c] = p;
      r--;
      p++;
      placement[r][c] = p;
      c--;
      p++;
      placement[r][c] = p;
      r++;
      p++;
      if (j >= n/2-2) break;
      placement[r][c] = p;
      c--;
      p++;
    }
  }
  placement[r][c] = p;
  r++;
  p++;
  for (int i = p3.size()-1; i >= 0; i--) {
    for (int j = n/2-2; j > p3[i]; j--) {
      placement[r][c] = p;
      c++;
      p++;
    }
    placement[r][c] = p;
    r++;
    p++;
    for (int j = n/2-2; j > p3[i]; j--) {
      placement[r][c] = p;
      c--;
      p++;
    }
    placement[r][c] = p;
    if (i > 0) r++;
    else c--;
    p++;
  }
  for (int i = 0; i < n/2; i++) {
    placement[r][c] = p;
    r--;
    p++;
  }
  for (int i = 0; i < n/2+1; i++) {
    placement[r][c] = p;
    c++;
    p++;
  }
}

int calcScore() {
  int score = 0;
  for (int i = 1; i <= n; i++) {
    for (int j = 1; j <= n; j++) {
      if (placement[i][j] == -1) board[i][j] = 0;
      else board[i][j] = snake[placement[i][j]];
    }
  }
  for (int i = 1; i <= n; i++) {
    for (int j = 1; j <= n; j++) {
      if (board[i][j] == 0) continue;
      int match = 0;
      if (board[i][j] == board[i-1][j]) match++;
      if (board[i][j] == board[i+1][j]) match++;
      if (board[i][j] == board[i][j-1]) match++;
      if (board[i][j] == board[i][j+1]) match++;
      score += scoretable[board[i][j]][match];
    }
  }
  return score;
}

string getAnswer() {
  int next = placement[n/2+1][n/2+1]-1;
  Pii head = Pii(n/2+1, n/2+1);
  string ans;
  while (next >= 0) {
    if (placement[head.first-1][head.second] == next) {
      ans += "U";
      head.first--;
      next--;
    }
    else if (placement[head.first+1][head.second] == next) {
      ans += "D";
      head.first++;
      next--;
    }
    else if (placement[head.first][head.second-1] == next) {
      ans += "L";
      head.second--;
      next--;
    }
    else if (placement[head.first][head.second+1] == next){
      ans += "R";
      head.second++;
      next--;
    }
    else break;
  }
  return ans;
}

int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);

  cin >> n >> v;
  cin >> s;

  snake = vector<int>(s.length());
  for (int i = 0; i < n * n; i++) snake[i] = s[i] - '0';

  board = vector<vector<int> >(n+2, vector<int>(n+2));
  placement = vector<vector<int> >(n+2, vector<int>(n+2, -1));
  p1 = vector<int>((n-3)/4);
  p2 = vector<int>((n-3)/4);
  p3 = vector<int>((n-3)/4);

  makePlacement();

  int score = calcScore();
  int lastScore = score;
  int bestScore = score;
  bestPlacement = placement;

  double baseTemperature = 1e4;
  double temperature = baseTemperature;
  double timeLimit = 9.900;
  int iterCount = 0;

  while (theTimer.time() < timeLimit) {
    int p = theRandom.nextUIntMod(3);
    int t = theRandom.nextUIntMod((n-3)/4);
    int v;
    if (p == 0) v = theRandom.nextUIntMod(n-2);
    else v = theRandom.nextUIntMod(n/2-1);

    #ifdef DEBUG
    if (iterCount % 10000 == 0) cerr << iterCount << " " << score << " " << lastScore << " " << bestScore << " " << temperature << " " << endl;
    #endif

    int lastv;
    if (p == 0) lastv = p1[t];
    else if (p == 1) lastv = p2[t];
    else lastv = p3[t];

    if (p == 0) p1[t] = v;
    else if (p == 1) p2[t] = v;
    else p3[t] = v;

    makePlacement();
    score = calcScore();

    if (score >= lastScore) {
      lastScore = score;
      if (score > bestScore) {
        bestPlacement = placement;
        bestScore = score;
      }
    }
    else if (theRandom.nextDouble() < exp(double(score - lastScore) / temperature)) {
      lastScore = score;
    }
    else {
      if (p == 0) p1[t] = lastv;
      else if (p == 1) p2[t] = lastv;
      else p3[t] = lastv;
    }

    iterCount++;
    temperature = baseTemperature * ((timeLimit - theTimer.time()) / timeLimit);
  }

  cerr << "iterCount   = " << iterCount << endl;
  cerr << "temperature = " << temperature << endl;
  cerr << "bestScore   = " << bestScore << endl;

  placement = bestPlacement;

  string ans = getAnswer();

  cout << ans.length() << endl;
  for (auto &x: ans) cout << x << endl;

  return 0;
}
