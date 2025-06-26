/**
 * @file test_parking_service.c
 * @brief 使用 cmocka 框架的服务层单元测试文件
 * @details
 * 该文件利用 cmocka 测试框架对服务层 (parking_service)
 * 的所有核心业务逻辑进行单元测试。每个测试用例都验证一个特定的服务函数，
 * 检查其返回值、错误处理和对停车场状态的正确影响。
 */

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../src/parking_service.h"
#include "cmocka.h"

/* ========================================================================== */
/*                                 测试固件设置                               */
/* ========================================================================== */

/**
 * @brief 测试前置函数，为每个测试用例创建一个新的停车场实例。
 * @details
 * 此函数在每个测试用例运行前被调用。它初始化一个包含10个车位的停车场，
 * 并将其指针存储在 cmocka 的 state 对象中，供测试用例使用。
 * @param state cmocka 框架的测试状态指针的指针。
 * @return 成功返回 0，失败返回 -1。
 */
static int setup(void **state) {
  ParkingLot *lot = init_parking_lot(10);
  if (!lot) {
    return -1;
  }
  *state = lot;
  return 0;
}

/**
 * @brief 测试后置函数，释放由 setup 创建的停车场实例。
 * @details
 * 此函数在每个测试用例运行结束后被调用。它从 state 对象中获取停车场指针，
 * 并调用 free_parking_lot 来释放所有相关内存，防止内存泄漏。
 * @param state cmocka 框架的测试状态指针的指针。
 * @return 成功返回 0。
 */
static int teardown(void **state) {
  if (*state) {
    free_parking_lot((ParkingLot *)*state);
  }
  return 0;
}

/* ========================================================================== */
/*                                 测试用例实现                               */
/* ========================================================================== */

/**
 * @brief 测试 `parking_service_add_slot` 函数的功能。
 * @details
 * 验证服务层添加车位的功能，包括：
 * 1. 成功添加一个新车位。
 * 2. 尝试添加一个已存在的车位ID，预期会失败。
 * 3. 使用无效参数（如NULL位置）调用，预期会失败。
 * @param state cmocka 框架的测试状态指针。
 */
static void test_service_add_slot(void **state) {
  ParkingLot *lot = (ParkingLot *)*state;
  ServiceResult result;

  /* 1. 测试成功添加 */
  result = parking_service_add_slot(lot, 1, "A-1");
  assert_int_equal(result.code, PARKING_SERVICE_SUCCESS);
  assert_non_null(find_slot_by_id(lot, 1));

  /* 2. 测试添加重复ID */
  result = parking_service_add_slot(lot, 1, "A-2");
  assert_int_equal(result.code, PARKING_SERVICE_SLOT_EXISTS);

  /* 3. 测试无效参数 */
  result = parking_service_add_slot(lot, 2, NULL);
  assert_int_equal(result.code, PARKING_SERVICE_INVALID_PARAM);
}

/**
 * @brief 测试 `parking_service_allocate_slot` 和
 * `parking_service_deallocate_slot` 的功能。
 * @details
 * 验证车辆入场和出场的完整业务流程：
 * 1. 成功为一个空闲车位分配车辆。
 * 2. 验证分配后车位状态变为已占用。
 * 3. 车辆出场并计算费用。
 * 4. 验证出场后车位状态恢复为空闲。
 * 5. 测试对不存在或未占用的车位执行出场操作的错误处理。
 * @param state cmocka 框架的测试状态指针。
 */
static void test_service_allocate_and_deallocate_slot(void **state) {
  ParkingLot *lot = (ParkingLot *)*state;
  parking_service_add_slot(lot, 101, "B-101");
  ServiceResult result;

  /* 1. 成功分配车辆 */
  result = parking_service_allocate_slot(lot, 101, "TestUser", "粤B12345",
                                         "13800138000", VISITOR_TYPE);
  assert_int_equal(result.code, PARKING_SERVICE_SUCCESS);
  assert_int_equal(lot->occupied_slots, 1);

  /* 2. 模拟一段时间后出场 */
  ParkingSlot *slot = find_slot_by_id(lot, 101);
  assert_non_null(slot);
  slot->entry_time = time(NULL) - 3600; /* 模拟停车1小时 */

  result = parking_service_deallocate_slot(lot, 101);
  assert_int_equal(result.code, PARKING_SERVICE_SUCCESS);
  assert_non_null(result.data); /* 应该有费用返回 */
  assert_true(*(double *)result.data > 0);
  parking_service_free_result(&result);

  assert_int_equal(lot->occupied_slots, 0);

  /* 3. 测试对空闲车位执行出场 */
  result = parking_service_deallocate_slot(lot, 101);
  assert_int_equal(result.code, PARKING_SERVICE_SLOT_FREE);
}

/**
 * @brief 测试 `parking_service_get_statistics` 函数的功能。
 * @details
 * 验证统计信息服务是否能返回正确的数据。
 * 1. 在空停车场中获取统计信息。
 * 2. 分配一个车位后，再次获取统计信息并验证占用数和使用率是否更新。
 * @param state cmocka 框架的测试状态指针。
 */
static void test_service_get_statistics(void **state) {
  ParkingLot *lot = (ParkingLot *)*state;
  ServiceResult result;
  ParkingStatistics *stats;

  /* 1. 空停车场统计 */
  result = parking_service_get_statistics(lot);
  assert_int_equal(result.code, PARKING_SERVICE_SUCCESS);
  stats = (ParkingStatistics *)result.data;
  assert_non_null(stats);
  assert_int_equal(stats->total_slots, 10);
  assert_int_equal(stats->occupied_slots, 0);
  parking_service_free_result(&result);

  /* 2. 分配一个车位后统计 */
  parking_service_add_slot(lot, 1, "C-1");
  parking_service_allocate_slot(lot, 1, "StatUser", "粤BSTAT1", "13912391239",
                                RESIDENT_TYPE);

  result = parking_service_get_statistics(lot);
  assert_int_equal(result.code, PARKING_SERVICE_SUCCESS);
  stats = (ParkingStatistics *)result.data;
  assert_non_null(stats);
  assert_int_equal(stats->occupied_slots, 1);
  assert_float_equal(stats->occupancy_rate, 10.0, 0.01);
  parking_service_free_result(&result);
}

/**
 * @brief 测试 `parking_service_save_data` 和 `parking_service_load_data`
 * 的功能。
 * @details
 * 验证服务层的数据持久化功能。
 * 1. 创建并填充一个停车场。
 * 2. 调用保存服务将数据写入文件。
 * 3. 调用加载服务从文件恢复数据到一个新实例。
 * 4. 验证加载后的数据与保存前一致。
 * @param state cmocka 框架的测试状态指针。
 */
static void test_service_data_persistence(void **state) {
  const char *test_file = "service_persistence_test.txt";
  ParkingLot *lot_to_save = init_parking_lot(5);
  parking_service_add_slot(lot_to_save, 1, "P-1");
  parking_service_allocate_slot(lot_to_save, 1, "SvcUser", "沪ASVC01",
                                "12312341234", RESIDENT_TYPE);

  /* 1. 测试保存 */
  ServiceResult save_result = parking_service_save_data(lot_to_save, test_file);
  assert_int_equal(save_result.code, PARKING_SERVICE_SUCCESS);

  /* 2. 测试加载 */
  ServiceResult load_result = parking_service_load_data(test_file);
  assert_int_equal(load_result.code, PARKING_SERVICE_SUCCESS);
  assert_non_null(load_result.data);
  ParkingLot *loaded_lot = (ParkingLot *)load_result.data;

  /* 3. 验证数据 */
  assert_int_equal(loaded_lot->total_slots, 5);
  assert_int_equal(loaded_lot->occupied_slots, 1);
  ParkingSlot *loaded_slot = find_slot_by_id(loaded_lot, 1);
  assert_non_null(loaded_slot);
  assert_string_equal(loaded_slot->owner_name, "SvcUser");

  /* 清理 */
  free_parking_lot(lot_to_save);
  free_parking_lot(loaded_lot);
  remove(test_file);
}

/* ========================================================================== */
/*                                 主测试函数                                 */
/* ========================================================================== */

/**
 * @brief 测试程序的主入口。
 * @details
 * 注册所有服务层的单元测试用例，并使用 cmocka 框架统一运行它们。
 * 每个测试组使用 setup 和 teardown 函数来管理测试环境的生命周期。
 * @return 返回 cmocka 测试组的执行结果。
 */
int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test_setup_teardown(test_service_add_slot, setup, teardown),
      cmocka_unit_test_setup_teardown(test_service_allocate_and_deallocate_slot,
                                      setup, teardown),
      cmocka_unit_test_setup_teardown(test_service_get_statistics, setup,
                                      teardown),
      cmocka_unit_test(test_service_data_persistence),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
