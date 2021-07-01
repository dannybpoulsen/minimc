

add_library (storage blobstorage.cpp) 
target_include_directories (storage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries (storage  support hash)
