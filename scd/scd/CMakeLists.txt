add_library (SCD_ParseNetlist OBJECT
  "parse_netlist.cpp")

add_library (SCD_Scheduling OBJECT
  "scheduling.cpp")

add_library (SCD_SCD OBJECT
  "scd.cpp")

add_library (SCD_SCD_Evaluator OBJECT
  "scd_evaluator.cpp")

add_library (SCD_V2SCD OBJECT
  "v_2_scd.cpp")

add_executable(V2SCD_Main
  "v_2_scd_main.cpp"
  "$<TARGET_OBJECTS:SCD_V2SCD>"
  "$<TARGET_OBJECTS:SCD_ParseNetlist>"
  "$<TARGET_OBJECTS:SCD_Scheduling>"
  "$<TARGET_OBJECTS:SCD_SCD>"
  "$<TARGET_OBJECTS:Util_Log>")

if (Boost_FOUND)
  target_link_libraries(V2SCD_Main ${Boost_LIBRARIES})
endif (Boost_FOUND)
target_link_libraries(V2SCD_Main -lrt)

add_executable(SCD_Evaluator_Main
  "scd_evaluator_main.cpp"
  "$<TARGET_OBJECTS:SCD_SCD_Evaluator>"
  "$<TARGET_OBJECTS:SCD_ParseNetlist>"
  "$<TARGET_OBJECTS:SCD_Scheduling>"
  "$<TARGET_OBJECTS:SCD_SCD>"
  "$<TARGET_OBJECTS:Util_Util>"
  "$<TARGET_OBJECTS:Util_Log>")

if (Boost_FOUND)
  target_link_libraries(SCD_Evaluator_Main ${Boost_LIBRARIES})
endif (Boost_FOUND)
if (OPENSSL_FOUND)
  target_link_libraries(SCD_Evaluator_Main ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)
target_link_libraries(SCD_Evaluator_Main -lrt)

add_executable(V2SCD_Test
  "v_2_scd_test.cpp"
  "$<TARGET_OBJECTS:SCD_V2SCD>"
  "$<TARGET_OBJECTS:SCD_ParseNetlist>"
  "$<TARGET_OBJECTS:SCD_Scheduling>"
  "$<TARGET_OBJECTS:SCD_SCD>"
  "$<TARGET_OBJECTS:Util_Log>")

if (Boost_FOUND)
  target_link_libraries(V2SCD_Test ${Boost_LIBRARIES})
endif (Boost_FOUND)
target_link_libraries(V2SCD_Test -lrt)

add_test (SCD_V2SCD_Test
  V2SCD_Test --error2std)

set_tests_properties (SCD_V2SCD_Test
  PROPERTIES
  FAIL_REGULAR_EXPRESSION "[^a-z]Failed;failed")

add_executable(SCD_Evaluator_Test
  "scd_evaluator_test.cpp"
  "$<TARGET_OBJECTS:SCD_SCD_Evaluator>"
  "$<TARGET_OBJECTS:SCD_ParseNetlist>"
  "$<TARGET_OBJECTS:SCD_Scheduling>"
  "$<TARGET_OBJECTS:SCD_SCD>"
  "$<TARGET_OBJECTS:Util_Util>"
  "$<TARGET_OBJECTS:Util_Log>")

if (Boost_FOUND)
  target_link_libraries(SCD_Evaluator_Test ${Boost_LIBRARIES})
endif (Boost_FOUND)
if (OPENSSL_FOUND)
  target_link_libraries(SCD_Evaluator_Test ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)
target_link_libraries(SCD_Evaluator_Test -lrt)

add_test (SCD_SCD_Evaluator_Test
  SCD_Evaluator_Test --log2std)

set_tests_properties (SCD_SCD_Evaluator_Test
  PROPERTIES
  FAIL_REGULAR_EXPRESSION "[^a-z]Failed;failed")

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/netlists)

execute_process(COMMAND
                tar -xf ${CMAKE_CURRENT_SOURCE_DIR}/netlists/v.tar.gz 
				     -C ${CMAKE_CURRENT_BINARY_DIR}/netlists)

configure_file(
  "V2SCD_ALL.sh.in"
  "V2SCD_ALL.sh"
  @ONLY)

add_custom_command(TARGET V2SCD_Main
                   COMMAND
                   sh ${CMAKE_CURRENT_BINARY_DIR}/V2SCD_ALL.sh)