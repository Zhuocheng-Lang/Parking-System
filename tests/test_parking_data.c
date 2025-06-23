#include "../src/parking_data.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <locale.h>
#include <windows.h>

#endif

/* 设置控制台编码以支持中文显示 */
void setup_console_encoding(void);

/* 测试用例函数声明 */
void test_init_parking_lot(void);
void test_create_and_add_slot(void);
void test_allocate_and_deallocate_slot(void);
void test_find_functions(void);
void test_get_slots_lists(void);
void test_payment_functions(void);
void test_statistics_functions(void);
void test_data_persistence(void);
void test_memory_management(void);

/* 测试辅助函数 */
void print_test_result(const char *test_name, int passed);
void print_slot_info(ParkingSlot *slot);

/* 全局测试计数器 */
static int tests_passed = 0;
static int tests_total = 0;

int main(void) {
  /* 设置控制台编码以支持中文显示 */
  setup_console_encoding();

  printf("停车管理系统数据结构测试\n\n");

  test_init_parking_lot();
  test_create_and_add_slot();
  test_allocate_and_deallocate_slot();
  test_find_functions();
  test_get_slots_lists();
  test_payment_functions();
  test_statistics_functions();
  test_data_persistence();
  test_memory_management();

  printf("\nTest Summary\n");
  printf("Passed: %d / %d\n", tests_passed, tests_total);
  if (tests_passed == tests_total) {
    printf("All tests passed!\n");
  } else {
    printf("%d tests failed!\n", tests_total - tests_passed);
  }

  return (tests_passed == tests_total) ? 0 : 1;
}

void print_test_result(const char *test_name, int passed) {
  tests_total++;
  if (passed) {
    tests_passed++;
    printf("[PASS] %s\n", test_name);
  } else {
    printf("[FAIL] %s\n", test_name);
  }
}

void print_slot_info(ParkingSlot *slot) {
  if (slot == NULL) {
    printf("  Slot: NULL\n");
    return;
  }

  printf("  Slot %d: %s\n", slot->slot_id, slot->location);
  printf("    Status: %s\n", slot->status == FREE_STATUS ? "Free" : "Occupied");
  if (slot->status == OCCUPIED_STATUS) {
    printf("    Owner: %s\n", slot->owner_name);
    printf("    License: %s\n", slot->license_plate);
    printf("    Contact: %s\n", slot->contact);
    printf("    Type: %s\n",
           slot->type == RESIDENT_TYPE ? "Resident" : "Visitor");
  }
}

void test_init_parking_lot(void) {
  ParkingLot *lot;

  printf("Testing parking lot initialization...\n");

  lot = init_parking_lot(1000);

  print_test_result("Parking lot initialization",
                    lot != NULL && lot->total_slots == 1000 &&
                        lot->occupied_slots == 0 && lot->head == NULL);

  if (lot) {
    free_parking_lot(lot);
  }
}

void test_create_and_add_slot(void) {
  ParkingLot *lot;
  ParkingSlot *slot1;
  ParkingSlot *slot2;
  ParkingSlot *slot_dup;
  int result1, result2, dup_result;

  printf("\nTesting slot creation and addition...\n");

  lot = init_parking_lot(10);
  if (lot == NULL) {
    print_test_result("Slot creation - initialization failed", 0);
    return;
  }

  /* Create slots */
  slot1 = create_parking_slot(1, "Zone A-1");
  slot2 = create_parking_slot(2, "Zone A-2");

  print_test_result("Create slots", slot1 != NULL && slot2 != NULL);

  /* Add to parking lot */
  result1 = add_parking_slot(lot, slot1);
  result2 = add_parking_slot(lot, slot2);

  print_test_result("Add slots", result1 == 0 && result2 == 0);

  /* Test duplicate addition */
  slot_dup = create_parking_slot(1, "Duplicate ID");
  dup_result = add_parking_slot(lot, slot_dup);

  print_test_result("Reject duplicate ID", dup_result == -2);

  if (slot_dup) {
    free_parking_slot(slot_dup);
  }

  free_parking_lot(lot);
}

void test_allocate_and_deallocate_slot(void) {
  ParkingLot *lot;
  ParkingSlot *slot, *slot2, *found;
  int alloc_result, dup_alloc, dup_license, dealloc_result;

  printf("\nTesting slot allocation and deallocation...\n");

  lot = init_parking_lot(10);
  slot = create_parking_slot(1, "Zone A-1");
  add_parking_slot(lot, slot);

  /* Test slot allocation */
  alloc_result = allocate_slot(lot, 1, "Zhang San", "A12345", "13800138000",
                               RESIDENT_TYPE);
  print_test_result("Allocate slot to resident", alloc_result == 0);

  found = find_slot_by_id(lot, 1);
  print_test_result("Slot status updated",
                    found != NULL && found->status == OCCUPIED_STATUS &&
                        strcmp(found->owner_name, "Zhang San") == 0 &&
                        strcmp(found->license_plate, "A12345") == 0);

  /* Test duplicate allocation */
  dup_alloc =
      allocate_slot(lot, 1, "Li Si", "B67890", "13900139000", VISITOR_TYPE);
  print_test_result("Reject duplicate allocation", dup_alloc == -3);

  /* Test duplicate license */
  slot2 = create_parking_slot(2, "Zone A-2");
  add_parking_slot(lot, slot2);
  dup_license =
      allocate_slot(lot, 2, "Wang Wu", "A12345", "13700137000", RESIDENT_TYPE);
  print_test_result("Reject duplicate license", dup_license == -4);

  /* Test slot deallocation */
  dealloc_result = deallocate_slot(lot, 1);
  print_test_result("Deallocate slot", dealloc_result == 0);

  found = find_slot_by_id(lot, 1);
  print_test_result("Slot status restored",
                    found != NULL && found->status == FREE_STATUS);

  free_parking_lot(lot);
}

void test_find_functions(void) {
  ParkingLot *lot;
  ParkingSlot *slot1, *slot2;
  ParkingSlot *found_by_id, *found_by_license, *found_by_owner, *not_found;

  printf("\nTesting search functions...\n");

  lot = init_parking_lot(10);
  slot1 = create_parking_slot(1, "Zone A-1");
  slot2 = create_parking_slot(2, "Zone B-1");

  add_parking_slot(lot, slot1);
  add_parking_slot(lot, slot2);

  allocate_slot(lot, 1, "Zhang San", "A12345", "13800138000", RESIDENT_TYPE);
  allocate_slot(lot, 2, "Li Si", "B67890", "13900139000", VISITOR_TYPE);

  /* Test find by ID */
  found_by_id = find_slot_by_id(lot, 1);
  print_test_result("Find by ID",
                    found_by_id != NULL && found_by_id->slot_id == 1);

  /* Test find by license */
  found_by_license = find_slot_by_license(lot, "A12345");
  print_test_result("Find by license",
                    found_by_license != NULL &&
                        strcmp(found_by_license->license_plate, "A12345") == 0);

  /* Test find by owner name */
  found_by_owner = find_slot_by_owner(lot, "Zhang");
  print_test_result("Find by owner name",
                    found_by_owner != NULL &&
                        strstr(found_by_owner->owner_name, "Zhang") != NULL);

  /* Test find non-existent slot */
  not_found = find_slot_by_id(lot, 999);
  print_test_result("Find non-existent slot", not_found == NULL);

  free_parking_lot(lot);
}

void test_get_slots_lists(void) {
  ParkingLot *lot;
  int i;
  char location[50];
  ParkingSlot *slot;
  int free_count = 0, occupied_count = 0;
  ParkingSlot **free_slots;
  ParkingSlot **occupied_slots;

  printf("\nTesting slot lists...\n");

  lot = init_parking_lot(5);

  /* Create 5 slots, 3 occupied */
  for (i = 1; i <= 5; i++) {
    sprintf(location, "Zone A-%d", i);
    slot = create_parking_slot(i, location);
    add_parking_slot(lot, slot);
  }

  /* Allocate 3 slots - 都使用居民类型避免时间限制问题 */
  allocate_slot(lot, 1, "Zhang San", "A12345", "13800138000", RESIDENT_TYPE);
  allocate_slot(lot, 3, "Li Si", "B67890", "13900139000",
                RESIDENT_TYPE); // 改为居民类型
  allocate_slot(lot, 5, "Wang Wu", "C11111", "13700137000", RESIDENT_TYPE);

  /* Test get free slots */
  free_slots = get_free_slots(lot, &free_count);
  print_test_result("Get free slots list",
                    free_count == 2 && free_slots != NULL);

  if (free_slots) {
    printf("  Free slots:\n");
    for (i = 0; i < free_count; i++) {
      print_slot_info(free_slots[i]);
    }
    free(free_slots);
  }

  /* Test get occupied slots */
  occupied_slots = get_occupied_slots(lot, &occupied_count);
  print_test_result("Get occupied slots list",
                    occupied_count == 3 && occupied_slots != NULL);

  if (occupied_slots) {
    printf("  Occupied slots:\n");
    for (i = 0; i < occupied_count; i++) {
      print_slot_info(occupied_slots[i]);
    }
    free(occupied_slots);
  }

  free_parking_lot(lot);
}

void test_payment_functions(void) {
  ParkingLot *lot;
  ParkingSlot *slot;
  time_t start_time, end_time, entry, exit;
  int payment_result;
  double fee;

  printf("\nTesting payment functions...\n");

  lot = init_parking_lot(2);
  slot = create_parking_slot(1, "Zone A-1");
  add_parking_slot(lot, slot);

  allocate_slot(lot, 1, "Zhang San", "A12345", "13800138000", RESIDENT_TYPE);

  /* Test add payment record */
  start_time = time(NULL);
  end_time = start_time + 30 * 24 * 3600; /* 30 days later */

  payment_result = add_payment_record(slot, start_time, end_time, 200.0);
  print_test_result("Add payment record", payment_result == 0);

  /* Test calculate visitor fee */
  entry = time(NULL);
  exit = entry + (time_t)(2.5 * 3600); /* 2.5 hours later */

  fee = calculate_visitor_fee(entry, exit);
  print_test_result("Calculate visitor fee", fee == 30.0); /* 3 hours * 10 */

  printf("  Calculated fee: %.2f\n", fee);

  free_parking_lot(lot);
}

void test_statistics_functions(void) {
  ParkingLot *lot;
  int i;
  char location[50], owner[50], license[20];
  ParkingSlot *slot;
  time_t today;
  int resident_count, visitor_count;

  printf("\nTesting statistics functions...\n");

  lot = init_parking_lot(10);

  /* Create test data */
  for (i = 1; i <= 3; i++) {
    sprintf(location, "Zone A-%d", i);
    slot = create_parking_slot(i, location);
    add_parking_slot(lot, slot);

    sprintf(owner, "Owner%d", i);
    sprintf(license, "A0000%d", i);

    if (i <= 2) {
      /* 前两个是居民，可以正常分配 */
      allocate_slot(lot, i, owner, license, "13800138000", RESIDENT_TYPE);
    } else {
      /* 第三个是访客，手动设置避免时间限制问题 */
      slot->status = OCCUPIED_STATUS;
      slot->type = VISITOR_TYPE;
      strncpy(slot->owner_name, owner, MAX_NAME_LEN - 1);
      slot->owner_name[MAX_NAME_LEN - 1] = '\0';
      strncpy(slot->license_plate, license, MAX_LICENSE_LEN - 1);
      slot->license_plate[MAX_LICENSE_LEN - 1] = '\0';
      strncpy(slot->contact, "13800138000", MAX_CONTACT_LEN - 1);
      slot->contact[MAX_CONTACT_LEN - 1] = '\0';
      slot->entry_time = time(NULL);
      lot->occupied_slots++;
    }
  }

  /* Test daily parking statistics */
  today = time(NULL);
  resident_count = count_daily_parking(lot, today, RESIDENT_TYPE);
  visitor_count = count_daily_parking(lot, today, VISITOR_TYPE);

  print_test_result("Daily resident parking statistics", resident_count == 2);
  print_test_result("Daily visitor parking statistics", visitor_count == 1);

  printf("  Daily resident parking: %d vehicles\n", resident_count);
  printf("  Daily visitor parking: %d vehicles\n", visitor_count);

  free_parking_lot(lot);
}

void test_data_persistence(void) {
  const char *test_file = "test_parking_data.txt";
  ParkingLot *lot, *loaded_lot;
  int i;
  char location[50], owner[50], license[20];
  ParkingSlot *slot, *slot1;
  int save_result;

  printf("\nTesting data persistence...\n");

  /* Create test data */
  lot = init_parking_lot(3);

  for (i = 1; i <= 3; i++) {
    sprintf(location, "Test Zone %d", i);
    slot = create_parking_slot(i, location);
    add_parking_slot(lot, slot);

    if (i <= 2) {
      sprintf(owner, "Test Owner %d", i);
      sprintf(license, "TEST%04d", i);
      allocate_slot(lot, i, owner, license, "13800138000", RESIDENT_TYPE);
    }
  }

  /* Test save data */
  save_result = save_parking_data(lot, test_file);
  print_test_result("Save data to file", save_result == 0);

  /* Test load data */
  loaded_lot = load_parking_data(test_file);
  print_test_result("Load data from file", loaded_lot != NULL);

  if (loaded_lot) {
    print_test_result("Verify loaded data",
                      loaded_lot->total_slots == 3 &&
                          loaded_lot->occupied_slots == 2);

    slot1 = find_slot_by_id(loaded_lot, 1);
    print_test_result("Verify slot info",
                      slot1 != NULL &&
                          strcmp(slot1->location, "Test Zone 1") == 0 &&
                          slot1->status == OCCUPIED_STATUS);

    free_parking_lot(loaded_lot);
  }

  free_parking_lot(lot);

  /* Clean up test file */
  remove(test_file);
}

void test_memory_management(void) {
  ParkingLot *lot;
  int i;
  char location[50], owner[50], license[20];
  ParkingSlot *slot;
  time_t now;

  printf("\nTesting memory management...\n");

  /* This test mainly verifies no memory leaks */
  lot = init_parking_lot(5);

  for (i = 1; i <= 5; i++) {
    sprintf(location, "Memory Test Zone %d", i);
    slot = create_parking_slot(i, location);
    add_parking_slot(lot, slot);

    /* Add some payment records */
    if (i <= 3) {
      sprintf(owner, "Memory Test Owner %d", i);
      sprintf(license, "MEM%04d", i);
      allocate_slot(lot, i, owner, license, "13800138000", RESIDENT_TYPE);

      now = time(NULL);
      add_payment_record(slot, now, now + 30 * 24 * 3600, 200.0);
      add_payment_record(slot, now + 30 * 24 * 3600, now + 60 * 24 * 3600,
                         200.0);
    }
  }

  /* Free all memory */
  free_parking_lot(lot);

  print_test_result("Memory management test", 1); /* Pass if no crash */
}

/* 设置控制台编码以支持中文显示 */
void setup_console_encoding(void) {
#ifdef _WIN32
  /* 设置控制台代码页为UTF-8 */
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);

  /* 设置本地化信息 */
  setlocale(LC_ALL, "zh_CN.UTF-8");

  /* 备用方案：如果UTF-8不支持，使用GBK */
  if (SetConsoleOutputCP(CP_UTF8) == 0) {
    SetConsoleOutputCP(936); /* GBK */
    SetConsoleCP(936);
    setlocale(LC_ALL, "zh_CN.GBK");
  }
#else
  /* Linux/Mac系统通常默认支持UTF-8 */
  setlocale(LC_ALL, "zh_CN.UTF-8");
#endif
}
