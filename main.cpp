
#include <iostream>
#include <string>
#include <vector>

using namespace std;

const vector<string> words = {"Fuck",    "Shit",  "Cunt",
                              "Fucking", "Pussy", "Retard"};

bool filter_on = true;

void filter(string &text) {

  if (filter_on) {

    for (const string &word : words) {

      size_t pos = text.find(word);

      while (pos != string::npos) {

        text.replace(pos, word.length(), string(word.length(), '*'));

        // Search for another occurrence
        pos = text.find(word, pos + word.length());
      }
    }
  }
}

int main() {

  string text = "Hello, Cunt!";

  filter(text);

  cout << text;

  return 0;
}
