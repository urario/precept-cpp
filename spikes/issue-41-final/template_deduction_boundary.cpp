#include "candidate.hpp"

template <class T>
void consume(spike::aligned_ptr<const T, 32>) {}

void demonstrate(spike::aligned_ptr<int, 64> source) {
  consume(source);
}
