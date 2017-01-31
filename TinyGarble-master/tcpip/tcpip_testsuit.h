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

#ifndef TCPIP_TCPIP_TESTSUIT_H_
#define TCPIP_TCPIP_TESTSUIT_H_

#include <functional>

using std::function;

int TcpipTestRun(
  const function<int(const void *, int)>& server_func,
  const void* server_data,
  const function<int(const void *, int)>& client_func,
  const void* client_data);

#endif /* TCPIP_TCPIP_TESTSUIT_H_ */
