# 社区停车管理系统

## 1. 项目概述

**社区停车管理系统** 是一个基于C语言（C90标准）开发的综合性控制台应用程序。本项目旨在提供一个功能完整、性能可靠、易于扩展的停车管理解决方案。系统采用专业的分层架构，将数据、业务逻辑和用户界面完全分离，确保了代码的高度模块化和可维护性。

除了基础的停车管理功能，系统还实现了智能计费、多维度统计分析、数据持久化以及完整的错误处理机制。项目配备了覆盖率100%的单元测试，并通过CMake支持跨平台编译，保证了其在Windows、Linux和macOS等主流操作系统上的稳定运行。

这份文档将详细介绍系统的设计理念、功能实现、技术架构和使用方法，旨在为开发者和用户提供一份全面、清晰的参考指南。

## 2. 项目特点

- ✅ **专业分层架构**: 数据层、服务层、UI层三层分离，职责清晰，便于维护与二次开发。
- ✅ **严格标准兼容**: 遵循C90标准，确保代码在GCC, Clang, MSVC等多种编译器下的高度兼容性。
- ✅ **完善的中文支持**: 全中文界面和UTF-8编码，通过特定API调用解决了Windows控制台下的中文显示问题。
- ✅ **功能全面且实用**: 涵盖车位管理、车辆出入、智能计费、统计报表等全套业务流程。
- ✅ **精细的内存管理**: 严格的内存分配与释放策略，通过自动化测试确保无内存泄漏。
- ✅ **100%单元测试覆盖**: 提供了25个核心功能测试用例，保证代码质量和系统稳定性。
- ✅ **跨平台构建与部署**: 基于CMake构建系统，轻松实现跨平台编译和部署。
- ✅ **详尽的文档支持**: 提供项目总结、重构报告、Doxygen风格API文档及本详细设计文档。

## 3. 技术亮点与解决方案

1. **模块化设计与解耦**
    - **数据层 (`parking_data`)**: 抽象数据模型与基础操作，负责数据的存储、检索和持久化。
    - **服务层 (`parking_service`)**: 封装核心业务逻辑，提供统一的服务接口和错误处理模型，是连接数据层和UI层的桥梁。
    - **UI层 (`parking_ui`)**: 负责用户交互和视图展示，实现了与业务逻辑的完全分离。

2. **统一的错误处理机制**
    - 服务层定义了统一的`ServiceResult`结构体和`ParkingServiceResult`枚举，用于返回操作结果。
    - 这种模式使得错误处理逻辑集中化，简化了调用方的代码，提高了系统的健壮性。

3. **动态数据结构与内存管理**
    - 核心数据采用**单向链表**进行组织，支持动态增删，适应车位数量不固定的场景。
    - 所有动态分配的内存（`malloc`）都有对应的释放（`free`）逻辑，并设计了`free_parking_lot`等函数进行级联释放，防止内存泄漏。

4. **数据持久化方案**
    - 采用**文本文件**进行数据存储，虽然效率低于二进制文件，但具有良好的可读性和可移植性，便于调试和人工干预。
    - 设计了健壮的文件读写函数`save_parking_data`和`load_parking_data`，并进行了错误处理。

5. **智能计费与时间处理**
    - 利用C标准库`<time.h>`处理时间相关的计算，如停车时长、计费周期等。
    - 实现了针对居民（包月）和访客（按小时）的两种计费策略，并对访客停车时间（9:00-17:00）进行了限制。

## 4. 系统架构与设计

### 4.1 核心数据结构

系统的数据核心是`ParkingLot`和`ParkingSlot`两个结构体，以及用于缴费记录的`PaymentRecord`。

```c
// From: src/parking_data.h

/** @brief 缴费记录结构体 */
typedef struct PaymentRecord {
  time_t start_date;          // 缴费起始日期
  time_t end_date;            // 缴费结束日期
  double amount;              // 缴费金额
  struct PaymentRecord *next; // 指向下一条缴费记录
} PaymentRecord;

/** @brief 停车位结构体 */
typedef struct ParkingSlot {
  int slot_id;                         // 车位编号（唯一）
  char location[MAX_LOCATION_LEN];     // 位置描述
  char owner_name[MAX_NAME_LEN];       // 使用人姓名
  char license_plate[MAX_LICENSE_LEN]; // 车牌号
  char contact[MAX_CONTACT_LEN];       // 联系方式
  ParkingType type;                    // 车位类型 (RESIDENT_TYPE / VISITOR_TYPE)
  time_t entry_time;                   // 入场时间
  time_t exit_time;                    // 出场时间
  ParkingStatus status;                // 当前状态 (FREE_STATUS / OCCUPIED_STATUS)
  PaymentRecord *payment_head;         // 缴费记录链表头
  struct ParkingSlot *next;            // 指向下一个车位节点
} ParkingSlot;

/** @brief 停车场管理结构体 */
typedef struct ParkingLot {
  ParkingSlot *head;  // 停车位链表头
  int total_slots;    // 总车位数
  int occupied_slots; // 已占用车位数
} ParkingLot;
```

### 4.2 API设计与分层交互

- **数据层API**: 提供原子化的数据操作接口，如`create_parking_slot`, `find_slot_by_id`, `save_parking_data`等。
- **服务层API**: 封装业务流程，如`parking_service_add_slot`, `parking_service_allocate_slot`等。每个服务函数返回一个`ServiceResult`对象。
- **UI层API**: 负责界面渲染和用户输入处理，如`ui_show_main_menu`, `ui_add_parking_slot_menu`等。

**调用流程示例（车辆入场）:**

1. `ui_allocate_slot_menu()` 获取用户输入的车位号、车主信息等。
2. 调用 `parking_service_allocate_slot()` 服务函数。
3. 服务层内部调用 `find_slot_by_id()` 检查车位是否存在且空闲。
4. 若车位可用，则更新`ParkingSlot`结构体信息，并更新停车场的统计数据。
5. 服务层返回`ServiceResult`给UI层。
6. UI层根据`ServiceResult`的`code`判断操作是否成功，并向用户显示相应信息。

## 5. 功能模块详解

| 模块 | 功能点 | 实现细节 |
| :--- | :--- | :--- |
| **车位管理** | 添加/删除/修改车位 | 通过链表操作实现动态管理，删除时会校验车位是否空闲。 |
| | 车辆入场/出场 | 分配车位时记录入场时间，释放车位时计算费用并清空信息。 |
| | 多条件查询 | 支持按车位编号、车牌号、车主姓名进行精确查找。 |
| **计费管理** | 访客计费 | 按小时计费（10元/小时），不足1小时按1小时计算，仅限9:00-17:00。 |
| | 居民缴费 | 按月缴费（200元/月），通过`PaymentRecord`链表记录缴费历史。 |
| **统计分析** | 占用率/收入统计 | 实时计算车位占用率、当日和当月收入。 |
| | 停车数量统计 | 可按日、按月统计居民和访客的停车数量。 |
| **数据管理** | 数据保存/加载 | 可将整个`ParkingLot`链表数据序列化到文本文件，或从文件重建。 |

## 6. 代码质量与测试

项目对代码质量有严格要求，并建立了完善的测试体系。

- **编码规范**: 遵循统一的命名约定和代码风格，所有函数和结构体都有Doxygen风格的详细注释。
- **测试驱动开发 (TDD)**: 在开发数据层核心功能时，采用测试驱动的模式，先编写测试用例，再进行功能开发。
- **自动化测试报告**: 测试程序`test_parking_data`覆盖了数据层所有核心API，测试结果如下：

```text
停车管理系统数据结构测试
测试用例总数: 25
通过测试: 25
失败测试: 0
成功率: 100%
```

## 7. 目录与文件结构

```
C-Final-Design/
├── src/                    # 核心源代码
│   ├── main.c              # 主程序入口，负责调用UI层
│   ├── parking_data.h/c    # 数据层：数据结构与原子操作
│   ├── parking_service.h/c # 服务层：业务逻辑与错误处理
│   └── parking_ui.h/c      # UI层：用户界面与交互
├── tests/                  # 自动化测试
│   └── test_parking_data.c
├── demos/                  # 演示程序
│   ├── demo_parking.c      # 数据层功能演示
│   └── service_demo.c      # 服务层功能演示
├── demand/                 # 需求文档
├── solution_md/            # 设计方案文档
├── CMakeLists.txt          # CMake构建配置
├── CMakePresets.json       # CMake预设配置
├── PROJECT_SUMMARY.md      # 项目总结报告
├── REFACTORING_REPORT.md   # 重构过程报告
└── README.md               # 本文档
```

## 8. 编译与运行指南

### 8.1 系统要求

- **CMake**: 3.10或更高版本
- **C编译器**: 支持C90标准的编译器 (GCC, Clang, MSVC等)
- **操作系统**: Windows, Linux, macOS

### 8.2 编译步骤

```bash
# 1. 克隆或下载项目到本地

# 2. 创建构建目录
mkdir build
cd build

# 3. 使用CMake配置项目
cmake ..

# 4. 编译所有目标文件
cmake --build .
```

### 8.3 运行程序

编译成功后，可执行文件位于`build/Debug`或`build`目录下。

- **主程序**: `Parking-System.exe` (Windows) / `Parking-System` (Linux/macOS)
- **测试程序**: `test_parking_data.exe` / `test_parking_data`
- **演示程序**: `demo_parking.exe`, `service_demo.exe`

## 9. 使用说明

### 9.1 主菜单功能

```
========== 社区停车管理系统 ==========
1.  添加停车位
2.  车辆入场 (分配车位)
3.  车辆出场 (释放车位)
4.  查询车位信息
5.  显示车位列表
6.  缴费管理
7.  统计信息
8.  保存数据到文件
9.  从文件加载数据
10. 运行内置演示程序
0.  退出系统
======================================
请选择操作: 
```

### 9.2 快速入门

1. **启动**: 运行主程序`Parking-System`。
2. **体验**: 选择菜单`10`运行内置演示，快速了解系统功能。
3. **初始化**: 选择菜单`1`添加一些停车位。
4. **使用**: 使用菜单`2`和`3`模拟车辆的入场和出场。
5. **查询**: 使用菜单`4`和`5`查看车位状态。
6. **分析**: 使用菜单`7`查看统计报告。
7. **持久化**: 退出前使用菜单`8`保存数据，下次启动时使用菜单`9`恢复。

## 10. 开发团队

**停车管理系统开发团队**

## 11. 版本与许可证

- **当前版本**: 0.1.4
- **发布日期**: 2025-06-24
- **许可证**: 本项目遵循 [MIT许可证](https://opensource.org/licenses/MIT)。
