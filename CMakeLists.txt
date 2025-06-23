cmake_minimum_required(VERSION 3.10.0)
project(Parking-System VERSION 0.1.4 LANGUAGES C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# 抑制MSVC安全警告
if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_definitions(_CRT_NONSTDC_NO_DEPRECATE)
    # 设置源文件编码为UTF-8
    add_compile_options(/utf-8)
else()
    add_compile_options(-std=c90)
endif()

# 设置源文件
set(PARKING_SOURCES
    src/parking_data.c
    src/parking_service.c
    src/parking_ui.c
)

# 主程序
add_executable(Parking-System 
    src/main.c
    ${PARKING_SOURCES}
)

# 测试程序 (从tests目录)
add_executable(test_parking_data
    tests/test_parking_data.c
    src/parking_data.c
)

# 演示程序 (从demos目录)
add_executable(demo_parking
    demos/demo_parking.c
    src/parking_data.c
)

# 服务层演示程序 (从demos目录)
add_executable(service_demo
    demos/service_demo.c
    ${PARKING_SOURCES}
)

# 设置包含目录
target_include_directories(Parking-System PRIVATE src)
target_include_directories(test_parking_data PRIVATE src)
target_include_directories(demo_parking PRIVATE src)
target_include_directories(service_demo PRIVATE src)

include(CTest)
enable_testing()

# 添加测试
add_test(NAME parking_data_test COMMAND test_parking_data)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
