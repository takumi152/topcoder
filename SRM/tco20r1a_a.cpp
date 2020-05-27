#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <string>
#include <queue>
#include <stack>
#include <unordered_set>

using namespace std;

typedef long long int ll;
typedef pair<int, int> Pii;

const ll mod = 1000000007;

class AveragePrice {
public:
  double nonDuplicatedAverage(vector<int> prices) {
    unordered_set<ll> p;
    for (auto &x: prices) p.insert(x);

    ll total = 0;
    for (auto &x: p) total += x;

    double ans = (double) total / (double) p.size();
    return ans;
  }
};

// driver main
int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);

  int n;
  cin >> n;
  vector<int> p(n);
  for (auto &x: p) cin >> x;

  AveragePrice solver;
  auto ans = solver.nonDuplicatedAverage(p);

  cout << ans << endl;
}
