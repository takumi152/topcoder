#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <string>
#include <queue>
#include <stack>

using namespace std;

typedef long long int ll;
typedef pair<int, int> Pii;

const ll mod = 1000000007;

class BlindBoxSets {
private:
  vector<vector<vector<vector<double> > > > dp;
  double recursiveDP(int n, int k, int i1, int i2, int i3, int i4) {
    if (dp[i1][i2][i3][i4] >= 0.0) return dp[i1][i2][i3][i4];
    if (i1 == 0 && i2 == 0 && i3 == 0 && i4 == 0) return 0.0;

    double result = 0.0;
    if (i1 > 0) result += recursiveDP(n, k, i1-1, i2, i3, i4) * i1;
    if (i2 > 0) result += recursiveDP(n, k, i1+1, i2-1, i3, i4) * i2;
    if (i3 > 0) result += recursiveDP(n, k, i1, i2+1, i3-1, i4) * i3;
    if (i4 > 0) result += recursiveDP(n, k, i1, i2, i3+1, i4-1) * i4;
    result += k;
    if (n == 1) result *= 1.0 / i1;
    if (n == 2) result *= 1.0 / (i1 + i2);
    if (n == 3) result *= 1.0 / (i1 + i2 + i3);
    if (n == 4) result *= 1.0 / (i1 + i2 + i3 + i4);

    return dp[i1][i2][i3][i4] = result;
  }
public:
  double expectedPurchases(int n, int k) {
    dp = vector<vector<vector<vector<double> > > >(k+1, vector<vector<vector<double> > >(k+1, vector<vector<double> >(k+1, vector<double>(k+1, -1e308))));
    if (n == 1) return recursiveDP(n, k, k, 0, 0, 0);
    if (n == 2) return recursiveDP(n, k, 0, k, 0, 0);
    if (n == 3) return recursiveDP(n, k, 0, 0, k, 0);
    if (n == 4) return recursiveDP(n, k, 0, 0, 0, k);
    return 0.0;
  }
};

// driver main
int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);

  int n, k;
  cin >> n >> k;

  BlindBoxSets solver;
  auto ans = solver.expectedPurchases(n, k);

  cout << ans << endl;
}
