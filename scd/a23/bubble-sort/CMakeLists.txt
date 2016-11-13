SET_SOURCE_FILES_PROPERTIES(bubble-sort.c 
  PROPERTIES LANGUAGE CXX)

add_executable(Bubble_Sort
  "bubble-sort.c"
  "$<TARGET_OBJECTS:A23_Lib_Main>"
  "$<TARGET_OBJECTS:Util_Util>"
  "$<TARGET_OBJECTS:Util_Log>")

if (OPENSSL_FOUND)
  target_link_libraries(Bubble_Sort ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)
if (Boost_FOUND)
  target_link_libraries(Bubble_Sort ${Boost_LIBRARIES})
endif (Boost_FOUND)
target_link_libraries(Bubble_Sort -lrt)