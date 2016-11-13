add_library (Crypto_BN OBJECT
  "BN.cpp")

add_library (Crypto_OT OBJECT
  "OT.cpp")

add_library (Crypto_OT_Extension OBJECT
  "OT_extension.cpp")

add_executable(BN_Test
  "BN_test.cpp"
  "$<TARGET_OBJECTS:Crypto_BN>"
  "$<TARGET_OBJECTS:TCPIP_TCPIP>"
  "$<TARGET_OBJECTS:Util_Util>"
  "$<TARGET_OBJECTS:Util_Log>")

if (OPENSSL_FOUND)
	target_link_libraries(BN_Test ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)
if (Boost_FOUND)
  target_link_libraries(BN_Test ${Boost_LIBRARIES})
endif (Boost_FOUND)
target_link_libraries(BN_Test -lrt)

add_test (Crypto_BN_Test
  BN_Test --log2std)

set_tests_properties (Crypto_BN_Test
	PROPERTIES
	FAIL_REGULAR_EXPRESSION "[^a-z]Failed;failed")

add_executable(OT_Test
  "OT_test.cpp"
  "$<TARGET_OBJECTS:Crypto_BN>"
  "$<TARGET_OBJECTS:Crypto_OT>"
  "$<TARGET_OBJECTS:TCPIP_TCPIP>"
  "$<TARGET_OBJECTS:TCPIP_TCPIP_Testsuit>"
  "$<TARGET_OBJECTS:Util_Util>"
  "$<TARGET_OBJECTS:Util_Log>")

if (OPENSSL_FOUND)
	target_link_libraries(OT_Test ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)
if (Boost_FOUND)
  target_link_libraries(OT_Test ${Boost_LIBRARIES})
endif (Boost_FOUND)
target_link_libraries(OT_Test -lrt)

add_test (Crypto_OT_Test
  OT_Test --log2std)

set_tests_properties (Crypto_OT_Test
	PROPERTIES
	FAIL_REGULAR_EXPRESSION "[^a-z]Failed;failed")

add_executable(OT_Extension_Test
  "OT_extension_test.cpp"
  "$<TARGET_OBJECTS:Crypto_BN>"
  "$<TARGET_OBJECTS:Crypto_OT>"
  "$<TARGET_OBJECTS:Crypto_OT_Extension>"
  "$<TARGET_OBJECTS:TCPIP_TCPIP>"
  "$<TARGET_OBJECTS:TCPIP_TCPIP_Testsuit>"
  "$<TARGET_OBJECTS:Util_Util>"
  "$<TARGET_OBJECTS:Util_Log>")

if (OPENSSL_FOUND)
	target_link_libraries(OT_Extension_Test ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)
if (Boost_FOUND)
  target_link_libraries(OT_Extension_Test ${Boost_LIBRARIES})
endif (Boost_FOUND)
target_link_libraries(OT_Extension_Test -lrt)

add_test (Crypto_OT_Extension_Test
  OT_Extension_Test --log2std)

set_tests_properties (Crypto_OT_Extension_Test
	PROPERTIES
	FAIL_REGULAR_EXPRESSION "[^a-z]Failed;failed")

add_executable(OT_Main
  "OT_main.cpp"
  "$<TARGET_OBJECTS:Crypto_BN>"
  "$<TARGET_OBJECTS:Crypto_OT>"
  "$<TARGET_OBJECTS:Crypto_OT_Extension>"
  "$<TARGET_OBJECTS:TCPIP_TCPIP>"
  "$<TARGET_OBJECTS:Util_Util>"
  "$<TARGET_OBJECTS:Util_Log>")

if (OPENSSL_FOUND)
	target_link_libraries(OT_Main ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)
if (Boost_FOUND)
  target_link_libraries(OT_Main ${Boost_LIBRARIES})
endif (Boost_FOUND)
target_link_libraries(OT_Main -lrt)
