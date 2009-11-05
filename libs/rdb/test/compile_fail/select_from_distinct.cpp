#include "test_fail.hpp"

void test() {
  select(p.id).from(p).distinct(p.id);
}

