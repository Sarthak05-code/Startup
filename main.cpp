#include <cstdint>
#include <iostream>
#include <string>

typedef uint32_t u32;
typedef uint8_t u8;

u32 fnv1a_hash(const std::string &str) {
  u32 hash = 0x811c9dc5;
  u32 prime = 0x01000193;
  for (char c : str) {
    hash ^= static_cast<u8>(c);
    hash *= prime;
  }

  return hash;
}

int main() {
  std::string data = "sarthak";
  u32 hashValue = fnv1a_hash(data);
  std::cout << "String : " << data << std::endl;
  std::cout << "Hashed : 0x" << hashValue << std::endl;

  return 0;
}
