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

class BlackAndWhiteBallsEasy {
public:
  int getNumber(vector<int> a, int w, int b) {
    int n = a.size();

    string s;
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < a[i]; j++) {
        if (i % 2 == 0) s += "W";
        else s += "B";
      }
    }

    int m = s.length();

    vector<vector<vector<ll> > > dp(m+2, vector<vector<ll> >(2, vector<ll>(max(w, b)+2)));
    dp[0][0][0] = 1; // white segment
    dp[0][1][0] = 1; // black segment
    for (int i = 0; i < m; i++) {
      if (s[i] == 'W') {
        for (int j = 0; j <= w; j++) {
          dp[i+1][0][j+1] = (dp[i+1][0][j+1] + dp[i][0][j]) % mod;
        }
        for (int j = 0; j <= b; j++) {
          dp[i+1][1][j] = (dp[i+1][1][j] + dp[i][1][j]) % mod;
        }
      }
      else {
        for (int j = 0; j <= w; j++) {
          dp[i+1][0][j] = (dp[i+1][0][j] + dp[i][0][j]) % mod;
        }
        for (int j = 0; j <= b; j++) {
          dp[i+1][1][j+1] = (dp[i+1][1][j+1] + dp[i][1][j]) % mod;
        }
      }
      dp[i+1][0][0] = (dp[i+1][0][0] + dp[i+1][0][w] + dp[i+1][1][b]) % mod;
      dp[i+1][1][0] = (dp[i+1][1][0] + dp[i+1][0][w] + dp[i+1][1][b]) % mod;
    }

    ll ans = (dp[m][0][w] + dp[m][1][b]) % mod;
    return (int) ans;
  }
};

// driver main
int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);

  int n;
  cin >> n;
  vector<int> a(n);
  for (auto &x: a) cin >> x;
  int b, w;
  cin >> w >> b;

  auto cc = BlackAndWhiteBallsEasy();
  auto ans = cc.getNumber(a, w, b);

  cout << ans << endl;
}
