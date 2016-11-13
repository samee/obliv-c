SET_SOURCE_FILES_PROPERTIES(hamming.c 
  PROPERTIES LANGUAGE CXX)

add_executable(Hamming
  "hamming.c"
  "$<TARGET_OBJECTS:A23_Lib_Main>"
  "$<TARGET_OBJECTS:Util_Util>"
  "$<TARGET_OBJECTS:Util_Log>")

if (OPENSSL_FOUND)
  target_link_libraries(Hamming ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)
if (Boost_FOUND)
  target_link_libraries(Hamming ${Boost_LIBRARIES})
endif (Boost_FOUND)
target_link_libraries(Hamming -lrt)