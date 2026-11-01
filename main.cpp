#include <iostream>
#include <vector>
using namespace std;

int main() {
  vector<int> values = {1, 2, 3, 4, 5, 6, 7};

  for (auto v : values) {
    cout << v << " ";
  }
  return 0;
}
