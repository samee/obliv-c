/*
 This file is part of TinyGarble. It is modified version of JustGarble
 under GNU license.

 TinyGarble is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TinyGarble is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TinyGarble.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "util/util.h"

#include <iostream>
#include <sstream>
#include "util/minunit.h"
#include "util/log.h"
#include "util/common.h"

using std::endl;
using std::stringstream;

void TestSetup() {
}
void TestTeardown() {
}

MU_TEST(StrBlockSeperation) {
  block v, w;
  string test_hex = "15141312 11100908 07060504 03020100";
  mu_check(Str2Block(test_hex, &v) == SUCCESS);
  test_hex = "15141312_11100908_07060504_03020100";
  mu_check(Str2Block(test_hex, &w) == SUCCESS);
  mu_check(CmpBlock(v, w));
  test_hex = "15141312\t11100908\t07060504\t03020100";
  mu_check(Str2Block(test_hex, &w) == SUCCESS);
  mu_check(CmpBlock(v, w));
  test_hex = "15141312111009080706050403020100";
  mu_check(Str2Block(test_hex, &w) == SUCCESS);
  mu_check(CmpBlock(v, w));
  test_hex = "1514131\t211100 90807060504_03020100";
  mu_check(Str2Block(test_hex, &w) == SUCCESS);
  mu_check(CmpBlock(v, w));
}

MU_TEST(StrBlockFillZero) {
  block v, w;
  string test_hex = "00000000 00000000 00000000 03020100";
  mu_check(Str2Block(test_hex, &v) == SUCCESS);
  test_hex = "3020100";
  mu_check(Str2Block(test_hex, &w) == SUCCESS);
  mu_check(CmpBlock(v, w));
  test_hex = "00000003020100";
  mu_check(Str2Block(test_hex, &w) == SUCCESS);
  mu_check(CmpBlock(v, w));
}

MU_TEST(StrBlockLSB) {
  block v;
  mu_check(Str2Block("01", &v) == SUCCESS);
  mu_check(get_LSB(v));
  mu_check(Str2Block("00", &v) == SUCCESS);
  mu_check(!get_LSB(v));
}

MU_TEST(StrBlockOperator) {
  stringstream o_stream;
  string test_hex = "15141312 11100908 07060504 03020100";
  block v;
  mu_check(Str2Block(test_hex, &v) == SUCCESS);
  o_stream << v;
  o_stream.flush();
  mu_check(o_stream.str() == test_hex);
}

MU_TEST_SUITE(TestSuite) {
  MU_SUITE_CONFIGURE(&TestSetup, &TestTeardown);

  MU_RUN_TEST(StrBlockSeperation);
  MU_RUN_TEST(StrBlockFillZero);
  MU_RUN_TEST(StrBlockLSB);
  MU_RUN_TEST(StrBlockOperator);
}

int main(int argc, char *argv[]) {
  LogInitial(argc, argv);
  MU_RUN_SUITE(TestSuite);
  MU_REPORT()
  ;
  LogFinish();
  return 0;
}

