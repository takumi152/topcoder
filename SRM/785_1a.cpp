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

vector<ll> p10(18);

class EllysPalMul {
public:
  ll solve(vector<int> now, int x) {
    ll ans = 1000000007;
    ll z1 = p10[now[2]] * now[0] + now[1];
    if (z1 % x == 0 && z1 / x < ans) ans = z1 / x;
    ll z2 = p10[now[2]] * (now[0] / 10) + now[1];
    if (z2 % x == 0 && z2 / x < ans) ans = z2 / x;
    if (now[2] < 7) {
      for (int i = 0; i <= 9; i++) {
        ans = min(ans, solve(vector<int>({now[0] * 10 + i, (int) (i * p10[now[2]] + now[1]), now[2] + 1}), x));
      }
    }
    return ans;
  }

  int getMin(int x) {
    p10[0] = 1;
    for (int i = 0; i < 18; i++) p10[i+1] = p10[i] * 10LL;
    ll ans = 1000000007;
    for (int i = 1; i <= 9; i++) ans = min(ans, solve(vector<int>({i, i, 1}), x));
    if (ans > 1000000000) return -1;
    else return ans;
  }
};

// driver main
int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);

  int x;
  cin >> x;

  EllysPalMul solver;
  auto ans = solver.getMin(x);

  cout << ans << endl;
}
