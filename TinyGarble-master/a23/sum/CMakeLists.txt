SET_SOURCE_FILES_PROPERTIES(sum.c 
  PROPERTIES LANGUAGE CXX)


add_executable(Sum
  "sum.c"
  "$<TARGET_OBJECTS:A23_Lib_Main>"
  "$<TARGET_OBJECTS:Util_Util>"
  "$<TARGET_OBJECTS:Util_Log>")

if (OPENSSL_FOUND)
  target_link_libraries(Sum ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)
if (Boost_FOUND)
  target_link_libraries(Sum ${Boost_LIBRARIES})
endif (Boost_FOUND)
target_link_libraries(Sum -lrt)