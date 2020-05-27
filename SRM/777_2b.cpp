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

class StringTransformationEasy {
public:
  string getResult(string s, string t) {
    int n = s.length();
    int m = t.length();

    if (n < m) return "NO";
    if (n % 2 != m % 2) return "NO";
    if (s[0] != t[0]) return "NO";

    vector<int> rs, rt;
    char now;
    int count;

    now = s[0];
    count = 1;
    for (int i = 1; i < n; i++) {
      if (s[i] == now) count++;
      else {
        rs.push_back(count);
        now = s[i];
        count = 1;
      }
    }
    rs.push_back(count);

    now = t[0];
    count = 1;
    for (int i = 1; i < m; i++) {
      if (t[i] == now) count++;
      else {
        rt.push_back(count);
        now = t[i];
        count = 1;
      }
    }
    rt.push_back(count);

    if (rs.size() != rt.size()) return "NO";

    int k = rs.size();
    if (k == 1) {
      if (rs[0] != rt[0]) return "NO";
    }
    else {
      for (int i = 0; i < k; i++) {
        if (rs[i] % 2 != rt[i] % 2) return "NO";
      }
    }

    return "YES";
  }
};

// driver main
int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);

  string s, t;
  cin >> s >> t;

  auto cc = StringTransformationEasy();
  auto ans = cc.getResult(s, t);

  cout << ans << endl;
}
