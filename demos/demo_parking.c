#include "../src/parking_data.h"
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

void demo_basic_operations(void);
void demo_payment_system(void);
void demo_statistics(void);
void demo_persistence(void);

int main(void) {
  /* 设置控制台编码以支持中文显示 */
  setup_console_encoding();

  printf("停车管理系统演示程序\n");
  printf("====================\n\n");

  demo_basic_operations();
  demo_payment_system();
  demo_statistics();
  demo_persistence();
  printf("演示完成！\n");
  return 0;
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

void demo_basic_operations(void) {
  ParkingLot *lot;
  ParkingSlot *slot;
  ParkingSlot *found;
  ParkingSlot **free_slots;
  int index;
  int free_count;

  printf("1. 基本操作演示\n");
  printf("---------------\n");

  /* Initialize parking lot with 1000 slots */
  lot = init_parking_lot(1000);
  printf("创建停车场，总车位数: %d\n",
         lot->total_slots); /* Create some parking slots */
  for (index = 1; index <= 5; index++) {
    char location[50];
    sprintf(location, "Building A, Floor %d", index);
    slot = create_parking_slot(index, location);
    add_parking_slot(lot, slot);
  }
  printf("添加了5个停车位\n");

  /* Allocate some slots */
  allocate_slot(lot, 1, "张三", "A12345", "13800138000", RESIDENT_TYPE);
  allocate_slot(lot, 3, "李四", "B67890", "13900139000", VISITOR_TYPE);
  printf("分配了2个车位 (1个居民, 1个访客)\n");

  /* Query operations */
  found = find_slot_by_license(lot, "A12345");
  if (found) {
    printf("通过车牌号A12345找到车位: 车位%d, 车主: %s\n", found->slot_id,
           found->owner_name);
  }

  /* Get free slots */
  free_slots = get_free_slots(lot, &free_count);
  printf("空闲车位: %d个\n", free_count);
  if (free_slots) {
    free(free_slots);
  }

  printf("已占用车位: %d个\n", lot->occupied_slots);
  printf("\n");

  free_parking_lot(lot);
}

void demo_payment_system(void) {
  ParkingLot *lot;
  ParkingSlot *slot;
  time_t now, later;
  double fee;

  printf("2. 缴费系统演示\n");
  printf("---------------\n");

  lot = init_parking_lot(10);
  slot = create_parking_slot(1, "VIP专区");
  add_parking_slot(lot, slot);

  /* Resident payment */
  allocate_slot(lot, 1, "王五", "C11111", "13700137000", RESIDENT_TYPE);
  now = time(NULL);
  add_payment_record(slot, now, now + 30 * 24 * 3600, 200.0);
  printf("为居民添加月度缴费记录: 200元\n");

  /* Visitor fee calculation */
  later = now + 3 * 3600; /* 3 hours later */
  fee = calculate_visitor_fee(now, later);
  printf("访客停车3小时费用: %.2f元\n", fee);

  printf("\n");
  free_parking_lot(lot);
}

void demo_statistics(void) {
  ParkingLot *lot;
  ParkingSlot *slot;
  time_t today;
  int resident_count, visitor_count;
  struct tm *tm_info;

  printf("3. Statistics Demo\n");
  printf("------------------\n");

  lot = init_parking_lot(20);

  /* Create some test data */
  slot = create_parking_slot(1, "Zone A-1");
  add_parking_slot(lot, slot);
  allocate_slot(lot, 1, "Resident A", "R00001", "13800000001", RESIDENT_TYPE);

  slot = create_parking_slot(2, "Zone A-2");
  add_parking_slot(lot, slot);
  allocate_slot(lot, 2, "Resident B", "R00002", "13800000002", RESIDENT_TYPE);

  slot = create_parking_slot(3, "Zone B-1");
  add_parking_slot(lot, slot);
  /* Note: We cannot easily create visitor entries because of time restriction
   * (9:00-17:00) */
  /* For demo, we'll manually set a visitor */
  slot->status = OCCUPIED_STATUS;
  slot->type = VISITOR_TYPE;
  strcpy(slot->owner_name, "Visitor C");
  strcpy(slot->license_plate, "V00001");
  strcpy(slot->contact, "13900000001");
  slot->entry_time = time(NULL);
  lot->occupied_slots++;

  /* Statistics */
  today = time(NULL);
  tm_info = localtime(&today);

  resident_count = count_daily_parking(lot, today, RESIDENT_TYPE);
  visitor_count = count_daily_parking(lot, today, VISITOR_TYPE);

  printf("Today's parking statistics:\n");
  printf("- Residents: %d vehicles\n", resident_count);
  printf("- Visitors: %d vehicles\n", visitor_count);
  printf("- Free slots: %d\n", lot->total_slots - lot->occupied_slots);

  printf("\n");
  free_parking_lot(lot);
}

void demo_persistence(void) {
  ParkingLot *lot, *loaded_lot;
  ParkingSlot *slot;
  const char *filename = "demo_parking_data.txt";

  printf("4. Data Persistence Demo\n");
  printf("------------------------\n");

  /* Create and populate parking lot */
  lot = init_parking_lot(50);

  slot = create_parking_slot(101, "Premium Zone A-101");
  add_parking_slot(lot, slot);
  allocate_slot(lot, 101, "Premium User", "PREM001", "13800888888",
                RESIDENT_TYPE);

  slot = create_parking_slot(102, "Premium Zone A-102");
  add_parking_slot(lot, slot);
  /* Keep slot 102 free */

  printf("Created parking lot with 2 slots (1 occupied, 1 free)\n");

  /* Save to file */
  if (save_parking_data(lot, filename) == 0) {
    printf("Data saved to %s\n", filename);
  }

  /* Load from file */
  loaded_lot = load_parking_data(filename);
  if (loaded_lot) {
    printf("Data loaded successfully:\n");
    printf("- Total slots: %d\n", loaded_lot->total_slots);
    printf("- Occupied slots: %d\n", loaded_lot->occupied_slots);

    ParkingSlot *premium_slot = find_slot_by_id(loaded_lot, 101);
    if (premium_slot && premium_slot->status == OCCUPIED_STATUS) {
      printf("- Slot 101: %s (Owner: %s)\n", premium_slot->location,
             premium_slot->owner_name);
    }

    free_parking_lot(loaded_lot);
  }

  free_parking_lot(lot);
  remove(filename); /* Clean up */
  printf("\n");
}
