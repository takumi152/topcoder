// WIP

#include <iostream>
#include <iomanip>
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

class EllysTwoRatings {
public:
  double getChance(int n, int a, int b) {
    vector<vector<double> > dpa(n+1, vector<double>(1001));
    vector<vector<double> > dpb(n+1, vector<double>(1001));
    dpa[0][a] = 1.0;
    dpb[0][b] = 1.0;
    double ans = 0.0;
    for (int i = 0; i < n; i++) {
      vector<double> deca(1001);
      vector<double> decb(1001);
      for (int j = 1; j <= 1000; j++) {
        int r = min(1000, j + 100) - max(1, j - 100);
        for (int k = max(1, j - 100); k <= min(1000, j + 100); k++) {
          dpa[i+1][k] += dpa[i][j] * (1.0 / r);
          dpb[i+1][k] += dpb[i][j] * (1.0 / r);
        }
      }
      for (int j = 1; j <= 1000; j++) {
        for (int k1 = max(1, j - 100); k1 <= min(1000, j + 100); k1++) {
          int r1 = min(1000, k1 + 100) - max(1, k1 - 100);
          for (int k2 = max(1, j - 100); k2 <= min(1000, j + 100); k2++) {
            int r2 = min(1000, k2 + 100) - max(1, k2 - 100);
            double p = 1.0 / (r1 * r2);
            deca[j] += dpa[i][k1] * dpb[i][k2] * p;
            decb[j] += dpa[i][k1] * dpb[i][k2] * p;
          }
        }
      }
      for (int j = 1; j <= 1000; j++) {
        ans += deca[j] + decb[j];
        dpa[i+1][j] -= deca[j];
        dpb[i+1][j] -= decb[j];
      }
      for (auto &x: dpa[i+1]) cerr << x << " ";
      cerr << endl;
      for (auto &x: dpb[i+1]) cerr << x << " ";
      cerr << endl;
    }
    return ans;
  }
};

// driver main
int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);

  int n, a, b;
  cin >> n >> a >> b;

  EllysTwoRatings solver;
  auto ans = solver.getChance(n, a, b);

  cout << setprecision(16) << ans << endl;
}
