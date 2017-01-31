SET_SOURCE_FILES_PROPERTIES(aes.c 
  PROPERTIES LANGUAGE CXX)

add_executable(AES
  "aes.c"
  "$<TARGET_OBJECTS:A23_Lib_Main>"
  "$<TARGET_OBJECTS:Util_Util>"
  "$<TARGET_OBJECTS:Util_Log>")

if (OPENSSL_FOUND)
  target_link_libraries(AES ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)
if (Boost_FOUND)
  target_link_libraries(AES ${Boost_LIBRARIES})
endif (Boost_FOUND)
target_link_libraries(AES -lrt)