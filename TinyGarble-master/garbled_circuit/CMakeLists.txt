add_library (Garbled_Circuit_Garbled_Circuit_Util OBJECT
  "garbled_circuit_util.cpp")
add_library (Garbled_Circuit_Garbled_Circuit_High_Mem OBJECT
  "garbled_circuit_high_mem.cpp")  
add_library (Garbled_Circuit_Garbled_Circuit_Low_Mem OBJECT
  "garbled_circuit_low_mem.cpp")
add_library (Garbled_Circuit_Garbled_Circuit OBJECT
  "garbled_circuit.cpp")
  add_library (Garbled_Circuit_Garbled_Circuit_Testing OBJECT
  "garbled_circuit_testing.cpp")

add_executable(TinyGarble
  "garbled_circuit_main.cpp"
  "$<TARGET_OBJECTS:Garbled_Circuit_Garbled_Circuit>"
  "$<TARGET_OBJECTS:Garbled_Circuit_Garbled_Circuit_Util>"
  "$<TARGET_OBJECTS:Garbled_Circuit_Garbled_Circuit_High_Mem>"
  "$<TARGET_OBJECTS:Garbled_Circuit_Garbled_Circuit_Low_Mem>"
  "$<TARGET_OBJECTS:Crypto_BN>"
  "$<TARGET_OBJECTS:Crypto_OT>"
  "$<TARGET_OBJECTS:Crypto_OT_Extension>"
  "$<TARGET_OBJECTS:SCD_SCD>"
  "$<TARGET_OBJECTS:Util_Util>"
  "$<TARGET_OBJECTS:TCPIP_TCPIP>"
  "$<TARGET_OBJECTS:Util_Log>")

if (OPENSSL_FOUND)
	target_link_libraries(TinyGarble ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)
if (Boost_FOUND)
  target_link_libraries(TinyGarble ${Boost_LIBRARIES})
endif (Boost_FOUND)
target_link_libraries(TinyGarble -lrt)

add_executable(A23_Test
  "a23_test_main.cpp"
  "$<TARGET_OBJECTS:Garbled_Circuit_Garbled_Circuit_Testing>"
  "$<TARGET_OBJECTS:Garbled_Circuit_Garbled_Circuit>"
  "$<TARGET_OBJECTS:Garbled_Circuit_Garbled_Circuit_Util>"
  "$<TARGET_OBJECTS:Garbled_Circuit_Garbled_Circuit_High_Mem>"
  "$<TARGET_OBJECTS:Garbled_Circuit_Garbled_Circuit_Low_Mem>"
  "$<TARGET_OBJECTS:SCD_SCD>"
  "$<TARGET_OBJECTS:SCD_SCD_Evaluator>"
  "$<TARGET_OBJECTS:Crypto_BN>"
  "$<TARGET_OBJECTS:Crypto_OT>"
  "$<TARGET_OBJECTS:Crypto_OT_Extension>"
  "$<TARGET_OBJECTS:Util_Util>"
  "$<TARGET_OBJECTS:TCPIP_TCPIP>"
  "$<TARGET_OBJECTS:TCPIP_TCPIP_Testsuit>"
  "$<TARGET_OBJECTS:Util_Log>")

if (OPENSSL_FOUND)
	target_link_libraries(A23_Test ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)
if (Boost_FOUND)
  target_link_libraries(A23_Test ${Boost_LIBRARIES})
endif (Boost_FOUND)
target_link_libraries(A23_Test -lrt)

add_executable(Garbled_Circuit_Test
  "garbled_circuit_test.cpp"
  "$<TARGET_OBJECTS:Garbled_Circuit_Garbled_Circuit_Testing>"
  "$<TARGET_OBJECTS:Garbled_Circuit_Garbled_Circuit>"
  "$<TARGET_OBJECTS:Garbled_Circuit_Garbled_Circuit_Util>"
  "$<TARGET_OBJECTS:Garbled_Circuit_Garbled_Circuit_High_Mem>"
  "$<TARGET_OBJECTS:Garbled_Circuit_Garbled_Circuit_Low_Mem>"
  "$<TARGET_OBJECTS:SCD_SCD>"
  "$<TARGET_OBJECTS:SCD_SCD_Evaluator>"
  "$<TARGET_OBJECTS:Crypto_BN>"
  "$<TARGET_OBJECTS:Crypto_OT>"
  "$<TARGET_OBJECTS:Crypto_OT_Extension>"
  "$<TARGET_OBJECTS:Util_Util>"
  "$<TARGET_OBJECTS:TCPIP_TCPIP>"
  "$<TARGET_OBJECTS:TCPIP_TCPIP_Testsuit>"
  "$<TARGET_OBJECTS:Util_Log>")

if (OPENSSL_FOUND)
	target_link_libraries(Garbled_Circuit_Test ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)
if (Boost_FOUND)
  target_link_libraries(Garbled_Circuit_Test ${Boost_LIBRARIES})
endif (Boost_FOUND)
target_link_libraries(Garbled_Circuit_Test -lrt)

add_test (Garbled_Circuit_Garbled_Circuit_Test
  Garbled_Circuit_Test --log2std)

set_tests_properties (Garbled_Circuit_Garbled_Circuit_Test
	PROPERTIES
	FAIL_REGULAR_EXPRESSION "[^a-z]Failed;failed")