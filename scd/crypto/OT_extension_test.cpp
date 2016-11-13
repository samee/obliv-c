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

#include "crypto/OT_extension.h"

#include <boost/program_options.hpp>
#include <openssl/rand.h>
#include <iostream>
#include "tcpip/tcpip.h"
#include "tcpip/tcpip_testsuit.h"
#include "util/common.h"
#include "util/util.h"
#include "util/log.h"

using std::endl;
namespace po = boost::program_options;

struct OTTestStruct {
  block** message;
  bool* select;
  uint len;
};

int Alice(const void* data, int connfd) {
  OTTestStruct* OT_test_str = (OTTestStruct*) data;
  block **message = OT_test_str->message;
  uint len = OT_test_str->len;

  uint64_t transfer_time = RDTSC;
  if (OTExtSend(message, len, connfd) == FAILURE) {
    LOG(ERROR) << "OTsend failed." << endl;
    return FAILURE;
  }
  transfer_time = RDTSC - transfer_time;

  LOG(INFO) << "OT Extension send total time (cc) = " << transfer_time
            << "\t(cc/bit) = " << (1.0 * transfer_time) / len << endl;

  return SUCCESS;
}

int Bob(const void *data, int connfd) {
  OTTestStruct* OT_test_str = (OTTestStruct*) data;
  block **message = OT_test_str->message;
  bool *select = OT_test_str->select;
  uint len = OT_test_str->len;
  block* message_recv = new block[len];

  uint64_t transfer_time = RDTSC;
  if (OTExtRecv(select, len, connfd, message_recv) == FAILURE) {
    LOG(ERROR) << "OTsend failed." << endl;
    return FAILURE;
  }
  transfer_time = RDTSC - transfer_time;

  bool test_passed = true;
  for (uint i = 0; i < len; i++) {
    if (!CmpBlock(message[i][(select[i] ? 1 : 0)], message_recv[i])) {
      LOG(ERROR) << "Equality test failed." << endl << message_recv[i] << "!="
          << message[i][select[i] ? 1 : 0] << endl;
      test_passed = false;
    }
  }

  LOG(INFO) << "OT Extension total recv time (cc) = " << transfer_time
            << "\t(cc/bit) = " << (1.0 * transfer_time) / len << endl;

  delete[] message_recv;
  if (!test_passed) {
    return FAILURE;
  } else {
    LOG(INFO) << "Equality test Passed." << endl;
  }
  return SUCCESS;
}

int main(int argc, char* argv[]) {
  LogInitial(argc, argv);
  HashInit();
  srand(time(NULL));
  SrandSSE(time(NULL));

  uint len = 0;

  po::options_description desc("Oblivious Transfer Extension Test.");
  desc.add_options()  //
  ("help,h", "produce help message")  //
  ("len,l", po::value<uint>(&len), "length of messages. Default is random.");

  po::variables_map vm;
  try {
    po::parsed_options parsed = po::command_line_parser(argc, argv).options(
        desc).allow_unregistered().run();
    po::store(parsed, vm);
    if (vm.count("help")) {
      std::cout << desc << endl;
      return SUCCESS;
    }
    po::notify(vm);
  } catch (po::error& e) {
    LOG(ERROR) << "ERROR: " << e.what() << endl << endl;
    std::cout << desc << endl;
    return FAILURE;
  }
  if (vm.count("len") == 0) {
    len = rand() % 500 + 1000;
  }

  LOG(INFO) << "Run OT extension 1 from 2 on a message with length " << len
            << endl;
  block** message = new block*[len];
  bool* select = new bool[len];
  for (uint i = 0; i < len; i++) {
    message[i] = new block[2];
    for (uint j = 0; j < 2; j++) {
      message[i][j] = RandomBlock();
    }
    select[i] = (rand() % 2 == 1);
  }

  OTTestStruct OT_test_str;
  OT_test_str.message = message;
  OT_test_str.select = select;
  OT_test_str.len = len;

  if (TcpipTestRun(Alice, (void *) &OT_test_str, Bob,
                   (void *) &OT_test_str) == FAILURE) {
    LOG(ERROR) << "tcpip test run failed for second run." << endl;
    return FAILURE;
  }

  for (uint i = 0; i < len; i++) {
    delete[] message[i];
  }
  delete[] message;
  delete[] select;

  LogFinish();
  HashFinish();
  return SUCCESS;
}

