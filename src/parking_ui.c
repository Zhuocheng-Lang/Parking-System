#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "parking_ui.h"

#ifdef _WIN32
#include <locale.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

/**
 * @file parking_ui.c
 * @brief UI层实现文件
 * @details
 * 该文件实现了在 parking_ui.h 中声明的所有用户界面功能。
 * 它负责处理用户输入、显示菜单、渲染数据，并通过调用服务层来执行业务逻辑。
 * UI层与核心业务逻辑完全分离，仅通过服务层接口进行交互。
 */

/* ========================================================================== */
/*                                 内部常量和全局变量                         */
/* ========================================================================== */

#define FILENAME_MAX_LEN 256         /**< 文件名的最大长度 */
#define GBK_CODE_PAGE 936            /**< Windows GBK 代码页 */
#define DEFAULT_PARKING_CAPACITY 100 /**< 默认停车场容量 */
#define SECONDS_PER_HOUR 3600.0      /**< 每小时的秒数 */

/**
 * @brief 全局停车场管理对象
 * @details 该静态全局变量存储了当前UI层正在操作的停车场实例。
 *          系统的所有操作都围绕这个对象展开。
 */
static ParkingLot *ui_parking_lot = NULL;

/* ========================================================================== */
/*                        系统初始化和主程序入口                              */
/* ========================================================================== */

/**
 * @brief 运行停车管理系统的主程序。
 * @details
 * 这是程序的总入口点。它首先进行初始化（设置编码、创建停车场对象），
 * 然后进入一个主循环，不断显示主菜单、获取用户选择并处理相应操作。
 * 当用户选择退出时，循环结束，并调用清理函数。
 */
void ui_run_parking_system(void) {
  int choice;
  int running = 1;

  ui_setup_console_encoding();
  ui_show_system_title();
  ui_initialize_parking_system(DEFAULT_PARKING_CAPACITY);

  while (running) {
    ui_show_main_menu();
    printf("请选择操作 (0-9): ");

    if (ui_safe_read_int(&choice) != 0) {
      ui_show_error("输入错误，请输入数字！");
      ui_wait_for_continue();
      continue;
    }

    if (choice == 0) {
      running = 0;
      ui_cleanup_and_exit();
    } else {
      ui_handle_menu_choice(choice);
      ui_wait_for_continue();
    }
  }
}

/**
 * @brief 初始化停车管理系统。
 * @details
 * 如果全局停车场对象 `ui_parking_lot` 为NULL，则调用数据层函数
 * `init_parking_lot` 创建一个新的停车场实例，并设置默认容量。
 * 如果内存分配失败，则打印错误并退出程序。
 * @param total_slots 要初始化的停车场总容量。
 */
void ui_initialize_parking_system(int total_slots) {
  if (ui_parking_lot == NULL) {
    ui_parking_lot = init_parking_lot(total_slots);
    if (ui_parking_lot == NULL) {
      ui_show_error("系统初始化失败，无法分配内存！");
      exit(EXIT_FAILURE);
    }
    printf("停车管理系统初始化成功，总车位数: %d\n\n",
           ui_parking_lot->total_slots);
  }
}

/**
 * @brief 清理资源并准备退出系统。
 * @details
 * 在程序退出前，此函数负责将当前停车场数据自动备份到
 * "parking_data_backup.txt" 文件中，然后调用 `free_parking_lot`
 * 释放所有相关的动态分配内存，防止内存泄漏。
 */
void ui_cleanup_and_exit(void) {
  printf("\n正在保存数据并退出系统...\n");

  if (ui_parking_lot) {
    parking_service_save_data(ui_parking_lot, "parking_data_backup.txt");
    free_parking_lot(ui_parking_lot);
    ui_parking_lot = NULL;
  }

  printf("感谢使用社区停车管理系统！\n");
}

/* ========================================================================== */
/*                            基础UI功能函数                                  */
/* ========================================================================== */

/**
 * @brief (跨平台) 清除控制台屏幕。
 */
void ui_clear_screen(void) {
#ifdef _WIN32
  system("cls");
#else
  system("clear");
#endif
}

/**
 * @brief 设置控制台编码以支持中文字符的正确显示。
 * @note 针对Windows和类Unix系统进行了适配。
 */
void ui_setup_console_encoding(void) {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
  setlocale(LC_ALL, "zh_CN.UTF-8");
  if (SetConsoleOutputCP(CP_UTF8) == 0) {
    SetConsoleOutputCP(GBK_CODE_PAGE);
    SetConsoleCP(GBK_CODE_PAGE);
    setlocale(LC_ALL, "zh_CN.GBK");
  }
#else
  setlocale(LC_ALL, "C.UTF-8");
#endif
}

/**
 * @brief 暂停程序执行，等待用户按Enter键继续。
 * @details 通常在显示完信息后调用，给用户阅读时间，然后清屏以准备显示新内容。
 */
void ui_wait_for_continue(void) {
  printf("\n按 Enter 键继续...");
  ui_clear_input_buffer();
}

/**
 * @brief 清除标准输入缓冲区中剩余的字符。
 * @details
 * 在使用 `scanf` 或 `fgets` 后，缓冲区中可能残留换行符或其他字符，
 * 此函数用于清除它们，以防止对后续的输入函数造成干扰。
 */
void ui_clear_input_buffer(void) {
  int c;
  while ((c = getchar()) != '\n' && c != EOF) {
  }
}

/**
 * @brief 安全地从标准输入读取一个整数。
 * @details
 * 读取整行输入，然后使用 `sscanf` 解析整数。
 * 这种方法比直接使用 `scanf("%d", ...)` 更健壮，可以处理无效输入
 * （例如非数字字符）并清除输入缓冲区，避免对后续输入造成影响。
 * @param[out] value 指向用于存储读取到的整数的变量的指针。
 * @return 成功返回0，失败（输入非数字或读取错误）返回-1。
 */
int ui_safe_read_int(int *value) {
  char buffer[100];
  if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
    return -1;
  }
  if (sscanf(buffer, "%d", value) != 1) {
    return -1;
  }
  return 0;
}

/**
 * @brief 安全地从标准输入读取一行字符串。
 * @details
 * 使用 `fgets` 读取一行，并自动移除末尾的换行符。
 * 这是一个比 `gets` 更安全的替代方案，因为它会检查缓冲区大小，防止溢出。
 * @param buffer 指向用于存储字符串的缓冲区的指针。
 * @param buffer_size 缓冲区的大小。
 * @return 成功返回0，若读取失败（如遇到EOF）则返回-1。
 */
int ui_safe_read_string(char *buffer, size_t buffer_size) {
  if (fgets(buffer, (int)buffer_size, stdin) == NULL) {
    return -1;
  }
  buffer[strcspn(buffer, "\n")] = '\0';
  return 0;
}

/* ========================================================================== */
/*                            主菜单相关函数                                  */
/* ========================================================================== */

/**
 * @brief 显示系统的主菜单选项。
 */
void ui_show_main_menu(void) {
  ui_clear_screen();
  printf("\n========== 主菜单 ==========\n");
  printf("1. 添加停车位\n");
  printf("2. 车辆入场（分配车位）\n");
  printf("3. 车辆出场（释放车位）\n");
  printf("4. 查询车位信息\n");
  printf("5. 显示车位列表\n");
  printf("6. 统计信息\n");
  printf("7. 保存数据\n");
  printf("8. 加载数据\n");
  printf("9. 运行演示程序\n");
  printf("0. 退出系统\n");
  printf("==========================\n");
}

/**
 * @brief 根据用户输入的选择，分发到对应的子菜单处理函数。
 * @param choice 用户从主菜单选择的选项编号。
 */
void ui_handle_menu_choice(int choice) {
  switch (choice) {
  case 1:
    ui_add_parking_slot_menu();
    break;
  case 2:
    ui_allocate_slot_menu();
    break;
  case 3:
    ui_deallocate_slot_menu();
    break;
  case 4:
    ui_query_slot_menu();
    break;
  case 5:
    ui_list_slots_menu();
    break;
  case 6:
    ui_statistics_menu();
    break;
  case 7:
    ui_save_data_menu();
    break;
  case 8:
    ui_load_data_menu();
    break;
  case 9:
    ui_run_demo_program();
    break;
  default:
    ui_show_error("无效选择，请重新输入！");
    break;
  }
}

/* ========================================================================== */
/*                           停车位管理菜单函数                               */
/* ========================================================================== */

/**
 * @brief 显示并处理“添加停车位”的交互流程。
 * @details
 * 引导用户输入新车位的编号和位置描述，然后调用服务层函数
 * `parking_service_add_slot` 来执行添加操作，并显示操作结果。
 */
void ui_add_parking_slot_menu(void) {
  int slot_id;
  char location[MAX_LOCATION_LEN];
  ServiceResult result;

  printf("\n========== 添加停车位 ==========\n");
  printf("请输入车位编号: ");
  if (ui_safe_read_int(&slot_id) != 0) {
    ui_show_error("输入错误！请输入一个数字。");
    return;
  }
  printf("请输入车位位置描述: ");
  if (ui_safe_read_string(location, sizeof(location)) != 0) {
    ui_show_error("读取位置信息失败！");
    return;
  }

  result = parking_service_add_slot(ui_parking_lot, slot_id, location);
  if (parking_service_is_success(result)) {
    ui_show_success(result.message);
  } else {
    parking_service_print_error(result);
  }
  parking_service_free_result(&result);
}

/**
 * @brief 显示并处理“车辆入场”（分配车位）的交互流程。
 * @details
 * 引导用户输入车位编号、车主信息、车牌号、联系方式和停车类型，
 * 然后调用服务层函数 `parking_service_allocate_slot` 来执行分配操作，
 * 并显示操作结果。
 */
void ui_allocate_slot_menu(void) {
  int slot_id;
  int type_choice;
  char owner_name[MAX_NAME_LEN];
  char license_plate[MAX_LICENSE_LEN];
  char contact[MAX_CONTACT_LEN];
  ParkingType parking_type;
  ServiceResult result;

  printf("\n========== 车辆入场 ==========\n");
  printf("请输入车位编号: ");
  if (ui_safe_read_int(&slot_id) != 0) {
    ui_show_error("输入错误！");
    return;
  }
  printf("请输入车主姓名: ");
  if (ui_safe_read_string(owner_name, sizeof(owner_name)) != 0) {
    ui_show_error("读取车主姓名失败！");
    return;
  }
  printf("请输入车牌号: ");
  if (ui_safe_read_string(license_plate, sizeof(license_plate)) != 0) {
    ui_show_error("读取车牌号失败！");
    return;
  }
  printf("请输入联系方式: ");
  if (ui_safe_read_string(contact, sizeof(contact)) != 0) {
    ui_show_error("读取联系方式失败！");
    return;
  }
  printf("请选择停车类型 (1: 居民, 2: 访客): ");
  if (ui_safe_read_int(&type_choice) != 0) {
    ui_show_error("输入错误！");
    return;
  }

  if (type_choice == 1) {
    parking_type = RESIDENT_TYPE;
  } else if (type_choice == 2) {
    parking_type = VISITOR_TYPE;
  } else {
    ui_show_error("无效的停车类型！");
    return;
  }

  result = parking_service_allocate_slot(ui_parking_lot, slot_id, owner_name,
                                         license_plate, contact, parking_type);
  if (parking_service_is_success(result)) {
    ui_show_success(result.message);
  } else {
    parking_service_print_error(result);
  }
  parking_service_free_result(&result);
}

/**
 * @brief 显示并处理“车辆出场”（释放车位）的交互流程。
 * @details
 * 引导用户输入要出场的车位编号，然后调用服务层函数
 * `parking_service_deallocate_slot` 来执行出场操作。
 * 如果操作成功且产生费用，将显示费用金额。
 */
void ui_deallocate_slot_menu(void) {
  int slot_id;
  ServiceResult result;

  printf("\n========== 车辆出场 ==========\n");
  printf("请输入要出场的车位编号: ");
  if (ui_safe_read_int(&slot_id) != 0) {
    ui_show_error("无效输入，请输入一个数字。");
    return;
  }

  result = parking_service_deallocate_slot(ui_parking_lot, slot_id);
  if (parking_service_is_success(result)) {
    double fee = result.data ? *((double *)result.data) : 0.0;
    printf("✓ %s\n", result.message);
    if (fee > 0) {
      printf("  本次停车费用为: %.2f 元\n", fee);
    }
  } else {
    parking_service_print_error(result);
  }
  parking_service_free_result(&result);
}

/**
 * @brief 显示并处理“查询车位信息”的交互流程。
 * @details
 * 引导用户输入要查询的车位编号，然后调用服务层函数
 * `parking_service_find_slot_by_id` 来查找车位，
 * 并调用 `ui_show_slot_status` 显示详细信息。
 */
void ui_query_slot_menu(void) {
  int slot_id;
  ServiceResult result;

  printf("\n========== 查询车位信息 ==========\n");
  printf("请输入车位编号: ");
  if (ui_safe_read_int(&slot_id) != 0) {
    ui_show_error("输入错误！");
    return;
  }

  result = parking_service_find_slot_by_id(ui_parking_lot, slot_id);
  if (parking_service_is_success(result)) {
    ui_show_slot_status((ParkingSlot *)result.data);
  } else {
    parking_service_print_error(result);
  }
  parking_service_free_result(&result);
}

/**
 * @brief 显示并处理“显示车位列表”的交互流程。
 * @details
 * 允许用户选择查看所有、空闲或已占用的车位列表。
 * 根据用户的选择，调用相应的服务层函数获取车位列表，
 * 然后遍历并显示每个车位的状态。
 */
void ui_list_slots_menu(void) {
  int choice;
  int count;
  ServiceResult result;
  SlotQueryResult *query_result;

  printf("\n========== 车位列表 ==========\n");
  printf("1. 显示所有车位\n2. 显示空闲车位\n3. 显示已占用车位\n请选择 (1-3): ");
  if (ui_safe_read_int(&choice) != 0) {
    ui_show_error("输入错误！");
    return;
  }

  switch (choice) {
  case 1:
    result = parking_service_get_all_slots(ui_parking_lot);
    break;
  case 2:
    result = parking_service_get_free_slots(ui_parking_lot);
    break;
  case 3:
    result = parking_service_get_occupied_slots(ui_parking_lot);
    break;
  default:
    ui_show_error("无效选择！");
    return;
  }

  if (parking_service_is_success(result)) {
    query_result = (SlotQueryResult *)result.data;
    count = query_result ? query_result->total_found : 0;
    printf("\n查询到 %d 个车位:\n", count);
    ui_show_separator();
    int i = 0;
    for (i = 0; i < count; i++) {
      ui_show_slot_status(query_result->slot_list[i]);
      ui_show_separator();
    }
  } else {
    parking_service_print_error(result);
  }
  parking_service_free_result(&result);
}

/* ========================================================================== */
/*                           支付和统计菜单函数                               */
/* ========================================================================== */

/**
 * @brief 显示并处理“统计信息”的交互流程。
 * @details
 * 调用服务层函数 `parking_service_get_statistics` 获取停车场的统计数据，
 * 包括总车位数、占用数、空闲数、使用率以及今日和本月收入，
 * 然后将这些信息格式化并显示给用户。
 */
void ui_statistics_menu(void) {
  ServiceResult result;
  ParkingStatistics *stats;

  printf("\n========== 统计信息 ==========\n");
  result = parking_service_get_statistics(ui_parking_lot);

  if (parking_service_is_success(result)) {
    stats = (ParkingStatistics *)result.data;
    printf("总车位数: %d\n", stats->total_slots);
    printf("已用车位数: %d\n", stats->occupied_slots);
    printf("空闲车位数: %d\n", stats->free_slots);
    printf("车位使用率: %.2f%%\n", stats->occupancy_rate);
    printf("今日收入: %.2f元\n", stats->today_revenue);
    printf("本月收入: %.2f元\n", stats->month_revenue);
    printf("==========================\n");
  } else {
    parking_service_print_error(result);
  }
  parking_service_free_result(&result);
}

/* ========================================================================== */
/*                            数据管理菜单函数                                */
/* ========================================================================== */

/**
 * @brief 显示并处理“保存数据到文件”的交互流程。
 * @details
 * 引导用户输入要保存数据的文件名（提供默认值），然后调用服务层函数
 * `parking_service_save_data` 将当前停车场的所有数据持久化到文本文件。
 */
void ui_save_data_menu(void) {
  char filename[FILENAME_MAX_LEN];
  ServiceResult result;

  printf("\n========== 保存数据 ==========\n");
  printf("请输入文件名 (默认: parking_data.txt): ");
  ui_safe_read_string(filename, sizeof(filename));
  if (strlen(filename) == 0) {
    strcpy(filename, "parking_data.txt");
  }

  result = parking_service_save_data(ui_parking_lot, filename);
  if (parking_service_is_success(result)) {
    printf("数据保存成功！文件：%s\n", filename);
  } else {
    parking_service_print_error(result);
  }
  parking_service_free_result(&result);
}

/**
 * @brief 显示并处理“从文件加载数据”的交互流程。
 * @details
 * 引导用户输入要加载数据的文件名（提供默认值）。
 * 调用服务层函数 `parking_service_load_data` 从文件加载数据。
 * 加载成功后，会释放旧的停车场内存，并用新加载的数据替换全局的
 * `ui_parking_lot` 对象。
 */
void ui_load_data_menu(void) {
  char filename[FILENAME_MAX_LEN];
  ServiceResult result;

  printf("\n========== 加载数据 ==========\n");
  printf("请输入文件名 (默认: parking_data.txt): ");
  ui_safe_read_string(filename, sizeof(filename));
  if (strlen(filename) == 0) {
    strcpy(filename, "parking_data.txt");
  }

  result = parking_service_load_data(filename);
  if (parking_service_is_success(result)) {
    if (ui_parking_lot) {
      free_parking_lot(ui_parking_lot);
    }
    ui_parking_lot = (ParkingLot *)result.data;
    printf("数据加载成功！总车位数: %d, 已占用: %d\n",
           ui_parking_lot->total_slots, ui_parking_lot->occupied_slots);
    /* 防止重复释放 result.data */
    result.data = NULL;
  } else {
    parking_service_print_error(result);
  }
  parking_service_free_result(&result);
}

/* ========================================================================== */
/*                            演示程序功能                                    */
/* ========================================================================== */

/**
 * @brief 运行一个内置的演示程序，展示系统的核心功能。
 * @details
 * 此函数通过调用一系列服务层函数，自动执行添加车位、分配车位等操作，
 * 并最后显示统计信息。它用于快速验证系统的基本功能是否正常工作，
 * 无需用户手动输入。
 */
void ui_run_demo_program(void) {
  ServiceResult result;
  printf("\n========== 内置演示程序 ==========\n");

  printf("1. 添加演示车位...\n");
  parking_service_add_slot(ui_parking_lot, 201, "演示区-201");
  parking_service_add_slot(ui_parking_lot, 202, "演示区-202");

  printf("2. 分配演示车位...\n");
  result =
      parking_service_allocate_slot(ui_parking_lot, 201, "演示用户",
                                    "京A-DEMO1", "13800138000", RESIDENT_TYPE);
  if (parking_service_is_success(result)) {
    printf("   ✓ 车位201分配成功\n");
  } else {
    parking_service_print_error(result);
  }

  printf("3. 显示当前统计信息...\n");
  ui_statistics_menu();
  printf("\n演示程序运行完成！\n");
}

/* ========================================================================== */
/*                            辅助显示函数                                    */
/* ========================================================================== */

/**
 * @brief 显示系统的欢迎标题。
 */
void ui_show_system_title(void) {
  printf("\n====================================================\n");
  printf("               社区停车管理系统 V1.0                \n");
  printf("====================================================\n\n");
}

/**
 * @brief 以标准格式显示单个停车位的详细信息。
 * @details
 * 如果车位被占用，还会显示车主、车牌、联系方式、类型和入场时间。
 * @param slot 指向要显示的 ParkingSlot 对象的指针。
 */
void ui_show_slot_status(const ParkingSlot *slot) {
  if (slot == NULL) {
    ui_show_error("车位信息为空");
    return;
  }
  printf("车位编号: %-5d | 位置: %-20s | 状态: %s\n", slot->slot_id,
         slot->location, slot->status == OCCUPIED_STATUS ? "已占用" : "空闲");

  if (slot->status == OCCUPIED_STATUS) {
    char time_buffer[30];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S",
             localtime(&slot->entry_time));
    printf("  -> 车主: %s, 车牌: %s, 联系方式: %s\n", slot->owner_name,
           slot->license_plate, slot->contact);
    printf("  -> 类型: %s, 入场时间: %s\n",
           slot->type == RESIDENT_TYPE ? "居民" : "访客", time_buffer);
  }
}

/**
 * @brief 以标准格式显示一条错误消息。
 * @param message 要显示的错误信息字符串。
 */
void ui_show_error(const char *message) {
  if (message != NULL) {
    printf("[错误] %s\n", message);
  }
}

/**
 * @brief 以标准格式显示一条成功操作的消息。
 * @param message 要显示的成功信息字符串。
 */
void ui_show_success(const char *message) {
  if (message != NULL) {
    printf("[成功] %s\n", message);
  }
}

/**
 * @brief 打印一条分隔线，用于美化界面布局。
 */
void ui_show_separator(void) {
  printf("----------------------------------------------------\n");
}

/* ========================================================================== */
/*                           获取全局停车场对象                               */
/* ========================================================================== */

/**
 * @brief 获取UI层当前管理的停车场对象的指针。
 * @details
 * 提供对静态全局变量 `ui_parking_lot` 的外部访问接口。
 * 主要用于测试或需要在UI层之外直接访问全局停车场对象的场景。
 * @return 指向全局 ParkingLot 对象的指针。
 */
ParkingLot *ui_get_parking_lot(void) { return ui_parking_lot; }
