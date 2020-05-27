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

class BagsOfMarbles {
public:
  int removeFewest(int desired, int bagSize, int noWhiteBags, int noBlackBags, int someWhiteBags, int someBlackBags) {
    int a = noWhiteBags;
    int b = someBlackBags;
    a += b;
   	int remaining = desired;
    if (remaining - noBlackBags * bagSize <= 0) return desired;
    remaining -= noBlackBags * bagSize;
    if (remaining <= someWhiteBags) return noBlackBags * bagSize + (remaining * bagSize);
    else return -1;
  }
};
