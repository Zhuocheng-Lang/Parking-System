/**
 * @file data_layer_demo.c
 * @brief 数据层功能演示程序
 * @details
 * 该程序独立演示了数据层 (parking_data) 的各项核心功能，
 * 包括数据结构的创建、增删改查、统计以及文件持久化。
 * 它不涉及任何服务层或UI层的逻辑，旨在提供一个纯粹的数据层API使用范例。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../src/parking_data.h"

#ifdef _WIN32
#include <locale.h>
#include <windows.h>
#endif

/* 函数声明 */
void setup_console_encoding(void);
void demo_basic_operations(void);
void demo_statistics(void);
void demo_persistence(void);

/**
 * @brief 主函数，按顺序执行所有数据层功能演示。
 * @return 程序退出码，0 表示成功。
 */
int main(void) {
  setup_console_encoding();

  printf("========== 数据层 (parking_data) 功能演示 ==========\n");

  demo_basic_operations();
  demo_statistics();
  demo_persistence();

  printf("====================================================\n");
  printf("数据层演示完成！\n");
  return 0;
}

/**
 * @brief 演示数据层的基本 CRUD (创建、读取、更新、删除) 操作。
 * @details
 * 该函数按顺序演示以下功能：
 * 1. 使用 `init_parking_lot` 初始化停车场。
 * 2. 使用 `create_parking_slot` 和 `add_parking_slot` 添加车位。
 * 3. 使用 `allocate_slot` 分配车位给车辆。
 * 4. 使用 `find_slot_by_license` 按车牌号查询车位。
 * 5. 使用 `get_free_slots` 获取所有空闲车位。
 * 6. 最后使用 `free_parking_lot` 释放所有内存。
 */
void demo_basic_operations(void) {
  ParkingLot *lot;
  ParkingSlot *slot;
  ParkingSlot *found;
  ParkingSlot **free_slots;
  int index;
  int free_count;

  printf("\n--- 1. 基本操作演示 ---\n");

  lot = init_parking_lot(10);
  printf("创建停车场，总车位数: %d\n", lot->total_slots);

  for (index = 1; index <= 5; index++) {
    char location[50];
    sprintf(location, "A区-%d", index);
    slot = create_parking_slot(index, location);
    add_parking_slot(lot, slot);
  }
  printf("添加了5个停车位\n");

  allocate_slot(lot, 1, "张三", "京A12345", "13800138000", RESIDENT_TYPE);
  allocate_slot(lot, 3, "李四", "沪B67890", "13900139000", VISITOR_TYPE);
  printf("分配了2个车位 (1个居民, 1个访客)\n");

  found = find_slot_by_license(lot, "A12345");
  if (found) {
    printf("通过车牌号A12345找到车位: 车位%d, 车主: %s\n", found->slot_id,
           found->owner_name);
  }

  free_slots = get_free_slots(lot, &free_count);
  printf("查询到空闲车位: %d个\n", free_count);
  if (free_slots) {
    free(free_slots);
  }

  printf("当前已占用车位: %d个\n", lot->occupied_slots);

  free_parking_lot(lot);
}

/**
 * @brief 演示数据层的统计功能。
 * @details
 * 该函数演示如何使用 `count_daily_parking` 函数来统计特定日期内，
 * 不同类型（居民、访客）车辆的入场数量。
 * 它会创建一个临时的停车场，并手动添加几条停车记录用于统计。
 */
void demo_statistics(void) {
  ParkingLot *lot;
  ParkingSlot *slot;
  time_t today;
  int resident_count;
  int visitor_count;

  printf("\n--- 2. 统计功能演示 ---\n");

  lot = init_parking_lot(20);

  slot = create_parking_slot(1, "A-1");
  add_parking_slot(lot, slot);
  allocate_slot(lot, 1, "居民A", "R001", "111", RESIDENT_TYPE);

  slot = create_parking_slot(2, "A-2");
  add_parking_slot(lot, slot);
  allocate_slot(lot, 2, "居民B", "R002", "222", RESIDENT_TYPE);

  slot = create_parking_slot(3, "B-1");
  add_parking_slot(lot, slot);
  /* 手动设置访客数据以绕过服务层的时间检查 */
  slot->status = OCCUPIED_STATUS;
  slot->type = VISITOR_TYPE;
  strcpy(slot->owner_name, "访客C");
  strcpy(slot->license_plate, "V001");
  slot->entry_time = time(NULL);
  lot->occupied_slots++;

  today = time(NULL);
  resident_count = count_daily_parking(lot, today, RESIDENT_TYPE);
  visitor_count = count_daily_parking(lot, today, VISITOR_TYPE);

  printf("今日停车统计:\n");
  printf("- 居民车辆: %d\n", resident_count);
  printf("- 访客车辆: %d\n", visitor_count);
  printf("- 剩余空闲车位: %d\n", lot->total_slots - lot->occupied_slots);

  free_parking_lot(lot);
}

/**
 * @brief 演示数据层的持久化功能。
 * @details
 * 该函数演示了以下数据持久化流程：
 * 1. 创建一个停车场并添加一些数据。
 * 2. 使用 `save_parking_data` 将停车场状态保存到文件中。
 * 3. 使用 `load_parking_data` 从文件中加载数据到一个新的停车场对象。
 * 4. 验证加载后的数据是否与保存前一致。
 * 5. 清理创建的临时文件和内存。
 */
void demo_persistence(void) {
  ParkingLot *lot;
  ParkingLot *loaded_lot;
  const char *filename = "demo_data_layer.txt";

  printf("\n--- 3. 数据持久化演示 ---\n");

  lot = init_parking_lot(5);
  add_parking_slot(lot, create_parking_slot(101, "P-101"));
  allocate_slot(lot, 101, "持久化用户", "P-SAVE", "888", RESIDENT_TYPE);
  printf("创建了一个包含1个已占用车位的停车场\n");

  if (save_parking_data(lot, filename) == 0) {
    printf("数据已成功保存到 %s\n", filename);
  }

  loaded_lot = load_parking_data(filename);
  if (loaded_lot) {
    printf("从文件加载数据成功:\n");
    printf("- 总车位数: %d\n", loaded_lot->total_slots);
    printf("- 已占用车位数: %d\n", loaded_lot->occupied_slots);
    ParkingSlot *found = find_slot_by_id(loaded_lot, 101);
    if (found) {
      printf("- 验证车位101数据: 车主 '%s'\n", found->owner_name);
    }
    free_parking_lot(loaded_lot);
  }

  free_parking_lot(lot);
  remove(filename);
}

/**
 * @brief 设置控制台编码以支持中文显示。
 * @details
 * 在 Windows 系统上，尝试将控制台的输入输出代码页设置为 UTF-8，
 * 以便正确显示中文字符。如果失败，则回退到 GBK (936)。
 * 在非 Windows 系统上，使用 setlocale 设置为 UTF-8 环境。
 */
void setup_console_encoding(void) {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
  setlocale(LC_ALL, "zh_CN.UTF-8");
  if (SetConsoleOutputCP(CP_UTF8) == 0) {
    SetConsoleOutputCP(936);
    SetConsoleCP(936);
    setlocale(LC_ALL, "zh_CN.GBK");
  }
#else
  setlocale(LC_ALL, "zh_CN.UTF-8");
#endif
}
