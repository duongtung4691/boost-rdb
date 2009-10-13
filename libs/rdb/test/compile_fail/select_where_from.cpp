#include "test_fail.hpp"

void test() {
  select(p.id).where(p.id == 1).from(p);
}

