#include "candidate.hpp"
struct base {
  int value{};
};
struct derived : base {
  int extra{};
};
using d64 = spike::aligned_ptr<derived, 64>;
using b64 = spike::aligned_ptr<base, 64>;
void misuse(d64 source) {
  b64 target = source;
  (void)target;
}
