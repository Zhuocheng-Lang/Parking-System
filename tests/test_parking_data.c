/**
 * @file test_parking_data.c
 * @brief 使用 cmocka 框架的数据层单元测试文件
 * @details
 * 该文件利用 cmocka 测试框架对数据层 (parking_data)
 * 的所有核心功能进行单元测试。 每个测试用例都被构造成一个独立的测试函数，使用
 * cmocka 的断言宏来验证结果。
 */

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/parking_data.h"
#include "cmocka.h"

/* ========================================================================== */
/*                                 测试用例实现                               */
/* ========================================================================== */

/**
 * @brief 测试 `init_parking_lot` 函数的功能。
 * @details 验证停车场初始化后，总车位数、已占用车位数和链表头指针是否正确。
 * @param state cmocka 框架的测试状态指针。
 */
static void test_init_parking_lot(void **state) {
  (void)state; /* not used */
  ParkingLot *lot = init_parking_lot(100);
  assert_non_null(lot);
  assert_int_equal(lot->total_slots, 100);
  assert_int_equal(lot->occupied_slots, 0);
  assert_null(lot->slot_head);
  free_parking_lot(lot);
}

/**
 * @brief 测试 `create_parking_slot` 和 `add_parking_slot` 函数的功能。
 * @details
 * 验证车位能否被成功创建和添加到停车场，并测试添加重复ID车位时的错误处理。
 * @param state cmocka 框架的测试状态指针。
 */
static void test_create_and_add_slot(void **state) {
  (void)state; /* not used */
  ParkingLot *lot = init_parking_lot(10);
  ParkingSlot *slot1 = create_parking_slot(1, "A-1");
  assert_non_null(slot1);

  int add_result = add_parking_slot(lot, slot1);
  assert_int_equal(add_result, 0);
  assert_ptr_equal(lot->slot_head, slot1);

  ParkingSlot *slot_dup = create_parking_slot(1, "A-2");
  int dup_result = add_parking_slot(lot, slot_dup);
  assert_int_equal(dup_result, -2); /* -2: ID已存在 */

  free_parking_slot(slot_dup);
  free_parking_lot(lot);
}

/**
 * @brief 测试 `allocate_slot` 和 `deallocate_slot` 函数的功能。
 * @details
 * 验证车位的分配和释放操作是否能正确执行，并检查已占用车位数和车位状态是否相应更新。
 * @param state cmocka 框架的测试状态指针。
 */
static void test_allocate_and_deallocate_slot(void **state) {
  (void)state; /* not used */
  ParkingLot *lot = init_parking_lot(10);
  add_parking_slot(lot, create_parking_slot(1, "A-1"));
  add_parking_slot(lot, create_parking_slot(2, "A-2"));

  int alloc_result = allocate_slot(lot, 1, "Zhang San", "沪A12321",
                                   "12312341234", RESIDENT_TYPE);
  assert_int_equal(alloc_result, 0);
  assert_int_equal(lot->occupied_slots, 1);

  int dealloc_result = deallocate_slot(lot, 1);
  assert_int_equal(dealloc_result, 0);
  assert_int_equal(lot->occupied_slots, 0);

  ParkingSlot *found = find_slot_by_id(lot, 1);
  assert_non_null(found);
  assert_int_equal(found->status, FREE_STATUS);

  free_parking_lot(lot);
}

/**
 * @brief 测试 `find_slot_by_id` 和 `find_slot_by_license` 函数的功能。
 * @details
 * 验证能否根据车位ID和车牌号准确地找到对应的车位，并测试查找不存在车位的情况。
 * @param state cmocka 框架的测试状态指针。
 */
static void test_find_functions(void **state) {
  (void)state; /* not used */
  ParkingLot *lot = init_parking_lot(10);
  add_parking_slot(lot, create_parking_slot(1, "A-1"));
  allocate_slot(lot, 1, "Zhang San", "沪A12321", "12312341234", RESIDENT_TYPE);

  assert_non_null(find_slot_by_id(lot, 1));
  assert_null(find_slot_by_id(lot, 99));
  assert_non_null(find_slot_by_license(lot, "沪A12321"));
  assert_null(find_slot_by_license(lot, "京Z99999"));

  free_parking_lot(lot);
}

/**
 * @brief 测试 `save_parking_data` 和 `load_parking_data` 的数据持久化功能。
 * @details
 * 验证停车场数据能否被成功保存到文件，并能从文件正确加载回来，同时保持数据的一致性。
 * @param state cmocka 框架的测试状态指针。
 */
static void test_data_persistence(void **state) {
  (void)state; /* not used */
  const char *test_file = "persistence_test.txt";
  ParkingLot *lot_to_save = init_parking_lot(5);
  add_parking_slot(lot_to_save, create_parking_slot(1, "P-1"));
  add_parking_slot(lot_to_save, create_parking_slot(2, "P-2")); // 一个空闲车位
  allocate_slot(lot_to_save, 1, "PersistentUser", "沪A12123", "12312341234",
                RESIDENT_TYPE);

  /* 1. 测试保存 */
  int save_result = save_parking_data(lot_to_save, test_file);
  assert_int_equal(save_result, 0);

  /* 2. 测试加载 */
  ParkingLot *loaded_lot = load_parking_data(test_file);
  assert_non_null(loaded_lot);

  /* 3. 验证加载后的数据 */
  if (loaded_lot) {
    assert_int_equal(loaded_lot->total_slots, 5);
    assert_int_equal(loaded_lot->occupied_slots, 1);

    ParkingSlot *loaded_slot_1 = find_slot_by_id(loaded_lot, 1);
    assert_non_null(loaded_slot_1);
    assert_int_equal(loaded_slot_1->status, OCCUPIED_STATUS);
    assert_string_equal(loaded_slot_1->owner_name, "PersistentUser");

    ParkingSlot *loaded_slot_2 = find_slot_by_id(loaded_lot, 2);
    assert_non_null(loaded_slot_2);
    assert_int_equal(loaded_slot_2->status, FREE_STATUS);
  }

  free_parking_lot(lot_to_save);
  if (loaded_lot) {
    free_parking_lot(loaded_lot);
  }
  remove(test_file);
}

/* ========================================================================== */
/*                                 主测试函数                                 */
/* ========================================================================== */

/**
 * @brief 测试程序的主入口。
 * @details 注册所有单元测试用例并使用 cmocka 框架运行它们。
 * @return 返回 cmocka 测试组的执行结果。
 */
int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_init_parking_lot),
      cmocka_unit_test(test_create_and_add_slot),
      cmocka_unit_test(test_allocate_and_deallocate_slot),
      cmocka_unit_test(test_find_functions),
      cmocka_unit_test(test_data_persistence),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
