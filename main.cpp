#include <cassert>
#include <stdio.h>
#include <string>
using namespace std;

void test_runner(int number_one, int number_two) {
  assert(number_one + number_two > 10);
  printf("Test passed.\n");
}

void test_case_sensitivity(const string &name) {
  assert(name == "Sarthak");
  printf("The test passed. as well. ");
}

int main(void) {
  test_runner(12, 2);
  test_case_sensitivity("sarthak");
  return 0;
}
