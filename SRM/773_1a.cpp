// WIP

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

class ChristmasSongBroadcast {
public:
  int minimizeCost(int t, vector<int> a, vector<int> b, vector<string> directCost) {
    int n = a.size();
    vector<int> baseCost(n);
    for (int i = 0; i < n; i++) {
      ll base = b[i];
      ll mul = 0;
      baseCost[i] = base;
      while (mul < t) {
        mul += (mod - base - 1) / a[i] + 1;
        if (mul < t) {
          if ((b[i] * mul) % mod < baseCost[i]) baseCost[i] = (b[i] * mul) % mod;
          base = (b[i] * mul) % mod;
        }
      }
    }
    for (auto &x: baseCost) cerr << x << endl;

    return 0;
  }
};

// driver main
int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);

  int t, n;
  cin >> t >> n;
  vector<int> a(n), b(n);
  for (auto &x: a) cin >> x;
  for (auto &x: b) cin >> x;
  vector<string> c(n);
  for (auto &x: c) cin >> x;

  auto cc = ChristmasSongBroadcast();
  int ans = cc.minimizeCost(t, a, b, c);

  cout << ans << endl;
}
