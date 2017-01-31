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

#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <iostream>
#include "util/common.h"
#include "util/log.h"

using std::endl;

int listenfd = 0;

int kSocketBuffSize = (1 << 15);

int ServerOpenSocket(int port) {
  struct sockaddr_in serv_addr;
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    LOG(ERROR) << strerror(errno) << endl;
    return FAILURE;
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);
  if (bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    LOG(INFO) << strerror(errno) << endl;
    return FAILURE;
  }
  listen(listenfd, 5);
  return SUCCESS;
}

int ServerWaitForClient() {
  struct sockaddr_in cli_addr;
  socklen_t clilen = sizeof(cli_addr);
  int connfd = accept(listenfd, (struct sockaddr *) &cli_addr, &clilen);
  if (connfd < 0) {
    LOG(ERROR) << strerror(errno) << endl;
    return FAILURE;
  }
  int flag = 1;
  int result = setsockopt(connfd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag,
                          sizeof(int));
  if (result < 0) {
    LOG(ERROR) << strerror(errno) << endl;
    return FAILURE;
  }
  return connfd;
}

int ServerInit(int port) {
  if (ServerOpenSocket(port) == FAILURE)
    return FAILURE;
  int connfd;
  if ((connfd = ServerWaitForClient()) == FAILURE)
    return FAILURE;
  return connfd;
}

int ServerClose(int sock) {
  close(sock);
  close(listenfd);
  return SUCCESS;
}

int ClientInit(const char* ip, int port) {
  int sockfd;
  struct hostent *server;
  struct sockaddr_in serv_addr;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    LOG(ERROR) << strerror(errno) << endl;
    return FAILURE;
  }
  server = gethostbyname(ip);
  if (server == NULL) {
    LOG(ERROR) << strerror(errno) << endl;
    return FAILURE;
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr,
  server->h_length);
  serv_addr.sin_port = htons(port);
  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    LOG(ERROR) << strerror(errno) << endl;
    return FAILURE;
  }

  int flag = 1;
  int result = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag,
                          sizeof(int));
  if (result < 0) {
    LOG(ERROR) << strerror(errno) << endl;
    return FAILURE;
  }
  return sockfd;
}

int ClientClose(int sock) {
  close(sock);
  return SUCCESS;
}

int SendData(int sock, const void *var, int len) {
  if (len <= 0)
    return SUCCESS;

  int remain_len = 0;
  do {
    int len_ret = write(sock, (const char *) var + remain_len,
                        len - remain_len);
    if (len_ret == FAILURE) {
      LOG(ERROR) << strerror(errno) << endl;
      return FAILURE;
    } else if (len_ret == 0) {
      LOG(ERROR) << "connection is possibly closed" << endl;
      return FAILURE;
    }
    remain_len += len_ret;
  } while (remain_len < len);

  return SUCCESS;
}

int RecvData(int sock, void* var, int len) {

  if (len <= 0)
    return SUCCESS;

  int remain_len = 0;
  do {
    int len_ret = read(sock, (char *) var + remain_len, len - remain_len);
    if (len_ret == FAILURE) {
      LOG(ERROR) << strerror(errno) << endl;
      return FAILURE;
    } else if (len_ret == 0) {
      LOG(ERROR) << len_ret << " connection is possibly closed" << endl;
      return FAILURE;
    }
    remain_len += len_ret;
  } while (remain_len < len);

  return SUCCESS;
}
