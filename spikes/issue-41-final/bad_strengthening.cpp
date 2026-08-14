#include "candidate.hpp"
using a32 = spike::aligned_ptr<int, 32>;
using a64 = spike::aligned_ptr<int, 64>;
void misuse(a32 source) {
  a64 target = source;
  (void)target;
}
