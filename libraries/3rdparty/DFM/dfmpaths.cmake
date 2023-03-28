
if(NOT DEFINED DFM_CLOUDPORT)
    message(SEND_ERROR "DFM_CLOUDPORT is not defined")
endif()

if(NOT DEFINED DFM_KERNELPORT)
    message(SEND_ERROR "DFM_KERNELPORT is not defined")
endif()

if(NOT DEFINED DFM_STORAGEPORT)
    message(SEND_ERROR "DFM_STORAGEPORT is not defined")
endif()

set( DFM_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/cloudports/${DFM_CLOUDPORT}/dfmCloudPort.c"
    "${CMAKE_CURRENT_LIST_DIR}/kernelports/${DFM_KERNELPORT}/dfmKernelPort.c"
    "${CMAKE_CURRENT_LIST_DIR}/storageports/${DFM_STORAGEPORT}/dfmStoragePort.c"
    "${CMAKE_CURRENT_LIST_DIR}/dfm.c" )

set( DFM_INCLUDE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/include"
    "${CMAKE_CURRENT_LIST_DIR}/config"
    "${CMAKE_CURRENT_LIST_DIR}/cloudports/${DFM_CLOUDPORT}/include"
    "${CMAKE_CURRENT_LIST_DIR}/cloudports/${DFM_CLOUDPORT}/config"
    "${CMAKE_CURRENT_LIST_DIR}/kernelports/${DFM_KERNELPORT}/include"
    "${CMAKE_CURRENT_LIST_DIR}/storageports/${DFM_STORAGEPORT}/include"
    )
