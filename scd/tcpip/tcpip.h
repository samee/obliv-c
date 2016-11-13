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

#ifndef TCPIP_TCPIP_H_
#define TCPIP_TCPIP_H_

#include <cstdint>
#include "crypto/block.h"

int ServerOpenSocket(int port);
int ServerWaitForClient();
int ServerInit(int port);
int ServerClose(int sock);

int ClientInit(const char* ip, int port);
int ClientClose(int sock);

int SendData(int sock, const void *var, int len);
int RecvData(int sock, void* var, int len);

#endif /* TCPIP_TCPIP_H_ */
