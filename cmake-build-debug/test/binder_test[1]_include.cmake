if(EXISTS "C:/Users/34279/Desktop/leveldb/cmu15445/cmake-build-debug/test/binder_test[1]_tests.cmake")
  include("C:/Users/34279/Desktop/leveldb/cmu15445/cmake-build-debug/test/binder_test[1]_tests.cmake")
else()
  add_test(binder_test_NOT_BUILT binder_test_NOT_BUILT)
endif()
