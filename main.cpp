#include <iostream>
#include <map>
#include <string>
#include <tuple>

using namespace std;

int main() {
  std::map<int, tuple<string, string, string>> users = {
      {1, {"Sarthak", "Sagar", "Ayush"}}};

  for (auto const &[id, detail] : users) {
    auto const &[_, email, _] = detail;

    cout << "User ID: " << id << " Notification to: " << email << "\n";
  }
  return 0;
}
