SET_SOURCE_FILES_PROPERTIES(dijkstra.c 
  PROPERTIES LANGUAGE CXX)
  
add_executable(Dijkstra
  "dijkstra.c"
  "$<TARGET_OBJECTS:A23_Lib_Main>"
  "$<TARGET_OBJECTS:Util_Util>"
  "$<TARGET_OBJECTS:Util_Log>")

if (OPENSSL_FOUND)
  target_link_libraries(Dijkstra ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)
if (Boost_FOUND)
  target_link_libraries(Dijkstra ${Boost_LIBRARIES})
endif (Boost_FOUND)
target_link_libraries(Dijkstra -lrt)