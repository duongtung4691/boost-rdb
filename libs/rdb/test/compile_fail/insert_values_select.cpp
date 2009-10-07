#include "test_fail.hpp"

void test() {
  insert_into(p)(p.id).values(1).select(p.name);
}

