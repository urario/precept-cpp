#include "candidate.hpp"
using ca32 = spike::aligned_ptr<const int, 32>;
using a32 = spike::aligned_ptr<int, 32>;
void misuse(ca32 source) {
  a32 target = source;
  (void)target;
}
