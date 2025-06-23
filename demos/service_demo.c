/*
 * 停车管理系统服务层演示程序
 * 展示服务层的各项功能
 */

#include "../src/parking_data.h"
#include "../src/parking_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <locale.h>
#include <windows.h>
#endif

/* 设置控制台编码以支持中文显示 */
void setup_console_encoding(void) {
#ifdef _WIN32
  /* 设置控制台代码页为UTF-8 */
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
  setlocale(LC_ALL, "zh_CN.UTF-8");

  /* 备用方案：如果UTF-8不支持，使用GBK */
  if (SetConsoleOutputCP(CP_UTF8) == 0) {
    SetConsoleOutputCP(936); /* GBK */
    SetConsoleCP(936);
    setlocale(LC_ALL, "zh_CN.GBK");
  }
#else
  setlocale(LC_ALL, "zh_CN.UTF-8");
#endif
}

/* 打印分隔线 */
void print_separator(const char *title) {
  printf("\n========== %s ==========\n", title);
}

/* 演示服务层功能 */
void demonstrate_service_layer(void) {
  ParkingLot *lot;
  ServiceResult result;
  ParkingStatistics *stats;

  print_separator("服务层功能演示");

  /* 初始化停车场 */
  lot = init_parking_lot(100);
  if (!lot) {
    printf("停车场初始化失败！\n");
    return;
  }
  printf("停车场初始化成功，当前车位数：%d\n", lot->total_slots);
  print_separator("1. 添加停车位");
  /* 添加几个停车位 */
  printf("开始添加车位101...\n");
  result = parking_service_add_slot(lot, 101, "A区-101");
  if (parking_service_is_success(result)) {
    printf("✓ 添加车位101成功\n");
  } else {
    printf("✗ 添加车位101失败：");
    parking_service_print_error(result);
  }
  parking_service_free_result(&result);

  printf("开始添加车位102...\n");
  result = parking_service_add_slot(lot, 102, "A区-102");
  if (parking_service_is_success(result)) {
    printf("✓ 添加车位102成功\n");
  } else {
    printf("✗ 添加车位102失败：");
    parking_service_print_error(result);
  }
  parking_service_free_result(&result);

  printf("开始添加车位103...\n");
  result = parking_service_add_slot(lot, 103, "A区-103");
  if (parking_service_is_success(result)) {
    printf("✓ 添加车位103成功\n");
  } else {
    printf("✗ 添加车位103失败：");
    parking_service_print_error(result);
  }
  parking_service_free_result(&result);

  print_separator("2. 分配停车位");
  /* 分配停车位给居民 */
  result = parking_service_allocate_slot(lot, 101, "张三", "A12345",
                                         "13800138000", RESIDENT_TYPE);
  if (parking_service_is_success(result)) {
    printf("✓ 车位101分配给居民张三成功\n");
  } else {
    parking_service_print_error(result);
  }
  parking_service_free_result(&result);

  /* 分配停车位给访客 */
  result = parking_service_allocate_slot(lot, 102, "李四", "B67890",
                                         "13900139000", VISITOR_TYPE);
  if (parking_service_is_success(result)) {
    printf("✓ 车位102分配给访客李四成功\n");
  } else {
    parking_service_print_error(result);
  }
  parking_service_free_result(&result);

  print_separator("3. 查询停车位"); /* 根据车位编号查询 */
  result = parking_service_find_slot_by_id(lot, 101);
  if (parking_service_is_success(result)) {
    ParkingSlot *slot = (ParkingSlot *)result.data;
    printf("✓ 查询车位101：\n");
    printf("  车主：%s\n", slot->owner_name);
    printf("  车牌：%s\n", slot->license_plate);
    printf("  类型：%s\n", (slot->type == RESIDENT_TYPE) ? "居民" : "访客");
  } else {
    parking_service_print_error(result);
  }
  /* 不释放查询结果的data，因为它指向停车场中的现有数据 */
  result.data = NULL;
  parking_service_free_result(&result);

  /* 根据车牌号查询 */
  result = parking_service_find_slot_by_license(lot, "A12345");
  if (parking_service_is_success(result)) {
    ParkingSlot *slot = (ParkingSlot *)result.data;
    printf("✓ 查询车牌A12345：车位%d\n", slot->slot_id);
  } else {
    parking_service_print_error(result);
  }
  /* 不释放查询结果的data */
  result.data = NULL;
  parking_service_free_result(&result);

  print_separator("4. 缴费管理");
  /* 为居民车位添加缴费记录 */
  result = parking_service_add_payment(lot, 101, 200.0, 30);
  if (parking_service_is_success(result)) {
    printf("✓ 车位101缴费记录添加成功（200元/30天）\n");
  } else {
    parking_service_print_error(result);
  }
  parking_service_free_result(&result);
  print_separator("5. 统计信息");
  /* 获取统计信息 */
  printf("正在获取统计信息...\n");
  result = parking_service_get_statistics(lot);
  if (parking_service_is_success(result)) {
    stats = (ParkingStatistics *)result.data;
    printf("✓ 停车场统计信息：\n");
    printf("  总车位数：%d\n", stats->total_slots);
    printf("  已占用：%d\n", stats->occupied_slots);
    printf("  空闲车位：%d\n", stats->free_slots);
    printf("  占用率：%.1f%%\n", stats->occupancy_rate);
    printf("  本月收费：%.2f元\n", stats->month_revenue);
  } else {
    printf("✗ 获取统计信息失败：");
    parking_service_print_error(result);
  }
  parking_service_free_result(&result);
  print_separator("6. 数据持久化");
  /* 保存数据 */
  printf("正在保存数据到 service_demo_data.txt ...\n");
  result = parking_service_save_data(lot, "service_demo_data.txt");
  if (parking_service_is_success(result)) {
    printf("✓ 数据保存成功\n");
  } else {
    printf("✗ 数据保存失败：");
    parking_service_print_error(result);
  }
  parking_service_free_result(&result);
  print_separator("7. 释放停车位");
  /* 先为车位102分配一个访客（不受时间限制的测试） */
  result = parking_service_allocate_slot(lot, 102, "访客李四", "B67890",
                                         "13900139000", VISITOR_TYPE);
  if (parking_service_is_success(result)) {
    printf("✓ 车位102分配给访客李四成功（测试用）\n");
  } else {
    printf("✗ 访客分配失败，跳过释放测试\n");
    parking_service_print_error(result);
    parking_service_free_result(&result);
    goto skip_deallocate;
  }
  parking_service_free_result(&result);

  /* 释放访客车位（会产生费用） */
  result = parking_service_deallocate_slot(lot, 102);
  if (parking_service_is_success(result)) {
    double *fee = (double *)result.data;
    printf("✓ 车位102释放成功\n");
    if (fee && *fee > 0) {
      printf("  停车费用：%.2f元\n", *fee);
    }
  } else {
    printf("✗ 释放车位失败：");
    parking_service_print_error(result);
  }
  parking_service_free_result(&result);

skip_deallocate:

  print_separator("演示完成");
  printf("服务层功能演示成功完成！\n\n");

  /* 清理资源 */
  if (lot) {
    free_parking_lot(lot);
  }
}

int main(void) {
  /* 设置控制台编码 */
  setup_console_encoding();

  printf("=========================================\n");
  printf("    停车管理系统服务层演示 v1.0\n");
  printf("=========================================\n");

  /* 运行服务层演示 */
  demonstrate_service_layer();

  printf("演示程序执行完毕。\n");

  return 0;
}
