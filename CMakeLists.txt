cmake_minimum_required(VERSION 3.14)

project(xianyu_server_arch LANGUAGES CXX)
#开启MOC、RCC和UIC，则会在预处理之前先进行MOC、RCC和UIC操作
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#设置可执行文件的输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

#查找yaml-cpp包0.7.0
find_package(yaml-cpp REQUIRED)
set(Yaml_cpp_INCLUDE_DIR "/usr/local/include/yaml-cpp")
set(Yaml_cpp_LIB "/usr/local/lib/libyaml-cpp.a")
include_directories(${Yaml_cpp_INCLUDE_DIR})

#查找boost包1.75.0
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
add_definitions("-DHAS_BOOST")

#查找需要的Qt包
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

#将各个文件夹下的所有文件打包
file(GLOB ${PROJECT_NAME}_SOURCES "./src/*")
file(GLOB ${PROJECT_NAME}_HEADER "./include/*")

#将/include路径包含到查找头文件路径
include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME}
    ${${PROJECT_NAME}_SOURCES}
    ${${PROJECT_NAME}_HEADER}
)

#和Qt的库进行链接
target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Core)

target_link_libraries(${PROJECT_NAME} ${Yaml_cpp_LIB})

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
