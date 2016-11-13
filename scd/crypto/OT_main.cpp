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

#include "crypto/OT.h"

#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <cstdlib>
#include "tcpip/tcpip.h"
#include "util/common.h"
#include "util/util.h"
#include "util/log.h"

namespace po = boost::program_options;
using std::string;
using std::endl;

int main(int argc, char *argv[]) {
  LogInitial(argc, argv);

  srand(time(NULL));
  SrandSSE(time(NULL));

  int m_len = 1;  // number of message pairs
  int port;
  string scd_file_address;
  string server_ip;
  string message0_str;
  string message1_str;
  string select_str;
  po::options_description desc("Oblivious Transfer.");
  desc.add_options()  //
  ("help,h", "produce help message")  //
  ("alice,a", "Run as Alice (server).")  //
  ("message0",
   po::value<string>(&message0_str)->default_value(
       "15141312_11100908_07060504_03020100"),
   "Alice's 128-bit message 0 in hexadecimal w/o '0x'.")  //
  ("message1",
   po::value<string>(&message1_str)->default_value(
       "00010203_04050607_08091011_12131415"),
   "Alice's 128-bit message 1 in hexadecimal w/o '0x'.")  //
  ("select", po::value<string>(&select_str)->default_value("0"),
   "Bob's 1-bit selection (0/1).")  //
  ("bob,b", "Run as Bob (client).")  //
  ("port,p", po::value<int>(&port)->default_value(1234), "socket port")  //
  ("server_ip,s", po::value<string>(&server_ip)->default_value("127.0.0.1"),
   "Server's (Alice's) IP, required when running as Bob.");

  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
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

  if (vm.count("alice") == 0 && vm.count("bob") == 0) {
    LOG(ERROR) << "One of --alice or --bob mode flag should be used." << endl
               << endl;
    std::cout << desc << endl;
    return FAILURE;
  }

  if (vm.count("alice")) {
    // open the socket
    int connfd;
    if ((connfd = ServerInit(port)) == FAILURE) {
      LOG(ERROR) << "Cannot open the socket in port " << port << endl;
      return FAILURE;
    }
    LOG(INFO) << "Open Alice server on port: " << port << endl;

    block** message = new block*[m_len];
    message[0] = new block[2];
    if (Str2Block(message0_str, &message[0][0]) == FAILURE) {
      LOG(ERROR) << "Cannot parse message0 " << message0_str << endl;
      return FAILURE;
    }
    if (Str2Block(message1_str, &message[0][1]) == FAILURE) {
      LOG(ERROR) << "Cannot parse message1 " << message1_str << endl;
      return FAILURE;
    }
    LOG(INFO) << "messages are:" << endl << "0: " << message[0][0] << endl
        << "1: " << message[0][1] << endl;

    LOG(INFO) << "start OT send" << endl;
    if (OTSend(message, m_len, connfd) == FAILURE) {
      LOG(ERROR) << "OTsend failed." << endl;
      return FAILURE;
    }

    delete[] message[0];
    delete[] message;
    ServerClose(connfd);
  } else if (vm.count("bob")) {
    if (vm.count("server_ip")) {
      server_ip = vm["server_ip"].as<string>();
    }

    // open socket, connect to server
    int connfd;
    if ((connfd = ClientInit(server_ip.c_str(), port)) == FAILURE) {
      LOG(ERROR) << "Cannot connect to " << server_ip << ":" << port << endl;
      return FAILURE;
    }
    LOG(INFO) << "Connect Bob client to Alice server on " << server_ip << ":"
              << port << endl;

    bool *select = new bool[m_len];
    select[0] = (atoi(select_str.c_str()) == 1);
    LOG(INFO) << "select is " << (select[0] ? "1" : "0") << endl;

    LOG(INFO) << "start OT recv" << endl;

    block *message = new block[m_len];
    if (OTRecv(select, m_len, connfd, message) == FAILURE) {
      LOG(ERROR) << "OTsend failed." << endl;
      return FAILURE;
    }

    LOG(INFO) << "selected message is:" << endl << select[0] << ": "
        << message[0] << endl;

    delete[] message;
    delete[] select;
    ClientClose(connfd);
  }
  LogFinish();
  return SUCCESS;
}

