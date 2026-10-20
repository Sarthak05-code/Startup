#include <iostream>
#include <type_traits>

using namespace std;
/// NOTE : Practice 2 : Generics comparisons.
// Understanding cpp generics via template, and doing random stuff with template
//

/*
 * Here i attempted to use generic type `T` and generic type `C` , since they
 * can now hold different data type , made a boolean function to return true and
 * false if the type are same or not.
 */
template <typename T, typename C> bool type_compare(T const &a, C const &b) {
  // i assume the way it works is it.
  // assume : a -> 12 => int and b -> true => boolean, now 12 != true and hence
  // [False] else [True]. This is how i understand this since i have never used
  // and comapred generics before.
  // recently learned template and wanted to do random stuffs.
  if (a == b) {
    return true;
  }
  return false;
}

/// NOTE : Practice 3 : Proper way to check the data type and not the value
/// inside the data type.
template <typename T, typename C>
constexpr bool same_type(const T &, const C &) {
  return std::is_same_v<T, C>;
}

class Caller {};
struct Test {};
enum Tester {};

// NOTE :  Practice 1 : using auto to test type inference. Pretty cool.
int main() {
  auto number = 10;         // int
  auto name = "Sarthak";    // const char *
  auto is_true = false;     // bool
  auto amount = 10.20;      // double , not float?
  auto amount_f = 10.20f;   // float
  auto large = 100000l;     // long
  auto larger = 100000ll;   // long long
  auto largest = 100000ull; // usigned long long
  auto negative = nullptr;  // std::nullptr_t
  auto option = NULL;       // NULL => long long ?
  // Dont think this is the way to do it, just the hint shows the name of the
  // function above and with a pointer.
  auto classer = new Caller(); // Caller *
  auto structer = new Test;    // Test *
  auto enumer = new Tester;    // Tester *

  cout << "Number : " << number << " Name : " << name
       << " Status : " << is_true // seems to return 0 for is_true
       << endl;

  // Really digging the auto keyword here. similar to var which i used in java,
  // i don't think there is any problem to permanently use auto comapred to just
  // writing int, float, or double.
  auto answer = type_compare(number, is_true);
  if (answer)
    cout << "True" << endl;
  else
    cout << "False" << endl;

  return 0;
}
