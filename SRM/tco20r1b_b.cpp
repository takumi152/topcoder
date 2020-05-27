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

class EllysWhatDidYouGet {
public:
  int getCount(int n) {
    int ans = 0;
    for (int x = 1; x <= 50; x++) {
      for (int y = 1; y <= 50; y++) {
        for (int z = 1; z <= 50; z++) {
          unordered_set<int> num;
          bool good = true;
          for (int k = 1; k <= n; k++) {
            int res = (k * x + y) * z;
            int digitsum = 0;
            while (res > 0) {
              digitsum += res % 10;
              res /= 10;
            }
            num.insert(digitsum);
            if (num.size() >= 2) {
              good = false;
              break;
            }
          }
          if (good) ans++;
        }
      }
    }
    return ans;
  }
};

// driver main
int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);

  int n;
  cin >> n;

  EllysWhatDidYouGet solver;
  auto ans = solver.getCount(n);

  cout << ans << endl;
}
