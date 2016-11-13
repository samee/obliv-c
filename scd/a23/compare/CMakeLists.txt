SET_SOURCE_FILES_PROPERTIES(compare.c 
  PROPERTIES LANGUAGE CXX)


add_executable(Compare
  "compare.c"
  "$<TARGET_OBJECTS:A23_Lib_Main>"
  "$<TARGET_OBJECTS:Util_Util>"
  "$<TARGET_OBJECTS:Util_Log>")

if (OPENSSL_FOUND)
  target_link_libraries(Compare ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)
if (Boost_FOUND)
  target_link_libraries(Compare ${Boost_LIBRARIES})
endif (Boost_FOUND)
target_link_libraries(Compare -lrt)