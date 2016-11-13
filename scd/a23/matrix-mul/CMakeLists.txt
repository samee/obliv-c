SET_SOURCE_FILES_PROPERTIES(matrix-mul.c 
  PROPERTIES LANGUAGE CXX)

add_executable(Matrix_Mul
  "matrix-mul.c"
  "$<TARGET_OBJECTS:A23_Lib_Main>"
  "$<TARGET_OBJECTS:Util_Util>"
  "$<TARGET_OBJECTS:Util_Log>")

if (OPENSSL_FOUND)
  target_link_libraries(Matrix_Mul ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)
if (Boost_FOUND)
  target_link_libraries(Matrix_Mul ${Boost_LIBRARIES})
endif (Boost_FOUND)
target_link_libraries(Matrix_Mul -lrt)
