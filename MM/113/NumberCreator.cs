using System;
using System.Collections.Generic;
using System.Numerics;
using System.Diagnostics;

public class NumberCreator {
  static Random theRandom = new Random();

  static HashSet<int> sieveOfEratosthenes(int x) {
    var sieve = new List<bool>();
    for (int i = 0; i <= x; i++) sieve.Add(true);
    for (int d = 2; d * d <= x; d++) {
      if (sieve[d]) {
        for (int d2 = d * d; d2 <= x; d2 += d) sieve[d2] = false;
      }
    }
    var prime = new HashSet<int>();
    for (int i = 2; i <= x; i++) {
      if (sieve[i]) prime.Add(i);
    }
    return prime;
  }

  static public void Main(){
    var sw = new Stopwatch();
    sw.Start();

    string[] buf;
    buf = Console.ReadLine().Split(' ');
    int num0 = int.Parse(buf[0]);
    buf = Console.ReadLine().Split(' ');
    int num1 = int.Parse(buf[0]);
    buf = Console.ReadLine().Split(' ');
    BigInteger target = BigInteger.Parse(buf[0]);

    int iterLimit = 200 / buf[0].Length;
    var fullPrime = sieveOfEratosthenes(10000);
    var subPrime = new List<int>();

    var best = new List<string>();
    int bestLength = 1000000007;
    for (int m = 2; m < 10000; m++) {
      if (fullPrime.Contains(m)) subPrime.Insert(0, m);
      for (int t = 0; t < iterLimit; t++) {
        var quot = new List<BigInteger>();
        var rem = new List<int>();
        var div = new List<int>();
        BigInteger k = target;
        int cost = 0;
        int lim = (int)Math.Ceiling(Math.Sqrt(m));
        int mp = 0;
        int mpn = 0;
        int mpf = 0;
        var numNeed = new HashSet<long>();
        while (k > m) {
          int d = 1;
          foreach (var x in numNeed) {
            if (x <= d || x < lim) continue;
            int r = (int)(k % x);
            if (r == 0) d = (int)x;
          }
          if (d > 1) k /= d;
          else {
            mp = mpn;
            mpn = 1000000007;
            mpf = 1000000007;
            for (; mp < subPrime.Count; mp++) {
              while (true) {
                int r = (int)(k % subPrime[mp]);
                if (r == 0) {
                  if (mp == 1000000007) mpf = mp;
                  if (d * subPrime[mp] > m) {
                    if (mp < mpn) mp = mpn;
                    break;
                  }
                  k /= subPrime[mp];
                  d *= subPrime[mp];
                }
                else break;
              }
            }
            if (mpf == 1000000007) {
              mpn = 0;
            }
            else if (mpn == 1000000007) {
              mpn = mpf;
            }
          }
          if (d >= lim) {
            quot.Add(k);
            rem.Add(0);
            div.Add(d);
            numNeed.Add(d);
            cost++;
          }
          else {
            k *= d;
            int r = (int)(k % m);
            if (theRandom.NextDouble() < 0.5) {
              r = r - m;
              k = (k + m - 1) / m;
            }
            else {
              k /= m;
            }
            quot.Add(k);
            rem.Add(r);
            div.Add(m);
            numNeed.Add(Math.Abs(r));
            numNeed.Add(m);
            mp = 0;
            cost += 2;
          }
        }
        quot.Reverse();
        rem.Reverse();
        div.Reverse();
        numNeed.Add((int)quot[0]);

        var op = new List<string>();
        if (numNeed.Contains(num0)) numNeed.Remove(num0);
        if (numNeed.Contains(num1)) numNeed.Remove(num1);
        var numID = new Dictionary<long, int>(){{num0, 0}, {num1, 1}};
        var numIDInv = new Dictionary<int, long>(){{0, num0}, {1, num1}};
        var numNext = new Dictionary<long, int>(){{num0, 0}, {num1, 1}};
        var numPossible = new HashSet<long>();
        var nonNecessarilyNum = new HashSet<long>();
        cost += numNeed.Count;
        int p = 2;
        while (numNeed.Count > 0 && cost < bestLength) {
          var numNew = new Dictionary<long, int>();
          foreach (var i1 in numNext) {
            long y = i1.Key;
            foreach (var i2 in numID) {
              long z = i2.Key;
              if (y + z < (1L >> 31)) numPossible.Add(y + z);
              if (y * z < (1L >> 31)) numPossible.Add(y * z);
              if (y - z > 0) numPossible.Add(y - z);
              else if (z - y > 0) numPossible.Add(z - y);
              if (y / z > 0) numPossible.Add(y / z);
              else if (z / y > 0) numPossible.Add(z / y);
              if (numNeed.Contains(y + z)) {
                op.Add((i1.Value).ToString() + " + " + (i2.Value).ToString());
                numNew.Add(y + z, p);
                numNeed.Remove(y + z);
                p++;
              }
              if (numNeed.Contains(y - z)) {
                op.Add((i1.Value).ToString() + " - " + (i2.Value).ToString());
                numNew.Add(y - z, p);
                numNeed.Remove(y - z);
                p++;
              }
              if (numNeed.Contains(z - y)) {
                op.Add((i2.Value).ToString() + " - " + (i1.Value).ToString());
                numNew.Add(z - y, p);
                numNeed.Remove(z - y);
                p++;
              }
              if (numNeed.Contains(y * z)) {
                op.Add((i1.Value).ToString() + " * " + (i2.Value).ToString());
                numNew.Add(y * z, p);
                numNeed.Remove(y * z);
                p++;
              }
              if (numNeed.Contains(y / z)) {
                op.Add((i1.Value).ToString() + " / " + (i2.Value).ToString());
                numNew.Add(y / z, p);
                numNeed.Remove(y / z);
                p++;
              }
              if (numNeed.Contains(z / y)) {
                op.Add((i2.Value).ToString() + " / " + (i1.Value).ToString());
                numNew.Add(z / y, p);
                numNeed.Remove(z / y);
                p++;
              }
            }
          }
          numNext.Clear();
          foreach (var x in numNew) {
            numID.Add(x.Key, x.Value);
            numIDInv.Add(x.Value, x.Key);
            numNext.Add(x.Key, x.Value);
          }
          if (numNew.Count == 0) {
            if (cost == bestLength - 1) {
              cost++;
              break;
            }
            long bestNext = (long) 9e18;
            double ch = 1.0;
            foreach (var x in numPossible) {
              if (numID.ContainsKey(x)) continue;
              if (theRandom.NextDouble() < 1.0 / ch) bestNext = x;
              ch += 1.0;
            }
            int bestRemaining = numNeed.Count;
            foreach (var x in numPossible) {
              if (numID.ContainsKey(x)) continue;
              var numNeedTmp = new HashSet<long>();
              foreach (var y in numNeed) numNeedTmp.Add(y);
              numNext.Add(x, p);
              do {
                numNew.Clear();
                foreach (var i1 in numNext) {
                  long y = i1.Key;
                  foreach (var i2 in numID) {
                    long z = i2.Key;
                    if (numNeedTmp.Contains(y + z)) {
                      numNew.Add(y + z, p);
                      numNeedTmp.Remove(y + z);
                    }
                    if (numNeedTmp.Contains(y - z)) {
                      numNew.Add(y - z, p);
                      numNeedTmp.Remove(y - z);
                    }
                    if (numNeedTmp.Contains(z - y)) {
                      numNew.Add(z - y, p);
                      numNeedTmp.Remove(z - y);
                    }
                    if (numNeedTmp.Contains(y * z)) {
                      numNew.Add(y * z, p);
                      numNeedTmp.Remove(y * z);
                    }
                    if (numNeedTmp.Contains(y / z)) {
                      numNew.Add(y / z, p);
                      numNeedTmp.Remove(y / z);
                    }
                    if (numNeedTmp.Contains(z / y)) {
                      numNew.Add(z / y, p);
                      numNeedTmp.Remove(z / y);
                    }
                  }
                }
                numNext.Clear();
                foreach (var y in numNew) {
                  numNext.Add(y.Key, y.Value);
                }
              } while (numNew.Count > 0);
              if (numNeedTmp.Count < bestRemaining) {
                bestNext = x;
                bestRemaining = numNeedTmp.Count;
              }
            }
            numNeed.Add(bestNext);
            nonNecessarilyNum.Add(bestNext);
            foreach (var x in numID) {
              numNext.Add(x.Key, x.Value);
            }
            cost++;
          }
        }

        if (cost < bestLength) {
          foreach (var x in op) {
            var ops = x.Split(' ');
            var lhs = int.Parse(ops[0]);
            var rhs = int.Parse(ops[2]);
            if (nonNecessarilyNum.Contains(numIDInv[lhs])) nonNecessarilyNum.Remove(numIDInv[lhs]);
            if (nonNecessarilyNum.Contains(numIDInv[rhs])) nonNecessarilyNum.Remove(numIDInv[rhs]);
          }
          foreach (var x in nonNecessarilyNum) {
            if (!numID.ContainsKey(x)) continue;
            var id = numID[x];
            numID.Remove(x);
            numIDInv.Remove(id);
            op.RemoveAt(id-2);
            cost--;
            for (int i = 0; i < op.Count; i++) {
              var ops = op[i].Split(' ');
              var lhs = int.Parse(ops[0]);
              var rhs = int.Parse(ops[2]);
              if (lhs > id) lhs--;
              if (rhs > id) rhs--;
              op[i] = lhs.ToString() + " " + ops[1] + " " + rhs.ToString();
            }
            for (int i = id+1; i <= numID.Count; i++) {
              var numMove = numIDInv[i];
              numID[numMove] = i-1;
              numIDInv.Remove(i);
              numIDInv.Add(i-1, numMove);
            }
            p--;
          }
          p = numID[(int)quot[0]];
          for (int i = 0; i < rem.Count; i++) {
            op.Add(p.ToString() + " * " + (numID[div[i]]).ToString());
            p = op.Count + 1;
            if (rem[i] > 0) {
              op.Add(p.ToString() + " + " + (numID[rem[i]]).ToString());
              p = op.Count + 1;
            }
            else if (rem[i] < 0) {
              op.Add(p.ToString() + " - " + (numID[-rem[i]]).ToString());
              p = op.Count + 1;
            }
          }

          if (op.Count < bestLength) {
            best = op;
            bestLength = op.Count;
          }
        }
        //Console.Error.WriteLine($"{m} {cost} {op.Count} {numNeed.Count}");
      }
      if (sw.ElapsedMilliseconds > 9900) {
        Console.Error.WriteLine($"m = {m}");
        break;
      }
    }

    Console.WriteLine(best.Count);
    foreach (var s in best) {
      Console.WriteLine(s);
    }

    return;
  }
}
