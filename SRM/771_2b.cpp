#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <string>
#include <queue>
#include <cstdlib>
#include <numeric>

using namespace std;

typedef long long int ll;
typedef pair<int, int> Pii;

const ll mod = 1000000007;

class CollectAllCoins {
public:
  vector<int> collect(int R, int C) {
    vector<int> ans;
    if (C % 2 == 1) {
      for (int i = 0; i < C; i++) {
        ans.push_back(i);
      }
      for (int i = 1; i < R; i++) {
        for (int j = 0; j < C; j += 2) {
       	  ans.push_back(i*C + j);
        }
        for (int j = 1; j < C; j += 2) {
       	  ans.push_back(i*C + j);
        }
      }
    }
    else if (R % 2 == 1) {
      for (int i = 0; i < R; i++) {
        ans.push_back(i*C);
      }
      for (int i = 1; i < C; i++) {
        for (int j = 0; j < R; j += 2) {
       	  ans.push_back(i + j*C);
        }
        for (int j = 1; j < R; j += 2) {
       	  ans.push_back(i + j*C);
        }
      }
    }
    return ans;
  }
};
