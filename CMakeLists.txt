# @file CMakeLists.txt
# @brief 项目的根 CMake 配置文件
# @details
# 该文件负责定义整个 Parking-System 项目的构建规则。它包括：
# 1. 项目基本信息（名称、版本、语言）。
# 2. 编译器标准和编译选项的设置。
# 3. 第三方依赖库（如 cmocka）的集成。
# 4. 核心静态库 (parkingsystem_lib) 的定义。
# 5. 多个可执行文件（主程序、测试、示例）的生成。
# 6. 库的链接规则。
# 7. 自动化测试 (CTest) 的配置。
# 8. 打包 (CPack) 的配置。
# 9. 解决特定平台（如 Windows）的构建后依赖问题。

# ==========================================================================
#                            项目基本信息定义
# ==========================================================================
# 设置所需的 CMake 最低版本。
cmake_minimum_required(VERSION 3.14)
# 定义项目名称、版本和所用编程语言。
project(Parking-System VERSION 0.1.6 LANGUAGES C)

# ==========================================================================
#                            编译器和编译选项设置
# ==========================================================================
# 强制使用 C90 标准进行编译。
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# 为 MSVC 编译器定义 _CRT_SECURE_NO_WARNINGS 宏，
# 以抑制关于使用非安全函数（如 strcpy, sscanf）的警告。
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

# 根据不同的编译器类型添加特定的编译选项。
if(MSVC)
    # 告诉 MSVC 编译器源文件是 UTF-8 编码。
    add_compile_options(/utf-8)
    # 禁用 MSVC 的 4996 警告，这与 _CRT_SECURE_NO_WARNINGS 作用类似。
    add_compile_options(/wd4996)
else()
    # 为 GCC/Clang 等编译器明确指定 C90 标准。
    add_compile_options(-std=c90)
    # 禁用关于使用已弃用声明的警告（例如，在 cmocka 中可能出现）。
    add_compile_options(-Wno-deprecated-declarations)
endif()

# ==========================================================================
#                            第三方库 (cmocka)
# ==========================================================================
# 将 vendor/cmocka 目录作为一个子项目添加进来。
# 这将编译 cmocka 库，使其可用于后续的链接。
# 默认情况下，cmocka 会构建为动态库（DLL/so）。
add_subdirectory(vendor/cmocka)

# ==========================================================================
#                            核心库的定义
# ==========================================================================
# 将所有核心业务逻辑的源文件编译成一个名为 parkingsystem_lib 的静态库。
# 这样做可以实现模块化，便于在主程序和测试程序中复用。
add_library(parkingsystem_lib STATIC
    src/parking_data.c
    src/parking_service.c
    src/parking_ui.c
)

# 将 "src" 目录添加到库的头文件搜索路径中。
# PUBLIC 关键字意味着链接到此库的任何目标也会自动继承这个包含路径。
target_include_directories(parkingsystem_lib PUBLIC src)

# ==========================================================================
#                            可执行文件的定义
# ==========================================================================
# 定义项目的主可执行文件。
add_executable(Parking-System src/main.c)
# 定义各个模块的单元测试可执行文件。
add_executable(test_parking_data tests/test_parking_data.c)
add_executable(test_parking_service tests/test_parking_service.c)
add_executable(test_parking_ui tests/test_parking_ui.c)
# 定义用于演示功能的示例程序。
add_executable(data_layer_demo demos/data_layer_demo.c)
add_executable(service_demo demos/service_demo.c)

# ==========================================================================
#                            链接库到可执行文件
# ==========================================================================
# 为每个测试目标添加 cmocka 的头文件搜索路径。
# 这包括 cmocka 的源码 include 目录和构建目录（用于生成的 config.h）。
target_include_directories(test_parking_data PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/cmocka/include"
    "${CMAKE_CURRENT_BINARY_DIR}/vendor/cmocka"
)
target_include_directories(test_parking_service PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/cmocka/include"
    "${CMAKE_CURRENT_BINARY_DIR}/vendor/cmocka"
)
target_include_directories(test_parking_ui PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/cmocka/include"
    "${CMAKE_CURRENT_BINARY_DIR}/vendor/cmocka"
)

# 将核心静态库 (parkingsystem_lib) 和 cmocka 库链接到相应的可执行文件。
# PRIVATE 表示链接关系仅对当前目标有效，不会传递。
target_link_libraries(Parking-System PRIVATE parkingsystem_lib)
target_link_libraries(test_parking_data PRIVATE parkingsystem_lib cmocka)
target_link_libraries(test_parking_service PRIVATE parkingsystem_lib cmocka)
target_link_libraries(test_parking_ui PRIVATE parkingsystem_lib cmocka)
target_link_libraries(data_layer_demo PRIVATE parkingsystem_lib)
target_link_libraries(service_demo PRIVATE parkingsystem_lib)

# ==========================================================================
#                            构建后操作 (解决 DLL 问题)
# ==========================================================================
# 在 Windows 上，当 cmocka 被编译为动态库 (cmocka.dll) 时，
# 运行测试程序需要将该 DLL 文件放置在与可执行文件相同的目录中。
# 以下命令定义了一个构建后步骤，在每次成功构建测试目标后，
# 自动将 cmocka.dll 复制到测试程序的输出目录。
add_custom_command(
    TARGET test_parking_data POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE:cmocka>"
    "$<TARGET_FILE_DIR:test_parking_data>"
    COMMENT "Copying cmocka.dll to test directory"
)
add_custom_command(
    TARGET test_parking_service POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE:cmocka>"
    "$<TARGET_FILE_DIR:test_parking_service>"
    COMMENT "Copying cmocka.dll to test directory"
)
add_custom_command(
    TARGET test_parking_ui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE:cmocka>"
    "$<TARGET_FILE_DIR:test_parking_ui>"
    COMMENT "Copying cmocka.dll to test directory"
)

# ==========================================================================
#                            测试配置 (CTest)
# ==========================================================================
# 引入 CTest 模块以启用测试功能。
include(CTest)
# 在项目中启用测试。
enable_testing()

# 将每个测试可执行文件注册为一个 CTest 测试用例。
# 这样就可以通过 `ctest` 命令来统一运行所有测试。
add_test(NAME parking_data_test COMMAND test_parking_data)
add_test(NAME parking_service_test COMMAND test_parking_service)
add_test(NAME parking_ui_test COMMAND test_parking_ui)

# ==========================================================================
#                            打包配置 (CPack)
# ==========================================================================
# 设置 CPack 使用的项目名称和版本，这些变量继承自 project() 命令。
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# 引入 CPack 模块，以启用 `cpack` 命令来创建安装包或归档文件
include(CPack)
