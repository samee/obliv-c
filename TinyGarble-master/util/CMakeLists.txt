add_library (Util_Log OBJECT
  "log.cpp")

add_library (Util_Util OBJECT
  "util.cpp")

add_executable(Util_Test
  "util_test.cpp"
  "$<TARGET_OBJECTS:Util_Util>"
  "$<TARGET_OBJECTS:Util_Log>")
if (Boost_FOUND)
	target_link_libraries(Util_Test ${Boost_LIBRARIES})
endif (Boost_FOUND)
if (OPENSSL_FOUND)
	target_link_libraries(Util_Test ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)
target_link_libraries(Util_Test -lrt)

add_test (Util_Util_Test
  Util_Test --log2std)

set_tests_properties (Util_Util_Test
	PROPERTIES
	FAIL_REGULAR_EXPRESSION "[^a-z]Error;ERROR;Failed;failed")
