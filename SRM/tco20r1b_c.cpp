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

class EllysDifferentPrimes {
public:
  int getClosest(int n) {
    vector<bool> sieve(100000000, true);
    sieve[0] = false;
    sieve[1] = false;
    for (int i = 2; i * i < 100000000; i++) {
      if (sieve[i]) {
        for (int j = i * i; j < 100000000; j += i) sieve[j] = false;
      }
    }
    for (int i = 0; i < 100000000; i++) {
      if (n-i > 0) {
        if (sieve[n-i]) {
          vector<bool> digits(10);
          int x = n-i;
          bool good = true;
          while (x > 0) {
            int next = x % 10;
            if (digits[next]) {
              good = false;
              break;
            }
            digits[next] = true;
            x /= 10;
          }
          if (good) return n-i;
        }
      }
      if (n+i < 100000000) {
        if (sieve[n+i]) {
          vector<bool> digits(10);
          int x = n+i;
          bool good = true;
          while (x > 0) {
            int next = x % 10;
            if (digits[next]) {
              good = false;
              break;
            }
            digits[next] = true;
            x /= 10;
          }
          if (good) return n+i;
        }
      }
    }
    return 0;
  }
};

// driver main
int main() {
  cin.tie(0);
  ios::sync_with_stdio(false);

  int n;
  cin >> n;

  EllysDifferentPrimes solver;
  auto ans = solver.getClosest(n);

  cout << ans << endl;
}
