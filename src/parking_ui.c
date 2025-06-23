#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS /* 禁用MSVC安全警告 */
#endif

#include "parking_ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <locale.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

/*
 * 停车管理系统UI层实现
 * 提供用户界面相关的所有功能
 * 分离UI逻辑和业务逻辑，使用服务层接口
 */

/* 常量定义 */
#define FILENAME_MAX_LEN 256
#define GBK_CODE_PAGE 936
#define DEFAULT_PARKING_CAPACITY 1000
#define SECONDS_PER_HOUR 3600.0
#define MENU_OPTION_1 1
#define MENU_OPTION_2 2
#define MENU_OPTION_3 3
#define MENU_OPTION_4 4
#define MENU_OPTION_5 5
#define MENU_OPTION_6 6
#define MENU_OPTION_7 7
#define MENU_OPTION_8 8
#define MENU_OPTION_9 9
#define MENU_OPTION_10 10

/* 全局停车场管理对象 - 仅在UI层使用 */
static ParkingLot *ui_parking_lot = NULL;

/* ===== 主程序入口 ===== */

/* 运行停车管理系统主程序 */
void ui_run_parking_system(void) {
  int choice;
  int running = 1;

  /* 设置控制台编码以支持中文显示 */
  ui_setup_console_encoding();

  /* 显示系统标题 */
  ui_show_system_title();

  /* 初始化停车管理系统 */
  ui_initialize_parking_system();

  /* 主循环 */
  while (running) {
    ui_show_main_menu();
    printf("请选择操作 (0-10): ");

    if (ui_safe_read_int(&choice) != 0) {
      ui_show_error("输入错误，请输入数字！");
      continue;
    }

    if (choice == 0) {
      running = 0;
      ui_cleanup_and_exit();
    } else {
      ui_handle_menu_choice(choice);
    }

    if (running) {
      ui_wait_for_continue();
    }
  }
}

/* 初始化停车管理系统 */
void ui_initialize_parking_system(void) {
  if (ui_parking_lot == NULL) {
    ui_parking_lot = init_parking_lot(DEFAULT_PARKING_CAPACITY);
    if (ui_parking_lot == NULL) {
      ui_show_error("系统初始化失败！");
      exit(EXIT_FAILURE);
    }
    printf("停车管理系统初始化成功，总车位数: %d\n\n",
           ui_parking_lot->total_slots);
  }
}

/* 清理资源并退出系统 */
void ui_cleanup_and_exit(void) {
  printf("\n正在保存数据并退出系统...\n");

  if (ui_parking_lot) {
    /* 自动保存数据 */
    save_parking_data(ui_parking_lot, "parking_data_backup.txt");
    free_parking_lot(ui_parking_lot);
    ui_parking_lot = NULL;
  }

  printf("感谢使用社区停车管理系统！\n");
}

/* ===== 基础UI功能函数 ===== */

/* 清屏函数 - 跨平台实现 */
void ui_clear_screen(void) {
#ifdef _WIN32
  /* Windows系统使用system调用 */
  system("cls");
#else
  /* Unix/Linux系统使用ANSI转义序列 */
  printf("\033[2J\033[H");
  fflush(stdout);
#endif
}

/* 设置控制台编码以支持中文显示 */
void ui_setup_console_encoding(void) {
#ifdef _WIN32
  /* 设置控制台代码页为UTF-8 */
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
  setlocale(LC_ALL, "zh_CN.UTF-8");

  /* 备用方案：如果UTF-8不支持，使用GBK */
  if (SetConsoleOutputCP(CP_UTF8) == 0) {
    SetConsoleOutputCP(GBK_CODE_PAGE); /* GBK */
    SetConsoleCP(GBK_CODE_PAGE);
    setlocale(LC_ALL, "zh_CN.GBK");
  }
#else
  setlocale(LC_ALL, "zh_CN.UTF-8");
#endif
}

/* 清除输入缓冲区 */
void ui_clear_input_buffer(void) {
  while (getchar() != '\n') {
    /* 清除输入缓冲区 */
  }
}

/* 等待用户按键继续，然后清屏 */
void ui_wait_for_continue(void) {
  printf("\n按Enter键继续...");
  getchar();
  ui_clear_screen(); /* 添加清屏功能 */
}

/* 安全的整数输入函数 */
int ui_safe_read_int(int *value) {
  if (scanf("%d", value) != 1) {
    ui_clear_input_buffer();
    return -1;
  }
  ui_clear_input_buffer();
  return 0;
}

/* 安全的字符串输入函数 */
int ui_safe_read_string(char *buffer, size_t buffer_size) {
  if (fgets(buffer, (int)buffer_size, stdin) == NULL) {
    return -1;
  }

  /* 移除换行符 */
  buffer[strcspn(buffer, "\n")] = '\0';
  return 0;
}

/* ===== 主菜单相关函数 ===== */

/* 显示主菜单 */
void ui_show_main_menu(void) {
  printf("\n========== 主菜单 ==========\n");
  printf("1. 添加停车位\n");
  printf("2. 车辆入场（分配车位）\n");
  printf("3. 车辆出场（释放车位）\n");
  printf("4. 查询车位信息\n");
  printf("5. 显示车位列表\n");
  printf("6. 缴费管理\n");
  printf("7. 统计信息\n");
  printf("8. 保存数据\n");
  printf("9. 加载数据\n");
  printf("10. 运行演示程序\n");
  printf("0. 退出系统\n");
  printf("==========================\n");
}

/* 处理菜单选择 */
void ui_handle_menu_choice(int choice) {
  switch (choice) {
  case MENU_OPTION_1:
    ui_add_parking_slot_menu();
    break;
  case MENU_OPTION_2:
    ui_allocate_slot_menu();
    break;
  case MENU_OPTION_3:
    ui_deallocate_slot_menu();
    break;
  case MENU_OPTION_4:
    ui_query_slot_menu();
    break;
  case MENU_OPTION_5:
    ui_list_slots_menu();
    break;
  case MENU_OPTION_6:
    ui_payment_menu();
    break;
  case MENU_OPTION_7:
    ui_statistics_menu();
    break;
  case MENU_OPTION_8:
    ui_save_data_menu();
    break;
  case MENU_OPTION_9:
    ui_load_data_menu();
    break;
  case MENU_OPTION_10:
    /* 运行演示程序功能 */
    ui_run_demo_program();
    break;
  default:
    ui_show_error("无效选择，请重新输入！");
    break;
  }
}

/* ===== 停车位管理菜单 ===== */

/* 添加停车位菜单 */
void ui_add_parking_slot_menu(void) {
  int slot_id;
  char location[MAX_LOCATION_LEN];
  ServiceResult result;

  printf("\n========== 添加停车位 ==========\n");

  printf("请输入车位编号: ");
  if (ui_safe_read_int(&slot_id) != 0) {
    ui_show_error("输入错误！");
    return;
  }

  printf("请输入车位位置描述: ");
  if (ui_safe_read_string(location, sizeof(location)) != 0) {
    ui_show_error("读取位置信息失败！");
    return;
  }

  /* 使用服务层添加车位 */
  result = parking_service_add_slot(ui_parking_lot, slot_id, location);

  if (parking_service_is_success(result)) {
    printf("车位 %d 添加成功！位置：%s\n", slot_id, location);
  } else {
    parking_service_print_error(result);
  }

  /* 清理服务结果 */
  parking_service_free_result(&result);
}

/* 车辆入场（分配车位）菜单 */
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

  printf("请选择停车类型:\n");
  printf("1. 居民车位\n");
  printf("2. 外来车位\n");
  printf("请选择 (1-2): ");

  if (ui_safe_read_int(&type_choice) != 0) {
    ui_show_error("输入错误！");
    return;
  }

  switch (type_choice) {
  case 1:
    parking_type = RESIDENT_TYPE;
    break;
  case 2:
    parking_type = VISITOR_TYPE;
    break;
  default:
    ui_show_error("无效的停车类型！");
    return;
  }

  /* 使用服务层分配车位 */
  result = parking_service_allocate_slot(ui_parking_lot, slot_id, owner_name,
                                         license_plate, contact, parking_type);

  if (parking_service_is_success(result)) {
    printf("车位分配成功！\n");
    printf("  车位编号: %d\n", slot_id);
    printf("  车主姓名: %s\n", owner_name);
    printf("  车牌号: %s\n", license_plate);
    printf("  联系方式: %s\n", contact);
    printf("  停车类型: %s\n",
           parking_type == RESIDENT_TYPE ? "居民车位" : "外来车位");
  } else {
    parking_service_print_error(result);
  }

  /* 清理服务结果 */
  parking_service_free_result(&result);
}

/* 车辆出场（释放车位）菜单 */
void ui_deallocate_slot_menu(void) {
  int slot_id;
  ServiceResult result;

  printf("\n========== 车辆出场 ==========\n");

  printf("请输入车位编号: ");
  if (ui_safe_read_int(&slot_id) != 0) {
    ui_show_error("输入错误！");
    return;
  }

  /* 使用服务层释放车位 */
  result = parking_service_deallocate_slot(ui_parking_lot, slot_id);

  if (parking_service_is_success(result)) {
    printf("车位 %d 释放成功！\n", slot_id);
  } else {
    parking_service_print_error(result);
  }

  /* 清理服务结果 */
  parking_service_free_result(&result);
}

/* 查询车位信息菜单 */
void ui_query_slot_menu(void) {
  int slot_id;
  ServiceResult result;
  ParkingSlot *found;

  printf("\n========== 查询车位信息 ==========\n");

  printf("请输入车位编号: ");
  if (ui_safe_read_int(&slot_id) != 0) {
    ui_show_error("输入错误！");
    return;
  }

  /* 使用服务层查询车位 */
  result = parking_service_find_slot_by_id(ui_parking_lot, slot_id);

  if (parking_service_is_success(result)) {
    found = (ParkingSlot *)result.data;
    ui_show_slot_status(found);
  } else {
    parking_service_print_error(result);
  }

  /* 清理服务结果 */
  parking_service_free_result(&result);
}

/* 显示车位列表菜单 */
void ui_list_slots_menu(void) {
  int choice;
  ServiceResult result;
  time_t now;

  printf("\n========== 车位列表 ==========\n");
  printf("1. 显示所有车位\n");
  printf("2. 显示可用车位\n");
  printf("3. 显示已占用车位\n");
  printf("请选择 (1-3): ");

  if (ui_safe_read_int(&choice) != 0) {
    ui_show_error("输入错误！");
    return;
  }

  time(&now);

  switch (choice) {
  case 1:
    /* 显示所有车位 */
    printf("\n=== 所有车位信息 ===\n");
    /* 遍历停车场的链表显示所有车位 */
    {
      ParkingSlot *current = ui_parking_lot->head;
      while (current != NULL) {
        ui_show_slot_status(current);
        ui_show_separator();
        current = current->next;
      }
    }
    break;
  case 2:
    /* 显示可用车位 */
    printf("\n=== 可用车位信息 ===\n");
    result = parking_service_get_free_slots(ui_parking_lot);
    if (parking_service_is_success(result)) {
      /* 遍历链表显示可用车位 */
      {
        ParkingSlot *current = ui_parking_lot->head;
        while (current != NULL) {
          if (current->status == FREE_STATUS) {
            ui_show_slot_status(current);
            ui_show_separator();
          }
          current = current->next;
        }
      }
    } else {
      parking_service_print_error(result);
    }
    parking_service_free_result(&result);
    break;
  case 3:
    /* 显示已占用车位 */
    printf("\n=== 已占用车位信息 ===\n");
    result = parking_service_get_occupied_slots(ui_parking_lot);
    if (parking_service_is_success(result)) {
      {
        ParkingSlot *current = ui_parking_lot->head;
        while (current != NULL) {
          if (current->status == OCCUPIED_STATUS) {
            ui_show_slot_status(current);
            if (current->entry_time > 0) {
              double hours =
                  difftime(now, current->entry_time) / SECONDS_PER_HOUR;
              printf("  停车时长: %.1f 小时\n", hours);
            }
            ui_show_separator();
          }
          current = current->next;
        }
      }
    } else {
      parking_service_print_error(result);
    }
    parking_service_free_result(&result);
    break;
  default:
    ui_show_error("无效选择！");
    break;
  }

  printf("总车位数: %d, 已占用: %d, 可用: %d\n", ui_parking_lot->total_slots,
         ui_parking_lot->occupied_slots,
         ui_parking_lot->total_slots - ui_parking_lot->occupied_slots);
}

/* ===== 支付和统计菜单 ===== */

/* 缴费管理菜单 */
void ui_payment_menu(void) {
  int slot_id;
  double amount;
  int days;
  ServiceResult result;

  printf("\n========== 缴费管理 ==========\n");

  printf("请输入车位编号: ");
  if (ui_safe_read_int(&slot_id) != 0) {
    ui_show_error("输入错误！");
    return;
  }

  printf("请输入缴费金额: ");
  if (scanf("%lf", &amount) != 1) {
    ui_show_error("输入错误！");
    ui_clear_input_buffer();
    return;
  }
  ui_clear_input_buffer();

  printf("请输入缴费天数: ");
  if (ui_safe_read_int(&days) != 0) {
    ui_show_error("输入错误！");
    return;
  }

  /* 使用服务层添加缴费记录 */
  result = parking_service_add_payment(ui_parking_lot, slot_id, amount, days);

  if (parking_service_is_success(result)) {
    printf("缴费记录添加成功！\n");
    printf("  车位编号: %d\n", slot_id);
    printf("  缴费金额: %.2f 元\n", amount);
    printf("  缴费天数: %d 天\n", days);
  } else {
    parking_service_print_error(result);
  }

  /* 清理服务结果 */
  parking_service_free_result(&result);
}

/* 统计信息菜单 */
void ui_statistics_menu(void) {
  ServiceResult result;
  ParkingStatistics *stats;

  printf("\n========== 统计信息 ==========\n");

  /* 使用服务层获取统计信息 */
  result = parking_service_get_statistics(ui_parking_lot);

  if (parking_service_is_success(result)) {
    stats = (ParkingStatistics *)result.data;

    printf("总车位数: %d\n", stats->total_slots);
    printf("已占用车位数: %d\n", stats->occupied_slots);
    printf("可用车位数: %d\n", stats->free_slots);
    printf("居民车辆数: %d\n", stats->resident_vehicles);
    printf("外来车辆数: %d\n", stats->visitor_vehicles);
    printf("使用率: %.1f%%\n", stats->occupancy_rate);
    printf("今日收入: %.2f 元\n", stats->today_revenue);
    printf("月收入: %.2f 元\n", stats->month_revenue);
  } else {
    parking_service_print_error(result);
  }

  /* 清理服务结果 */
  parking_service_free_result(&result);
}

/* ===== 数据管理菜单 ===== */

/* 保存数据菜单 */
void ui_save_data_menu(void) {
  char filename[FILENAME_MAX_LEN];
  ServiceResult result;

  printf("\n========== 保存数据 ==========\n");

  printf("请输入文件名 (默认: parking_data.txt): ");
  if (ui_safe_read_string(filename, sizeof(filename)) != 0) {
    ui_show_error("读取文件名失败！");
    return;
  }
  if (strlen(filename) == 0) {
    strncpy(filename, "parking_data.txt", sizeof(filename) - 1);
    filename[sizeof(filename) - 1] = '\0';
  }

  /* 使用服务层保存数据 */
  result = parking_service_save_data(ui_parking_lot, filename);

  if (parking_service_is_success(result)) {
    printf("数据保存成功！文件：%s\n", filename);
  } else {
    parking_service_print_error(result);
  }

  /* 清理服务结果 */
  parking_service_free_result(&result);
}

/* 加载数据菜单 */
void ui_load_data_menu(void) {
  char filename[FILENAME_MAX_LEN];
  ServiceResult result;
  ParkingLot *loaded_lot;

  printf("\n========== 加载数据 ==========\n");

  printf("请输入文件名 (默认: parking_data.txt): ");
  if (ui_safe_read_string(filename, sizeof(filename)) != 0) {
    ui_show_error("读取文件名失败！");
    return;
  }
  if (strlen(filename) == 0) {
    strncpy(filename, "parking_data.txt", sizeof(filename) - 1);
    filename[sizeof(filename) - 1] = '\0';
  }

  /* 使用服务层加载数据 */
  result = parking_service_load_data(filename);

  if (parking_service_is_success(result)) {
    loaded_lot = (ParkingLot *)result.data;

    /* 释放当前数据 */
    if (ui_parking_lot) {
      free_parking_lot(ui_parking_lot);
    }

    ui_parking_lot = loaded_lot;
    printf("数据加载成功！\n");
    printf("总车位数: %d\n", ui_parking_lot->total_slots);
    printf("已占用: %d\n", ui_parking_lot->occupied_slots);
  } else {
    parking_service_print_error(result);
  }

  /* 注意：这里不要释放result.data，因为它被赋值给了ui_parking_lot */
  result.data = NULL;
  parking_service_free_result(&result);
}

/* ===== 演示程序功能 ===== */

/* 运行内置演示程序 */
void ui_run_demo_program(void) {
  printf("\n========== 内置演示程序 ==========\n");
  printf("正在运行停车管理系统演示...\n\n");

  /* 添加几个演示车位 */
  printf("1. 添加演示车位:\n");
  {
    ServiceResult result;

    result = parking_service_add_slot(ui_parking_lot, 201, "演示区-201");
    if (parking_service_is_success(result)) {
      printf("   ✓ 车位201添加成功\n");
    }
    parking_service_free_result(&result);

    result = parking_service_add_slot(ui_parking_lot, 202, "演示区-202");
    if (parking_service_is_success(result)) {
      printf("   ✓ 车位202添加成功\n");
    }
    parking_service_free_result(&result);
  }

  /* 分配车位给演示用户 */
  printf("\n2. 分配演示车位:\n");
  {
    ServiceResult result;

    result =
        parking_service_allocate_slot(ui_parking_lot, 201, "演示用户",
                                      "京A12345", "13800000000", RESIDENT_TYPE);
    if (parking_service_is_success(result)) {
      printf("   ✓ 车位201分配给演示用户成功\n");
    } else {
      printf("   ✗ 分配失败: ");
      parking_service_print_error(result);
    }
    parking_service_free_result(&result);
  }

  /* 显示当前状态 */
  printf("\n3. 当前系统状态:\n");
  {
    ServiceResult result = parking_service_get_statistics(ui_parking_lot);
    if (parking_service_is_success(result)) {
      ParkingStatistics *stats = (ParkingStatistics *)result.data;
      printf("   总车位数: %d\n", stats->total_slots);
      printf("   已占用: %d\n", stats->occupied_slots);
      printf("   空闲车位: %d\n", stats->free_slots);
      printf("   占用率: %.1f%%\n", stats->occupancy_rate);
    }
    parking_service_free_result(&result);
  }

  printf("\n演示程序运行完成！\n");
  printf("您可以继续使用系统的其他功能。\n");
}

/* ===== 获取全局停车场对象 ===== */

/* 获取UI层管理的停车场对象指针 */
ParkingLot *ui_get_parking_lot(void) { return ui_parking_lot; }

/* ===== UI显示辅助函数 ===== */

/* 显示系统标题 */
void ui_show_system_title(void) {
  printf("\n");
  printf("====================================================\n");
  printf("               社区停车管理系统                     \n");
  printf("               Community Parking System            \n");
  printf("====================================================\n");
  printf("\n");
}

/* 显示车位状态信息 */
void ui_show_slot_status(const ParkingSlot *slot) {
  if (slot == NULL) {
    printf("车位信息为空\n");
    return;
  }

  printf("车位编号: %d\n", slot->slot_id);
  printf("位置: %s\n", slot->location);
  printf("状态: %s\n", slot->status == OCCUPIED_STATUS ? "已占用" : "空闲");

  if (slot->status == OCCUPIED_STATUS) {
    printf("车主姓名: %s\n", slot->owner_name);
    printf("车牌号: %s\n", slot->license_plate);
    printf("联系方式: %s\n", slot->contact);
    printf("停车类型: %s\n",
           slot->type == RESIDENT_TYPE ? "居民车位" : "外来车位");
    printf("入场时间: %s", ctime(&slot->entry_time));
  }
}

/* 显示错误信息 */
void ui_show_error(const char *message) {
  if (message != NULL) {
    printf("[错误] %s\n", message);
  }
}

/* 显示分隔线 */
void ui_show_separator(void) {
  printf("----------------------------------------------------\n");
}
