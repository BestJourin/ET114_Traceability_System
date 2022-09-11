cmake_minimum_required(VERSION 3.0)

set(DEMO_NAME demo)

project(${DEMO_NAME} CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)

set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTORCC ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(WITH_MKL        "Compile demo with MKL/OpenBlas support, default use MKL."               ON)
option(WITH_GPU        "Compile demo with GPU/CPU, default use CPU."                            OFF)
option(WITH_STATIC_LIB "Compile demo with static/shared library, default don't use static."     OFF)
option(USE_TENSORRT "Compile demo with TensorRT."                                               OFF)
option(WITH_ROCM "Compile demo with rocm."                                                      OFF)
option(WITH_ONNXRUNTIME "Compile demo with ONNXRuntime"                                         OFF)
option(WITH_ARM "Compile demo with ARM"                                                         OFF)
option(WITH_MIPS "Compile demo with MIPS"                                                       OFF)
option(WITH_SW   "Compile demo with SW"                                                         OFF) 
option(WITH_XPU "Compile demow ith xpu"                                                         OFF)
option(WITH_NPU "Compile demow ith npu"                                                         OFF)



# yaml-cpp
include(cmake/yaml-cpp.cmake)
include_directories("${CMAKE_SOURCE_DIR}/")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/ext/yaml-cpp/src/ext-yaml-cpp/include")
link_directories("${CMAKE_CURRENT_BINARY_DIR}/ext/yaml-cpp/lib")


# opencv
find_package(OpenCV REQUIRED)



macro(safe_set_static_flag)
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      if(${flag_var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
      endif(${flag_var} MATCHES "/MD")
    endforeach(flag_var)
endmacro()




# 指定paddle interface目录
set(PADDLE_LIB ${CMAKE_SOURCE_DIR}/../../lib/paddle_inference)

include_directories("${PADDLE_LIB}/")
set(PADDLE_LIB_THIRD_PARTY_PATH "${PADDLE_LIB}/third_party/install/")







include_directories("${PADDLE_LIB}/")
include_directories("${PADDLE_LIB}/paddle/include/")
set(PADDLE_LIB_THIRD_PARTY_PATH "${PADDLE_LIB}/third_party/install/")
include_directories("${PADDLE_LIB_THIRD_PARTY_PATH}protobuf/include")
include_directories("${PADDLE_LIB_THIRD_PARTY_PATH}glog/include")
include_directories("${PADDLE_LIB_THIRD_PARTY_PATH}gflags/include")
include_directories("${PADDLE_LIB_THIRD_PARTY_PATH}xxhash/include")
include_directories("${PADDLE_LIB_THIRD_PARTY_PATH}cryptopp/include")
include_directories("${PADDLE_LIB_THIRD_PARTY_PATH}onnxruntime/include")
include_directories("${PADDLE_LIB_THIRD_PARTY_PATH}paddle2onnx/include")


link_directories("${PADDLE_LIB_THIRD_PARTY_PATH}protobuf/lib")
link_directories("${PADDLE_LIB_THIRD_PARTY_PATH}glog/lib")
link_directories("${PADDLE_LIB_THIRD_PARTY_PATH}gflags/lib")
link_directories("${PADDLE_LIB_THIRD_PARTY_PATH}xxhash/lib")
link_directories("${PADDLE_LIB_THIRD_PARTY_PATH}cryptopp/lib")
link_directories("${PADDLE_LIB}/paddle/lib")
link_directories("${PADDLE_LIB_THIRD_PARTY_PATH}onnxruntime/lib")
link_directories("${PADDLE_LIB_THIRD_PARTY_PATH}paddle2onnx/lib")



# linux


if(WITH_MKL)
    set(FLAG_OPENMP "-fopenmp")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ${FLAG_OPENMP}")



if(WITH_GPU)
  if(NOT WIN32)
    include_directories("/usr/local/cuda/include")
    if(CUDA_LIB STREQUAL "")
      set(CUDA_LIB "/usr/local/cuda/lib64/" CACHE STRING "CUDA Library")
    endif()
  else()
    include_directories("C:\\Program\ Files\\NVIDIA GPU Computing Toolkit\\CUDA\\v8.0\\include")
    if(CUDA_LIB STREQUAL "")
      set(CUDA_LIB "C:\\Program\ Files\\NVIDIA GPU Computing Toolkit\\CUDA\\v8.0\\lib\\x64")
    endif()
  endif(NOT WIN32)
endif()



if(WITH_MKL)
  set(MATH_LIB_PATH "${PADDLE_LIB_THIRD_PARTY_PATH}mklml")
  include_directories("${MATH_LIB_PATH}/include")
  if(WIN32)
    set(MATH_LIB ${MATH_LIB_PATH}/lib/mklml${CMAKE_STATIC_LIBRARY_SUFFIX}
                 ${MATH_LIB_PATH}/lib/libiomp5md${CMAKE_STATIC_LIBRARY_SUFFIX})
  else()
    set(MATH_LIB ${MATH_LIB_PATH}/lib/libmklml_intel${CMAKE_SHARED_LIBRARY_SUFFIX}
                 ${MATH_LIB_PATH}/lib/libiomp5${CMAKE_SHARED_LIBRARY_SUFFIX})
  endif()
  set(MKLDNN_PATH "${PADDLE_LIB_THIRD_PARTY_PATH}mkldnn")
  if(EXISTS ${MKLDNN_PATH})
    include_directories("${MKLDNN_PATH}/include")
    if(WIN32)
      set(MKLDNN_LIB ${MKLDNN_PATH}/lib/mkldnn.lib)
    else(WIN32)
      set(MKLDNN_LIB ${MKLDNN_PATH}/lib/libmkldnn.so.0)
    endif(WIN32)
  endif()
elseif((NOT WITH_MIPS) AND (NOT WITH_SW))
  set(OPENBLAS_LIB_PATH "${PADDLE_LIB_THIRD_PARTY_PATH}openblas")
  include_directories("${OPENBLAS_LIB_PATH}/include/openblas")
  if(WIN32)
    set(MATH_LIB ${OPENBLAS_LIB_PATH}/lib/openblas${CMAKE_STATIC_LIBRARY_SUFFIX})
  else()
    set(MATH_LIB ${OPENBLAS_LIB_PATH}/lib/libopenblas${CMAKE_STATIC_LIBRARY_SUFFIX})
  endif()
endif()



if(WITH_STATIC_LIB)
  set(DEPS ${PADDLE_LIB}/paddle/lib/libpaddle_inference${CMAKE_STATIC_LIBRARY_SUFFIX})
else()
  if(WIN32)
    set(DEPS ${PADDLE_LIB}/paddle/lib/paddle_inference${CMAKE_STATIC_LIBRARY_SUFFIX})
  else()
    set(DEPS ${PADDLE_LIB}/paddle/lib/libpaddle_inference${CMAKE_SHARED_LIBRARY_SUFFIX})
  endif()
endif()

if (WITH_ONNXRUNTIME)
  if(WIN32)
    set(DEPS ${DEPS} ${PADDLE_LIB_THIRD_PARTY_PATH}onnxruntime/lib/onnxruntime.lib paddle2onnx)
  elseif(APPLE)
    set(DEPS ${DEPS} ${PADDLE_LIB_THIRD_PARTY_PATH}onnxruntime/lib/libonnxruntime.1.11.1.dylib paddle2onnx)
  else()
    set(DEPS ${DEPS} ${PADDLE_LIB_THIRD_PARTY_PATH}onnxruntime/lib/libonnxruntime.so.1.11.1 paddle2onnx) #linux
  endif()
endif()


if (NOT WIN32)
  set(EXTERNAL_LIB "-lrt -ldl -lpthread")
  set(DEPS ${DEPS}
      ${MATH_LIB} ${MKLDNN_LIB}
      glog gflags protobuf xxhash cryptopp
      ${EXTERNAL_LIB})
else()
  set(DEPS ${DEPS}
      ${MATH_LIB} ${MKLDNN_LIB}
      glog gflags_static libprotobuf xxhash cryptopp-static ${EXTERNAL_LIB})
  set(DEPS ${DEPS} shlwapi.lib)
endif(NOT WIN32)



set(DEPS ${DEPS} ${PADDLE_LIB_THIRD_PARTY_PATH}onnxruntime/lib/libonnxruntime.so.1.11.1)
set(DEPS ${DEPS} ${PADDLE_LIB_THIRD_PARTY_PATH}paddle2onnx/lib/libpaddle2onnx.so.1.0.0rc2)
set(DEPS ${DEPS} ${OpenCV_LIBS} yaml-cpp sqlite3)


set(SRCS src/preprocess_op.cc src/object_detector.cc src/picodet_postprocess.cc src/utils.cc 
    src/my_sqlite.cc   
)


# Qt5Core
find_package(Qt5Core REQUIRED)
if(Qt5Core_FOUND OR Qt5Core_CXX_FOUND)
    include_directories(${Qt5Core_INCLUDE_DIRS})
    message(STATUS "Qt library status:")
    message(STATUS "    include path: ${Qt5Core_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "Could not found Qt5Core!")
    return()
endif()

# Qt5Gui
find_package(Qt5Gui REQUIRED)
if(Qt5Gui_FOUND OR Qt5Gui_CXX_FOUND)
    include_directories(${Qt5Gui_INCLUDE_DIRS})
    message(STATUS "    include path: ${Qt5Gui_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "Could not found Qt5Gui!")
    return()
endif()

# Qt5Widgets
find_package(Qt5Widgets REQUIRED)
if(Qt5Widgets_FOUND OR Qt5Widgets_CXX_FOUND)
    include_directories(${Qt5Widgets_INCLUDE_DIRS})
    message(STATUS "    include path: ${Qt5Widgets_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "Could not found Qt5Widgets!")
    return()
endif()

# 添加头文件
set(HEADERS 
    include/gui_main.h)

# 添加UI文件
set(UI 
    src/gui_main.ui)

# 添加源文件
set(SOURCES
    main.cpp 
    src/gui_main.cpp)


# 
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR})


add_executable(${DEMO_NAME}  ${SOURCES} ${HEADERS} ${UI} ${SRCS})
ADD_DEPENDENCIES(${DEMO_NAME} ext-yaml-cpp)
target_link_libraries(${DEMO_NAME} Qt5::Widgets)

target_link_libraries(${DEMO_NAME} Qt5::Core)

target_link_libraries(${DEMO_NAME} Qt5::Gui)
target_link_libraries(${DEMO_NAME} ${DEPS})







if(WIN32)
  if(USE_TENSORRT)
    add_custom_command(TARGET ${DEMO_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${TENSORRT_LIB_DIR}/nvinfer${CMAKE_SHARED_LIBRARY_SUFFIX}
              ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}
            COMMAND ${CMAKE_COMMAND} -E copy ${TENSORRT_LIB_DIR}/nvinfer_plugin${CMAKE_SHARED_LIBRARY_SUFFIX}
              ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}
    )
    if(${TENSORRT_MAJOR_VERSION} GREATER_EQUAL 7)
      add_custom_command(TARGET ${DEMO_NAME} POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E copy ${TENSORRT_LIB_DIR}/myelin64_1${CMAKE_SHARED_LIBRARY_SUFFIX}
                ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    endif()
  endif()
  if(WITH_MKL)
    add_custom_command(TARGET ${DEMO_NAME} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy ${MATH_LIB_PATH}/lib/mklml.dll ${CMAKE_BINARY_DIR}/Release
          COMMAND ${CMAKE_COMMAND} -E copy ${MATH_LIB_PATH}/lib/libiomp5md.dll ${CMAKE_BINARY_DIR}/Release
          COMMAND ${CMAKE_COMMAND} -E copy ${MKLDNN_PATH}/lib/mkldnn.dll  ${CMAKE_BINARY_DIR}/Release
    )
  else()
    add_custom_command(TARGET ${DEMO_NAME} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy ${OPENBLAS_LIB_PATH}/lib/openblas.dll ${CMAKE_BINARY_DIR}/Release
    )
  endif()
  if(WITH_ONNXRUNTIME)
    add_custom_command(TARGET ${DEMO_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${PADDLE_LIB_THIRD_PARTY_PATH}onnxruntime/lib/onnxruntime.dll
      ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}
    COMMAND ${CMAKE_COMMAND} -E copy ${PADDLE_LIB_THIRD_PARTY_PATH}paddle2onnx/lib/paddle2onnx.dll
      ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}
    )
  endif()
  if(NOT WITH_STATIC_LIB)
      add_custom_command(TARGET ${DEMO_NAME} POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy "${PADDLE_LIB}/paddle/lib/paddle_inference.dll" ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}
      )
  endif()
endif()
