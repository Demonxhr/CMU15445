if(EXISTS "C:/Users/34279/Desktop/leveldb/cmu15445/cmake-build-debug/test/transaction_test[1]_tests.cmake")
  include("C:/Users/34279/Desktop/leveldb/cmu15445/cmake-build-debug/test/transaction_test[1]_tests.cmake")
else()
  add_test(transaction_test_NOT_BUILT transaction_test_NOT_BUILT)
endif()