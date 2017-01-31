SET_SOURCE_FILES_PROPERTIES(mem-test.c 
  PROPERTIES LANGUAGE CXX)

add_executable(Mem_Test
  "mem-test.c"
  "$<TARGET_OBJECTS:A23_Lib_Main>"
  "$<TARGET_OBJECTS:Util_Util>"
  "$<TARGET_OBJECTS:Util_Log>")

if (OPENSSL_FOUND)
  target_link_libraries(Mem_Test ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)
if (Boost_FOUND)
  target_link_libraries(Mem_Test ${Boost_LIBRARIES})
endif (Boost_FOUND)
target_link_libraries(Mem_Test -lrt)