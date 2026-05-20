#include <iostream>
int main(int argc, char** argv) {
  std::cerr << "hello argc=" << argc << std::endl;
  return argc < 2 ? 2 : 0;
}
