/**
 * @file parking_data.c
 * @brief 数据层实现文件
 * @details
 * 该文件实现了在 parking_data.h 中声明的所有数据操作函数。
 * 它负责管理停车场和停车位数据结构的创建、查询、修改和删除，
 * 以及数据的持久化（保存到文件和从文件加载）。
 * 这是系统的核心数据管理模块，不包含任何业务逻辑或用户界面代码。
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "parking_data.h"

/**
 * @brief (静态辅助函数) 解析并赋值单个字段。
 * @details 从文件加载数据时，根据字段索引将字符串值解析并赋给 ParkingSlot
 * 对象的相应成员。
 * @param slot 指向要修改的 ParkingSlot 对象的指针。
 * @param index 字段的索引 (0-9)。
 * @param value 要解析的字符串值。
 */
static void parse_and_assign_field(ParkingSlot *slot, int index,
                                   const char *value) {
  if (!value) {
    return;
  }
  switch (index) {
  case 0:
    slot->slot_id = atoi(value);
    break;
  case 1:
    strncpy(slot->location, value, MAX_LOCATION_LEN - 1);
    break;
  case 2:
    strncpy(slot->owner_name, value, MAX_NAME_LEN - 1);
    break;
  case 3:
    strncpy(slot->license_plate, value, MAX_LICENSE_LEN - 1);
    break;
  case 4:
    strncpy(slot->contact, value, MAX_CONTACT_LEN - 1);
    break;
  case 5:
    slot->type = (ParkingType)atoi(value);
    break;
  case 6:
    slot->entry_time = (time_t)atoll(value);
    break;
  case 7:
    slot->exit_time = (time_t)atoll(value);
    break;
  case 8:
    slot->status = (ParkingStatus)atoi(value);
    break;
  case 9:
    slot->resident_due_date = (time_t)atoll(value);
    break;
  }
}

/**
 * @brief 初始化一个新的停车场对象。
 * @details 为停车场分配内存并设置其初始状态，包括总车位数、占用数和链表头。
 * @param total_slots 停车场的总容量。
 * @return 成功时返回指向新分配的 ParkingLot 对象的指针，若内存分配失败则返回
 * NULL。
 */
ParkingLot *init_parking_lot(int total_slots) {
  ParkingLot *lot = (ParkingLot *)malloc(sizeof(ParkingLot));
  if (lot == NULL) {
    return NULL;
  }
  lot->total_slots = total_slots;
  lot->occupied_slots = 0;
  lot->slot_head = NULL;
  lot->today_revenue = 0.0;
  lot->month_revenue = 0.0;
  lot->last_update_time = 0;
  return lot;
}

/**
 * @brief 创建一个新的停车位对象。
 * @details 为单个停车位分配内存并初始化其属性。新创建的车位状态默认为空闲。
 * @param slot_id 要创建的车位的唯一编号。
 * @param location 车位的物理位置描述。
 * @return 成功时返回指向新分配的 ParkingSlot 对象的指针，若内存分配失败则返回
 * NULL。
 */
ParkingSlot *create_parking_slot(int slot_id, const char *location) {
  ParkingSlot *slot = (ParkingSlot *)malloc(sizeof(ParkingSlot));
  if (slot == NULL) {
    return NULL;
  }

  slot->slot_id = slot_id;
  strncpy(slot->location, location, MAX_LOCATION_LEN - 1);
  slot->location[MAX_LOCATION_LEN - 1] = '\0';

  /* 初始化为空状态 */
  slot->owner_name[0] = '\0';
  slot->license_plate[0] = '\0';
  slot->contact[0] = '\0';
  slot->type = RESIDENT_TYPE; /* 默认为居民类型 */
  slot->entry_time = 0;
  slot->exit_time = 0;
  slot->resident_due_date = 0;
  slot->status = FREE_STATUS;
  slot->next = NULL;

  return slot;
}

/**
 * @brief 将一个已创建的停车位添加到停车场链表中。
 * @details 使用头插法将车位节点添加到链表。在添加前会检查车位ID是否已存在。
 * @param lot 目标停车场。
 * @param slot 要添加的停车位节点。
 * @return 成功返回 0，若参数无效返回 -1，若车位ID已存在返回 -2。
 */
int add_parking_slot(ParkingLot *lot, ParkingSlot *slot) {
  if (!lot || !slot) {
    return -1;
  }

  /* 核心修复：添加重复ID检查 */
  if (find_slot_by_id(lot, slot->slot_id) != NULL) {
    return -2; /* ID already exists */
  }

  /* 头插法，新车位成为新的头节点 */
  slot->next = lot->slot_head;
  lot->slot_head = slot;
  return 0;
}

/**
 * @brief 根据车位编号查找停车位。
 * @param lot 目标停车场。
 * @param slot_id 要查找的车位编号。
 * @return 若找到，返回对应的 ParkingSlot 指针；否则返回 NULL。
 */
ParkingSlot *find_slot_by_id(ParkingLot *lot, int slot_id) {
  if (!lot) {
    return NULL;
  }
  ParkingSlot *current = lot->slot_head;
  while (current != NULL) {
    if (current->slot_id == slot_id) {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

/**
 * @brief 根据车牌号查找停车位。
 * @details 遍历所有车位，查找与给定车牌号匹配的已占用车位。
 * @note 只在状态为 OCCUPIED_STATUS 的车位中进行查找。
 * @param lot 目标停车场。
 * @param license_plate 要查找的车牌号。
 * @return 若找到，返回对应的 ParkingSlot 指针；否则返回 NULL。
 */
ParkingSlot *find_slot_by_license(ParkingLot *lot, const char *license_plate) {
  ParkingSlot *current;

  if (lot == NULL || license_plate == NULL) {
    return NULL;
  }

  current = lot->slot_head;
  while (current != NULL) {
    if (current->status == OCCUPIED_STATUS &&
        strcmp(current->license_plate, license_plate) == 0) {
      return current;
    }
    current = current->next;
  }

  return NULL;
}

/**
 * @brief 根据车主姓名查找停车位（模糊查找）。
 * @details 使用子字符串匹配 (strstr) 在已占用的车位中查找车主姓名。
 * @note 只在状态为 OCCUPIED_STATUS 的车位中进行查找。
 * @param lot 目标停车场。
 * @param owner_name 要查找的车主姓名（或姓名的一部分）。
 * @return 若找到，返回第一个匹配的 ParkingSlot 指针；否则返回 NULL。
 */
ParkingSlot *find_slot_by_owner(ParkingLot *lot, const char *owner_name) {
  ParkingSlot *current;

  if (lot == NULL || owner_name == NULL) {
    return NULL;
  }

  current = lot->slot_head;
  while (current != NULL) {
    if (current->status == OCCUPIED_STATUS &&
        strstr(current->owner_name, owner_name) != NULL) {
      return current;
    }
    current = current->next;
  }

  return NULL;
}

/**
 * @brief (静态辅助函数) 检查访客车辆的入场时间是否在允许的时间段内。
 * @details 检查给定时间戳的小时部分是否在 VISITOR_START_HOUR 和
 * VISITOR_END_HOUR 之间。
 * @param entry_time 车辆的入场时间戳。
 * @return 如果在允许时段内返回 1，否则返回 0。
 */
static int is_valid_visitor_time(time_t entry_time) {
  struct tm *tm_info = localtime(&entry_time);
  int hour = tm_info->tm_hour;

  return (hour >= VISITOR_START_HOUR && hour < VISITOR_END_HOUR);
}

/**
 * @brief 分配一个停车位给车辆（车辆入场）。
 * @details
 * 为指定车位分配车辆。此函数会执行一系列检查：
 * 1. 参数有效性。
 * 2. 车位是否存在且空闲。
 * 3. 该车牌号是否已在场内。
 * 4. 对于访客车辆，入场时间是否在允许时段内。
 * 检查通过后，更新车位信息并增加停车场的已占用计数。
 * @param lot 目标停车场。
 * @param slot_id 要分配的车位编号。
 * @param owner_name 车主姓名。
 * @param license_plate 车牌号。
 * @param contact 联系方式。
 * @param type 停车类型 (居民/访客)。
 * @return 返回码：0 成功, -1 参数无效, -2 车位不存在, -3 车位已被占用, -4
 * 该车牌号已在场内, -5 访客车辆在非允许时段入场。
 */
int allocate_slot(ParkingLot *lot, int slot_id, const char *owner_name,
                  const char *license_plate, const char *contact,
                  ParkingType type) {
  ParkingSlot *slot;
  time_t current_time;

  if (lot == NULL || owner_name == NULL || license_plate == NULL) {
    return -1; /* 参数无效 */
  }

  slot = find_slot_by_id(lot, slot_id);
  if (slot == NULL) {
    return -2; /* 车位不存在 */
  }

  if (slot->status == OCCUPIED_STATUS) {
    return -3; /* 车位已被占用 */
  }

  /* 检查车牌号是否已在其他车位 */
  if (find_slot_by_license(lot, license_plate) != NULL) {
    return -4; /* 车牌号已存在 */
  }

  current_time = time(NULL);

  /* 对于访客车辆，检查入场时间 */
  if (type == VISITOR_TYPE && !is_valid_visitor_time(current_time)) {
    return -5; /* 访客车辆在非允许时段入场 */
  }

  /* 分配车位并填充信息 */
  strncpy(slot->owner_name, owner_name, MAX_NAME_LEN - 1);
  slot->owner_name[MAX_NAME_LEN - 1] = '\0';

  strncpy(slot->license_plate, license_plate, MAX_LICENSE_LEN - 1);
  slot->license_plate[MAX_LICENSE_LEN - 1] = '\0';

  if (contact != NULL) {
    strncpy(slot->contact, contact, MAX_CONTACT_LEN - 1);
    slot->contact[MAX_CONTACT_LEN - 1] = '\0';
  } else {
    slot->contact[0] = '\0';
  }

  slot->type = type;
  slot->entry_time = current_time;
  slot->exit_time = 0; /* 清除上一次的出场时间 */
  slot->status = OCCUPIED_STATUS;

  lot->occupied_slots++;

  return 0; /* 成功 */
}

/**
 * @brief 释放一个停车位（车辆出场）。
 * @details
 * 检查车位是否存在且状态为已占用。成功后，记录出场时间，
 * 清空车主相关信息，将车位状态设置为空闲，并减少停车场的已占用计数。
 * @param lot 目标停车场。
 * @param slot_id 要释放的车位编号。
 * @return 返回码：0 成功, -1 停车场对象为空, -2 车位不存在, -3
 * 车位本就是空闲状态。
 */
int deallocate_slot(ParkingLot *lot, int slot_id) {
  ParkingSlot *slot;

  if (lot == NULL) {
    return -1; /* 停车场对象为空 */
  }

  slot = find_slot_by_id(lot, slot_id);
  if (slot == NULL) {
    return -2; /* 车位不存在 */
  }

  if (slot->status == FREE_STATUS) {
    return -3; /* 车位本就是空闲状态 */
  }

  slot->exit_time = time(NULL);

  /* 清空车位占用信息 */
  slot->owner_name[0] = '\0';
  slot->license_plate[0] = '\0';
  slot->contact[0] = '\0';
  slot->status = FREE_STATUS;

  lot->occupied_slots--;

  return 0; /* 成功 */
}

/**
 * @brief 获取所有空闲车位的列表。
 * @details 通过两次遍历实现：第一次统计空闲车位数量，第二次分配数组并填充指针。
 * @note 返回的数组需要调用者手动 `free()` 释放。
 * @param lot 目标停车场。
 * @param[out] count 用于接收空闲车位数量的指针。
 * @return 返回一个动态分配的 ParkingSlot
 * 指针数组。如果无空闲车位或内存分配失败，返回 NULL。
 */
ParkingSlot **get_free_slots(ParkingLot *lot, int *count) {
  ParkingSlot **free_slots;
  ParkingSlot *current;
  int free_count = 0;
  int index = 0;

  if (lot == NULL || count == NULL) {
    return NULL;
  }

  /* 第一遍：统计空闲车位数量 */
  current = lot->slot_head;
  while (current != NULL) {
    if (current->status == FREE_STATUS) {
      free_count++;
    }
    current = current->next;
  }

  *count = free_count;

  if (free_count == 0) {
    return NULL;
  }

  /* 分配指针数组内存 */
  free_slots = (ParkingSlot **)malloc(free_count * sizeof(ParkingSlot *));
  if (free_slots == NULL) {
    *count = 0;
    return NULL;
  }

  /* 第二遍：收集空闲车位指针 */
  current = lot->slot_head;
  while (current != NULL && index < free_count) {
    if (current->status == FREE_STATUS) {
      free_slots[index++] = current;
    }
    current = current->next;
  }

  return free_slots;
}

/**
 * @brief 获取所有已占用车位的列表。
 * @details 实现方式与 `get_free_slots` 类似，但筛选条件为已占用状态。
 * @note 返回的数组需要调用者手动 `free()` 释放。
 * @param lot 目标停车场。
 * @param[out] count 用于接收已占用车位的数量。
 * @return 返回一个动态分配的 ParkingSlot
 * 指针数组。如果无已占用车位或内存分配失败，返回 NULL。
 */
ParkingSlot **get_occupied_slots(ParkingLot *lot, int *count) {
  ParkingSlot **occupied_slots;
  ParkingSlot *current;
  int occupied_count = 0;
  int index = 0;

  if (lot == NULL || count == NULL) {
    return NULL;
  }

  /* 第一遍：统计已占用车位数量 */
  current = lot->slot_head;
  while (current != NULL) {
    if (current->status == OCCUPIED_STATUS) {
      occupied_count++;
    }
    current = current->next;
  }

  *count = occupied_count;

  if (occupied_count == 0) {
    return NULL;
  }

  /* 分配指针数组内存 */
  occupied_slots =
      (ParkingSlot **)malloc(occupied_count * sizeof(ParkingSlot *));
  if (occupied_slots == NULL) {
    *count = 0;
    return NULL;
  }

  /* 第二遍：收集已占用车位指针 */
  current = lot->slot_head;
  while (current != NULL && index < occupied_count) {
    if (current->status == OCCUPIED_STATUS) {
      occupied_slots[index++] = current;
    }
    current = current->next;
  }

  return occupied_slots;
}

/**
 * @brief (静态辅助函数) 计算一个车位的停车时长（单位：秒）。
 * @details 如果车辆已出场，则计算 entry_time 和 exit_time 的差值。
 *          如果车辆仍在场，则计算 entry_time 和当前时间的差值。
 * @param slot 要计算时长的车位。
 * @return 停车时长（秒）。如果车位为空闲或无入场时间，返回0。
 */
static long calculate_parking_duration(ParkingSlot *slot) {
  time_t current_time;

  if (slot->status == FREE_STATUS || slot->entry_time == 0) {
    return 0;
  }

  if (slot->exit_time > 0) {
    return (long)(slot->exit_time - slot->entry_time);
  }
  current_time = time(NULL);
  return (long)(current_time - slot->entry_time);
}

/**
 * @brief (静态辅助函数) qsort 的比较函数，用于按停车时长升序排列。
 * @param a 指向 ParkingSlot* 的指针。
 * @param b 指向 ParkingSlot* 的指针。
 * @return -1 如果 a < b, 1 如果 a > b, 0 如果 a == b。
 */
static int compare_parking_duration_asc(const void *a, const void *b) {
  ParkingSlot *slot_a = *(ParkingSlot **)a;
  ParkingSlot *slot_b = *(ParkingSlot **)b;

  long duration_a = calculate_parking_duration(slot_a);
  long duration_b = calculate_parking_duration(slot_b);

  if (duration_a < duration_b) {
    return -1;
  }
  if (duration_a > duration_b) {
    return 1;
  }
  return 0;
}

/**
 * @brief (静态辅助函数) qsort 的比较函数，用于按停车时长降序排列。
 * @param a 指向 ParkingSlot* 的指针。
 * @param b 指向 ParkingSlot* 的指针。
 * @return 1 如果 a < b, -1 如果 a > b, 0 如果 a == b。
 */
static int compare_parking_duration_desc(const void *a, const void *b) {
  return -compare_parking_duration_asc(a, b);
}

/**
 * @brief 按停车时长排序后，获取已占用车辆列表。
 * @details 首先获取所有已占用车位的列表，然后使用 `qsort`
 * 和自定义比较函数进行排序。
 * @note 返回的数组需要调用者手动 `free()` 释放。
 * @param lot 目标停车场。
 * @param[out] count 用于接收已占用车位的数量。
 * @param ascending 是否按升序排列 (1: 升序, 0: 降序)。
 * @return 返回排序后的 ParkingSlot 指针数组。数组使用后需要调用 free() 释放。
 */
ParkingSlot **get_slots_by_duration(ParkingLot *lot, int *count,
                                    int ascending) {
  ParkingSlot **occupied_slots = get_occupied_slots(lot, count);

  if (occupied_slots == NULL || *count == 0) {
    return occupied_slots;
  }

  /* 使用 qsort 进行排序 */
  if (ascending) {
    qsort(occupied_slots, *count, sizeof(ParkingSlot *),
          compare_parking_duration_asc);
  } else {
    qsort(occupied_slots, *count, sizeof(ParkingSlot *),
          compare_parking_duration_desc);
  }

  return occupied_slots;
}

/**
 * @brief 计算访客车辆的停车费用。
 * @details 停车时长按小时向上取整，然后乘以小时费率。
 * @param entry_time 车辆入场时间戳。
 * @param exit_time 车辆出场时间戳。
 * @return 停车费用。如果出场时间早于或等于入场时间，返回0.0。
 */
double calculate_visitor_fee(time_t entry_time, time_t exit_time) {
  double hours;
  long duration_seconds;

  if (exit_time <= entry_time) {
    return 0.0;
  }

  duration_seconds = (long)(exit_time - entry_time);
  hours = duration_seconds / 3600.0;

  /* 不足1小时按1小时计费 */
  return ceil(hours) * VISITOR_HOURLY_FEE;
}

/**
 * @brief 更新一个停车位的信息。
 * @details 根据传入的非 NULL 参数更新车位信息。
 * @note `location` 字段可以随时更新。`owner_name` 和 `contact`
 * 字段只有在车位被占用时才能更新。
 * @param slot 要更新的停车位。
 * @param location 新的位置描述 (如果为NULL则不更新)。
 * @param owner_name 新的车主姓名 (如果为NULL则不更新)。
 * @param contact 新的联系方式 (如果为NULL则不更新)。
 * @return 成功返回 0，如果 `slot` 参数为 NULL 则返回 -1。
 */
int update_slot_info(ParkingSlot *slot, const char *location,
                     const char *owner_name, const char *contact) {
  if (slot == NULL) {
    return -1;
  }

  if (location != NULL) {
    strncpy(slot->location, location, MAX_LOCATION_LEN - 1);
    slot->location[MAX_LOCATION_LEN - 1] = '\0';
  }

  if (owner_name != NULL && slot->status == OCCUPIED_STATUS) {
    strncpy(slot->owner_name, owner_name, MAX_NAME_LEN - 1);
    slot->owner_name[MAX_NAME_LEN - 1] = '\0';
  }

  if (contact != NULL && slot->status == OCCUPIED_STATUS) {
    strncpy(slot->contact, contact, MAX_CONTACT_LEN - 1);
    slot->contact[MAX_CONTACT_LEN - 1] = '\0';
  }

  return 0;
}

/**
 * @brief 从停车场中删除一个车位。
 * @details 从链表中找到并移除指定ID的车位节点，然后释放其内存。
 * @note 只有空闲车位才能被删除。
 * @param lot 目标停车场。
 * @param slot_id 要删除的车位编号。
 * @return 返回码：0 成功, -1 停车场对象为空, -2 车位正在被使用，无法删除, -3
 * 车位不存在。
 */
int delete_slot(ParkingLot *lot, int slot_id) {
  ParkingSlot *current;
  ParkingSlot *prev = NULL;

  if (lot == NULL) {
    return -1; /* 停车场对象为空 */
  }

  current = lot->slot_head;
  while (current != NULL) {
    if (current->slot_id == slot_id) {
      /* 检查车位是否为空闲状态 */
      if (current->status == OCCUPIED_STATUS) {
        return -2; /* 车位正在被使用，无法删除 */
      }

      /* 从链表中移除节点 */
      if (prev == NULL) { /* 如果是头节点 */
        lot->slot_head = current->next;
      } else {
        prev->next = current->next;
      }

      /* 释放内存并更新总数 */
      free_parking_slot(current);
      lot->total_slots--;

      return 0; /* 成功 */
    }
    prev = current;
    current = current->next;
  }

  return -3; /* 车位不存在 */
}

/**
 * @brief 统计指定日期内某类型车辆的入场总数。
 * @details 遍历所有已占用车位，比较其入场时间的年、月、日与指定日期是否匹配。
 * @param lot 目标停车场。
 * @param date 指定的日期 (time_t)。
 * @param type 车辆类型 (居民/访客)。
 * @return 指定日期和类型的车辆入场总数。若 `lot` 为 NULL 返回 -1。
 */
int count_daily_parking(ParkingLot *lot, time_t date, ParkingType type) {
  ParkingSlot *current;
  int count = 0;
  struct tm *target_date;
  struct tm *entry_date;

  if (lot == NULL) {
    return -1;
  }

  target_date = localtime(&date);

  current = lot->slot_head;
  while (current != NULL) {
    if (current->status == OCCUPIED_STATUS && current->type == type &&
        current->entry_time > 0) {

      entry_date = localtime(&current->entry_time);

      /* 比较年、月、日是否都相同 */
      if (target_date->tm_year == entry_date->tm_year &&
          target_date->tm_mon == entry_date->tm_mon &&
          target_date->tm_mday == entry_date->tm_mday) {
        count++;
      }
    }
    current = current->next;
  }

  return count;
}

/**
 * @brief 统计指定月份内某类型车辆的入场总数。
 * @details 遍历所有已占用车位，比较其入场时间的年份和月份是否与指定年月匹配。
 * @param lot 目标停车场。
 * @param year 年份。
 * @param month 月份 (1-12)。
 * @param type 车辆类型 (居民/访客)。
 * @return 指定月份和类型的车辆入场总数。若参数无效返回 -1。
 */
int count_monthly_parking(ParkingLot *lot, int year, int month,
                          ParkingType type) {
  ParkingSlot *current;
  int count = 0;
  struct tm *entry_date;

  if (lot == NULL || month < 1 || month > 12) {
    return -1;
  }

  current = lot->slot_head;
  while (current != NULL) {
    if (current->status == OCCUPIED_STATUS && current->type == type &&
        current->entry_time > 0) {

      entry_date = localtime(&current->entry_time);

      /* 比较年份和月份是否相同 */
      if ((entry_date->tm_year + 1900) == year &&
          (entry_date->tm_mon + 1) == month) {
        count++;
      }
    }
    current = current->next;
  }

  return count;
}

/**
 * @brief 将停车场的所有数据保存到文本文件。
 * @details
 * 将停车场元数据和所有车位信息序列化到文本文件。文件格式如下：
 * - 停车场信息: `LOT|<total_slots>`
 * - 车位信息:
 * `SLOT|id|location|owner|license|contact|type|entry|exit|status|due_date`
 *   空闲车位的车主、车牌等信息为空。
 * @param lot 要保存的停车场。
 * @param filename 目标文件名。
 * @return 成功返回 0，若参数无效或文件无法打开则返回 -1。
 */
int save_parking_data(ParkingLot *lot, const char *filename) {
  FILE *file;
  ParkingSlot *current;

  if (lot == NULL || filename == NULL) {
    return -1;
  }

  file = fopen(filename, "w");
  if (file == NULL) {
    return -1;
  }

  /* 核心修复：只保存总车位数，与 load_parking_data 的解析逻辑同步 */
  fprintf(file, "LOT|%d\n", lot->total_slots);

  /* 遍历并保存每个车位的信息 */
  for (current = lot->slot_head; current != NULL; current = current->next) {
    if (current->status == OCCUPIED_STATUS) {
      fprintf(file, "SLOT|%d|%s|%s|%s|%s|%d|%lld|%lld|%d|%lld\n",
              current->slot_id, current->location, current->owner_name,
              current->license_plate, current->contact, current->type,
              (long long)current->entry_time, (long long)current->exit_time,
              current->status, (long long)current->resident_due_date);
    } else {
      fprintf(file, "SLOT|%d|%s||||%d|0|0|%d|0\n", current->slot_id,
              current->location, current->type, current->status);
    }
  }

  fclose(file);
  return 0;
}

/**
 * @brief 从文本文件中加载停车场数据。
 * @details
 * 解析与 `save_parking_data` 函数格式兼容的文件，重建停车场对象。
 * 首先读取 `LOT` 行初始化停车场，然后逐行读取 `SLOT`
 * 行并创建/添加车位。
 * 加载完成后会重新计算已占用车位数以确保数据一致性。
 * @param filename 源文件名。
 * @return 成功时返回重建的 ParkingLot 指针；失败（文件不存在或格式错误）时返回
 * NULL。
 */
ParkingLot *load_parking_data(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    return NULL;
  }

  char line[512];
  ParkingLot *lot = NULL;

  /* 读取并解析LOT行 */
  if (fgets(line, sizeof(line), file)) {
    int total = 0;
    if (sscanf(line, "LOT|%d", &total) == 1 && total > 0) {
      lot = init_parking_lot(total);
    }
  }

  if (!lot) {
    fclose(file);
    return NULL;
  }

  /* 读取并解析所有SLOT行 */
  while (fgets(line, sizeof(line), file)) {
    if (strncmp(line, "SLOT|", 5) != 0) {
      continue;
    }

    ParkingSlot *slot = (ParkingSlot *)malloc(sizeof(ParkingSlot));
    if (!slot) {
      continue;
    }
    memset(slot, 0, sizeof(ParkingSlot));

    char *p = line + 5;
    char *field_start = p;
    int field_index = 0;

    for (;; ++p) {
      if (*p == '|' || *p == '\0' || *p == '\n' || *p == '\r') {
        char saved_char = *p;
        *p = '\0';
        parse_and_assign_field(slot, field_index, field_start);
        *p = saved_char;

        field_index++;
        field_start = p + 1;

        if (saved_char == '\0' || saved_char == '\n' || saved_char == '\r' ||
            field_index >= 10) {
          break;
        }
      }
    }
    add_parking_slot(lot, slot);
  }

  fclose(file);

  /* 为确保数据一致性，加载后重新计算已占用车位数 */
  int occupied_count = 0;
  ParkingSlot *current = lot->slot_head;
  while (current) {
    if (current->status == OCCUPIED_STATUS) {
      occupied_count++;
    }
    current = current->next;
  }
  lot->occupied_slots = occupied_count;

  return lot;
}

/**
 * @brief 释放单个停车位对象占用的内存。
 * @param slot 要释放的停车位。
 */
void free_parking_slot(ParkingSlot *slot) {
  if (slot != NULL) {
    free(slot);
  }
}

/**
 * @brief 释放整个停车场（包括所有车位）占用的内存。
 * @details 遍历链表，逐个释放所有停车位节点，最后释放停车场本身。
 * @param lot 要释放的停车场。
 */
void free_parking_lot(ParkingLot *lot) {
  if (lot != NULL) {
    ParkingSlot *current = lot->slot_head;
    while (current != NULL) {
      ParkingSlot *next = current->next;
      free_parking_slot(current);
      current = next;
    }
    free(lot);
  }
}
