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

class LimpingDog {
public:
  int countSteps(int t) {
    int e = 0;
    int step = 0;
    while (e < t) {
      if (step % 4 == 2) e += 2;
      else e += 1;
      if (e <= t) step++;
      if (step % 47 == 0) e += 42;
    }
    return step;
  }
};

// driver main
int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);

  int t;
  cin >> t;

  auto cc = LimpingDog();
  auto ans = cc.countSteps(t);

  cout << ans << endl;
}
