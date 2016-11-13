/*
 This file is part of TinyGarble.

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

#include "tcpip/tcpip.h"

#include <iostream>
#include <cstring>
#include "util/common.h"
#include "util/log.h"
#include "tcpip/tcpip_testsuit.h"

using std::endl;

int ServerFunc(const void *data, int connfd) {
  uint len = strlen((char *)data) + 1;
  char *data_recv = new char[len];
  if (RecvData(connfd, data_recv, len) == FAILURE) {
    LOG(ERROR) << "recv failed, server side. Test failed" << endl;
    return FAILURE;
  }
  if (SendData(connfd, data, len) == FAILURE) {
    LOG(ERROR) << "send failed, server side. Test failed" << endl;
    return FAILURE;
  }
  if (strcmp((const char *)data, data_recv) != 0) {
    LOG(ERROR) << "data are not same, server side. Test failed" << endl;
    return FAILURE;
  }
  return SUCCESS;
}


int ClientFunc(const void *data, int connfd) {
  uint len = strlen((char *)data) + 1;
  if (SendData(connfd, data, len) == FAILURE) {
    LOG(ERROR) << "send failed, client side. Test failed" << endl;
    return FAILURE;
  }
  char *data_recv = new char[len];
  if (RecvData(connfd, data_recv, len) == FAILURE) {
    LOG(ERROR) << "recv failed, client side. Test failed" << endl;
    return FAILURE;
  }
  if (strcmp((const char *)data, data_recv) != 0) {
    LOG(ERROR) << "data are not same, client side. Test failed" << endl;
    return FAILURE;
  }
  return SUCCESS;
}

int main(int argc, char* argv[]) {
  LogInitial(argc, argv);
  srand(time(0));
  char data[] = "Hello World!";
  if (TcpipTestRun(ServerFunc, (const void *)data, ClientFunc,
    (const void *)data) == FAILURE) {
    LOG(ERROR) << "tcpip test run failed" << endl;
    return FAILURE;
  }

  LogFinish();
  return SUCCESS;
}
