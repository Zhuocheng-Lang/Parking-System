/**
 * @file service_demo.c
 * @brief 服务层功能演示程序
 * @details
 * 该程序独立演示了服务层 (parking_service) 的各项核心功能。
 * 它展示了如何调用服务层API来执行完整的业务流程，
 * 如添加车位、车辆入场、查询、统计、计费出场和数据持久化，
 * 并演示了如何正确处理返回的 ServiceResult 对象。
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../src/parking_service.h"

/**< 定义每月的秒数（假设30天为一个月），用于模拟计费周期。 */
#define SECONDS_PER_MONTH (30 * 24 * 60 * 60)

#ifdef _WIN32
#include <locale.h>
#include <windows.h>
#endif

void setup_console_encoding(void);
void print_separator(const char *title);
void demonstrate_service_layer(void);

/**
 * @brief 主函数，运行服务层功能演示。
 * @details
 * 程序入口点。首先调用 `setup_console_encoding()` 确保控制台能正确显示中文，
 * 然后调用 `demonstrate_service_layer()` 执行核心的演示逻辑。
 * @return 程序执行成功返回 0。
 */
int main(void) {
  setup_console_encoding();

  printf("========== 服务层 (parking_service) 功能演示 ==========\n");
  demonstrate_service_layer();
  printf("\n====================================================\n");
  printf("服务层演示完成！\n");

  return 0;
}

/**
 * @brief 演示服务层的主要业务流程。
 * @details
 * 该函数按顺序执行以下操作来展示服务层的功能：
 * 1. 初始化停车场 (`init_parking_lot`)。
 * 2. 添加几个停车位 (`parking_service_add_slot`)。
 * 3. 为居民和访客分配车位 (`parking_service_allocate_slot`)。
 * 4. 查询并打印停车场统计信息 (`parking_service_get_statistics`)。
 * 5. 模拟车辆出场并计算费用
 * (`parking_service_deallocate_slot`)，包括居民欠费和访客正常计费两种情况。
 * 6. 将最终的停车场数据保存到文件 (`parking_service_save_data`)。
 * 7. 清理所有分配的资源。
 */
void demonstrate_service_layer(void) {
  ParkingLot *lot;
  ServiceResult result;

  lot = init_parking_lot(10);
  if (!lot) {
    printf("停车场初始化失败！\n");
    return;
  }

  print_separator("1. 添加车位");
  result = parking_service_add_slot(lot, 101, "A区-101");
  if (parking_service_is_success(result)) {
    printf("✓ 添加车位101成功\n");
  } else {
    parking_service_print_error(result);
  }

  result = parking_service_add_slot(lot, 102, "A区-102");
  if (parking_service_is_success(result)) {
    printf("✓ 添加车位102成功\n");
  } else {
    parking_service_print_error(result);
  }

  print_separator("2. 车辆入场");
  result = parking_service_allocate_slot(lot, 101, "居民张三", "J-RES01",
                                         "138...", RESIDENT_TYPE);
  if (parking_service_is_success(result)) {
    printf("✓ 车位101分配给居民成功\n");
  } else {
    parking_service_print_error(result);
  }

  result = parking_service_allocate_slot(lot, 102, "访客李四", "F-VIS02",
                                         "139...", VISITOR_TYPE);
  if (parking_service_is_success(result)) {
    printf("✓ 车位102分配给访客成功\n");
  } else {
    parking_service_print_error(result);
  }

  print_separator("3. 查询与统计");
  result = parking_service_get_statistics(lot);
  if (parking_service_is_success(result)) {
    ParkingStatistics *stats = (ParkingStatistics *)result.data;
    printf("✓ 获取统计信息成功: 总车位 %d, 已占用 %d, 使用率 %.2f%%\n",
           stats->total_slots, stats->occupied_slots, stats->occupancy_rate);
    parking_service_free_result(&result); /* 释放stats内存 */
  } else {
    parking_service_print_error(result);
  }

  print_separator("4. 车辆出场与计费");
  /* 模拟居民车辆出场（通常无费用，除非欠费）*/
  ParkingSlot *resident_slot = find_slot_by_id(lot, 101);
  if (resident_slot) {
    /* 模拟欠费1个月 */
    resident_slot->resident_due_date = time(NULL) - (SECONDS_PER_MONTH + 1);
  }
  result = parking_service_deallocate_slot(lot, 101);
  if (parking_service_is_success(result)) {
    double fee = result.data ? *((double *)result.data) : 0.0;
    printf("✓ 居民车辆出场成功。补缴费用: %.2f元\n", fee);
    parking_service_free_result(&result);
  } else {
    parking_service_print_error(result);
  }

  /* 模拟访客车辆停车2.5小时后出场 */
  ParkingSlot *visitor_slot = find_slot_by_id(lot, 102);
  if (visitor_slot) {
    visitor_slot->entry_time = time(NULL) - (time_t)(2.5 * 3600);
  }
  result = parking_service_deallocate_slot(lot, 102);
  if (parking_service_is_success(result)) {
    double fee = result.data ? *((double *)result.data) : 0.0;
    printf("✓ 访客车辆出场成功。停车费用: %.2f元\n", fee);
    parking_service_free_result(&result);
  } else {
    parking_service_print_error(result);
  }

  print_separator("5. 数据持久化");
  result = parking_service_save_data(lot, "service_demo_data.txt");
  if (parking_service_is_success(result)) {
    printf("✓ 数据保存到 service_demo_data.txt 成功\n");
  } else {
    parking_service_print_error(result);
  }

  free_parking_lot(lot);
  remove("service_demo_data.txt");
}

/**
 * @brief 打印格式化的分隔线。
 * @details
 * 在控制台输出一个带有标题的分隔线，用于区分演示中的不同部分，使输出更清晰。
 * @param title 要在分隔线中显示的标题文本。
 */
void print_separator(const char *title) {
  printf("\n========== %s ==========\n", title);
}

/**
 * @brief 设置控制台编码以支持中文显示。
 * @details
 * 这是一个跨平台的辅助函数。
 * - 在 Windows (`_WIN32`) 系统上，尝试将控制台的输入和输出代码页设置为 UTF-8。
 *   如果失败，则回退到 GBK (代码页 936)。
 * - 在其他系统上，使用 `setlocale` 将区域设置为支持 UTF-8 的中文环境。
 * 这确保了程序输出的中文字符能够被正确渲染。
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
