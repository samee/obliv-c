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

#include "tcpip/tcpip_testsuit.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdlib>
#include <iostream>
#include "util/common.h"
#include "tcpip/tcpip.h"
#include "util/log.h"

using std::endl;

#define PORT_TRIAL 20
#define SLEEP_BEFORE_SEC 1 // sleep before connection in client

int TcpipTestRun(const function<int(const void *, int)>& server_func,
                 const void* server_data,
                 const function<int(const void *, int)>& client_func,
                 const void* client_data) {

  char server_ip[] = "127.0.0.1";
  int port = rand() % 50000 + 10000;

  for (int i = 0; i < PORT_TRIAL; i++) {
    if (ServerOpenSocket(port) == FAILURE) {
      port = rand() % 5000 + 1000;
      LOG(INFO) << "Cannot open the socket in port " << port << endl;
      if (i == PORT_TRIAL - 1) {
        LOG(ERROR) << "Connection failed." << endl;
        return FAILURE;
      }
    } else {
      break;
    }
  }

  pid_t childPID = fork();
  if (childPID >= 0) {  // fork was successful
    if (childPID == 0) {  // client
      sleep(SLEEP_BEFORE_SEC);
      int client_connfd;
      if ((client_connfd = ClientInit(server_ip, port)) == -1) {
        LOG(ERROR) << "Cannot connect to " << server_ip << ":" << port << endl;
        LOG(ERROR) << "Connection failed." << endl;
        exit(FAILURE);
      }
      if (client_func(client_data, client_connfd) == FAILURE) {
        LOG(ERROR) << "client failed." << endl;
        exit(FAILURE);
      }
      if (ClientClose(client_connfd) == FAILURE) {
        LOG(ERROR) << "closing client failed." << endl;
        exit(FAILURE);
      }
      exit(SUCCESS);
    } else {  //server
      int server_connfd;
      if ((server_connfd = ServerWaitForClient()) == FAILURE) {
        LOG(ERROR) << "server connection failed." << endl;
        return FAILURE;
      }
      if (server_func(server_data, server_connfd) == FAILURE) {
        LOG(ERROR) << "server failed." << endl;
        return FAILURE;
      }
      if (ServerClose(server_connfd) == FAILURE) {
        LOG(ERROR) << "closing server failed." << endl;
        return FAILURE;
      }
      int client_returnStatus;
      waitpid(childPID, &client_returnStatus, 0);
      if (((char) WEXITSTATUS(client_returnStatus)) != SUCCESS
          || WIFEXITED(client_returnStatus) == false) {
        LOG(ERROR) << "client child failed." << endl;
        return FAILURE;
      }
    }
  } else {  // fork failed
    LOG(ERROR) << "Fork failed." << endl;
    return FAILURE;
  }

  return SUCCESS;
}
