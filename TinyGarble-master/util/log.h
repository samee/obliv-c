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

#ifndef UTIL_LOG_H_
#define UTIL_LOG_H_

#include <fstream>
#include <string>
#include <new>

#include "util/common.h"
#include "crypto/block.h"

using std::ostream;
using std::string;

class DummyLog : public ostream {
  typedef ostream& (*ostreamManipulator)(ostream&);
 public:
  template<class T>
  inline DummyLog& operator <<(const T& v) {
    return *this;
  }

  inline DummyLog& operator<<(ostreamManipulator manip) {
    return *this;
  }

};

#define ERROR 0
#define INFO 1
// Error: red, info:green
#define LOG_COLOR(X) (((X)==ERROR)?31:32)

#ifdef ENABLE_DUMP
#define DUMP(X) Dump(X)
#else
#define DUMP(X) DummyLogStream()
#endif

#ifdef ENABLE_LOG
#define CHECK_ALLOC(X) try { X; } catch (std::bad_alloc& e) { \
    LOG(ERROR) << e.what() << std::endl; \
    return FAILURE; }
#define LOG(X) LogStream((X)) << __FILE__ << ":" <<  __LINE__ << " \033[" \
  << LOG_COLOR(X) << "m" << #X << "\033[0m: "
#define CHECK_EXPR(X) if((X)==false) { LOG(ERROR) << #X << " failed" \
  << std::endl; return FAILURE; }
#define CHECK_EXPR_MSG(X, Y) if((X)==false) { LOG(ERROR) << (#X) << " failed: \"" \
  << (Y) << "\"" << std::endl; return FAILURE; }
#define CHECK(X) if((X)==FAILURE) { LOG(ERROR) << #X << " failed" \
  << std::endl; return FAILURE; }
#define BN_CHECK(X) if((X)==0) { LOG(ERROR) << #X << " failed" \
  << std::endl; return FAILURE; }
#else /* ENABLE_LOG */
extern bool __dummy_expr__;
#define CHECK_ALLOC(X) X
#define LOG(X) DummyLogStream()
#define CHECK_EXPR(X) __dummy_expr__ = X
#define CHECK_EXPR_MSG(X, Y) __dummy_expr__ = X
#define CHECK(X) X
#define BN_CHECK(X) X
#endif /* ENABLE_LOG */

void LogInitial(int argc, char *argv[]);
void LogFinish();
ostream& Dump(const string& dump_file);
ostream& LogStream(int log_code);
ostream& operator <<(ostream & o, const block& v);
DummyLog& DummyLogStream();

#endif /* UTIL_LOG_H_ */
