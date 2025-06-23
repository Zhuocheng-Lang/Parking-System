#include "parking_data.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* 常量定义 */
#define RESIDENT_MONTHLY_FEE 200.0 /* 居民月费 */
#define VISITOR_HOURLY_FEE 10.0    /* 外来车辆小时费 */
#define VISITOR_START_HOUR 9       /* 外来车辆允许入场开始时间 */
#define VISITOR_END_HOUR 17        /* 外来车辆允许入场结束时间 */

/* 初始化停车场 */
ParkingLot *init_parking_lot(int total_slots) {
  ParkingLot *lot = (ParkingLot *)malloc(sizeof(ParkingLot));
  if (lot == NULL) {
    return NULL;
  }

  lot->head = NULL;
  lot->total_slots = total_slots;
  lot->occupied_slots = 0;

  return lot;
}

/* 创建停车位 */
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
  slot->type = RESIDENT_TYPE;
  slot->entry_time = 0;
  slot->exit_time = 0;
  slot->status = FREE_STATUS;
  slot->payment_head = NULL;
  slot->next = NULL;

  return slot;
}

/* 添加停车位到停车场 */
int add_parking_slot(ParkingLot *lot, ParkingSlot *slot) {
  if (lot == NULL || slot == NULL) {
    return -1;
  }

  /* 检查车位编号是否已存在 */
  if (find_slot_by_id(lot, slot->slot_id) != NULL) {
    return -2; /* 车位编号已存在 */
  }

  /* 头插法 */
  slot->next = lot->head;
  lot->head = slot;

  return 0;
}

/* 根据车位编号查找停车位 */
ParkingSlot *find_slot_by_id(ParkingLot *lot, int slot_id) {
  ParkingSlot *current;

  if (lot == NULL) {
    return NULL;
  }

  current = lot->head;
  while (current != NULL) {
    if (current->slot_id == slot_id) {
      return current;
    }
    current = current->next;
  }

  return NULL;
}

/* 根据车牌号查找停车位 */
ParkingSlot *find_slot_by_license(ParkingLot *lot, const char *license_plate) {
  ParkingSlot *current;

  if (lot == NULL || license_plate == NULL) {
    return NULL;
  }

  current = lot->head;
  while (current != NULL) {
    if (current->status == OCCUPIED_STATUS &&
        strcmp(current->license_plate, license_plate) == 0) {
      return current;
    }
    current = current->next;
  }

  return NULL;
}

/* 根据车主姓名查找停车位 */
ParkingSlot *find_slot_by_owner(ParkingLot *lot, const char *owner_name) {
  ParkingSlot *current;

  if (lot == NULL || owner_name == NULL) {
    return NULL;
  }

  current = lot->head;
  while (current != NULL) {
    if (current->status == OCCUPIED_STATUS &&
        strstr(current->owner_name, owner_name) != NULL) {
      return current;
    }
    current = current->next;
  }

  return NULL;
}

/* 检查外来车辆入场时间是否合规 */
static int is_valid_visitor_time(time_t entry_time) {
  struct tm *tm_info = localtime(&entry_time);
  int hour = tm_info->tm_hour;

  return (hour >= VISITOR_START_HOUR && hour < VISITOR_END_HOUR);
}

/* 分配停车位（车辆入场） */
int allocate_slot(ParkingLot *lot, int slot_id, const char *owner_name,
                  const char *license_plate, const char *contact,
                  ParkingType type) {
  ParkingSlot *slot;
  time_t current_time;

  if (lot == NULL || owner_name == NULL || license_plate == NULL) {
    return -1;
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

  /* 对于外来车辆，检查入场时间 */
  if (type == VISITOR_TYPE && !is_valid_visitor_time(current_time)) {
    return -5; /* 外来车辆入场时间不合规 */
  }

  /* 分配车位 */
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
  slot->exit_time = 0;
  slot->status = OCCUPIED_STATUS;

  lot->occupied_slots++;

  return 0;
}

/* 释放停车位（车辆出场） */
int deallocate_slot(ParkingLot *lot, int slot_id) {
  ParkingSlot *slot;

  if (lot == NULL) {
    return -1;
  }

  slot = find_slot_by_id(lot, slot_id);
  if (slot == NULL) {
    return -2; /* 车位不存在 */
  }

  if (slot->status == FREE_STATUS) {
    return -3; /* 车位本来就是空闲的 */
  }

  slot->exit_time = time(NULL);

  /* 清空车位信息 */
  slot->owner_name[0] = '\0';
  slot->license_plate[0] = '\0';
  slot->contact[0] = '\0';
  slot->status = FREE_STATUS;

  lot->occupied_slots--;

  return 0;
}

/* 获取空闲停车位列表 */
ParkingSlot **get_free_slots(ParkingLot *lot, int *count) {
  ParkingSlot **free_slots;
  ParkingSlot *current;
  int free_count = 0;
  int index = 0;

  if (lot == NULL || count == NULL) {
    return NULL;
  }

  /* 首先统计空闲车位数量 */
  current = lot->head;
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

  /* 分配数组 */
  free_slots = (ParkingSlot **)malloc(free_count * sizeof(ParkingSlot *));
  if (free_slots == NULL) {
    *count = 0;
    return NULL;
  }

  /* 收集空闲车位 */
  current = lot->head;
  while (current != NULL && index < free_count) {
    if (current->status == FREE_STATUS) {
      free_slots[index++] = current;
    }
    current = current->next;
  }

  return free_slots;
}

/* 获取已占用停车位列表 */
ParkingSlot **get_occupied_slots(ParkingLot *lot, int *count) {
  ParkingSlot **occupied_slots;
  ParkingSlot *current;
  int occupied_count = 0;
  int index = 0;

  if (lot == NULL || count == NULL) {
    return NULL;
  }

  /* 首先统计已占用车位数量 */
  current = lot->head;
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

  /* 分配数组 */
  occupied_slots =
      (ParkingSlot **)malloc(occupied_count * sizeof(ParkingSlot *));
  if (occupied_slots == NULL) {
    *count = 0;
    return NULL;
  }

  /* 收集已占用车位 */
  current = lot->head;
  while (current != NULL && index < occupied_count) {
    if (current->status == OCCUPIED_STATUS) {
      occupied_slots[index++] = current;
    }
    current = current->next;
  }

  return occupied_slots;
}

/* 计算停车时长（秒） */
static long calculate_parking_duration(ParkingSlot *slot) {
  time_t current_time;

  if (slot->status == FREE_STATUS || slot->entry_time == 0) {
    return 0;
  }

  if (slot->exit_time > 0) {
    return (long)(slot->exit_time - slot->entry_time);
  } else {
    current_time = time(NULL);
    return (long)(current_time - slot->entry_time);
  }
}

/* 比较函数用于排序停车时长 */
static int compare_parking_duration_asc(const void *a, const void *b) {
  ParkingSlot *slot_a = *(ParkingSlot **)a;
  ParkingSlot *slot_b = *(ParkingSlot **)b;

  long duration_a = calculate_parking_duration(slot_a);
  long duration_b = calculate_parking_duration(slot_b);

  if (duration_a < duration_b)
    return -1;
  if (duration_a > duration_b)
    return 1;
  return 0;
}

static int compare_parking_duration_desc(const void *a, const void *b) {
  return -compare_parking_duration_asc(a, b);
}

/* 按停车时长排序获取车辆列表 */
ParkingSlot **get_slots_by_duration(ParkingLot *lot, int *count,
                                    int ascending) {
  ParkingSlot **occupied_slots = get_occupied_slots(lot, count);

  if (occupied_slots == NULL || *count == 0) {
    return occupied_slots;
  }

  /* 排序 */
  if (ascending) {
    qsort(occupied_slots, *count, sizeof(ParkingSlot *),
          compare_parking_duration_asc);
  } else {
    qsort(occupied_slots, *count, sizeof(ParkingSlot *),
          compare_parking_duration_desc);
  }

  return occupied_slots;
}

/* 添加缴费记录 */
int add_payment_record(ParkingSlot *slot, time_t start_date, time_t end_date,
                       double amount) {
  PaymentRecord *record;

  if (slot == NULL) {
    return -1;
  }

  record = (PaymentRecord *)malloc(sizeof(PaymentRecord));
  if (record == NULL) {
    return -2;
  }

  record->start_date = start_date;
  record->end_date = end_date;
  record->amount = amount;
  record->next = slot->payment_head;
  slot->payment_head = record;

  return 0;
}

/* 计算外来车辆停车费用 */
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

/* 修改停车位信息 */
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

/* 删除停车位 */
int delete_slot(ParkingLot *lot, int slot_id) {
  ParkingSlot *current;
  ParkingSlot *prev = NULL;

  if (lot == NULL) {
    return -1;
  }

  current = lot->head;
  while (current != NULL) {
    if (current->slot_id == slot_id) {
      /* 检查车位是否为空闲状态 */
      if (current->status == OCCUPIED_STATUS) {
        return -2; /* 车位非空闲状态，不能删除 */
      }

      /* 从链表中移除 */
      if (prev == NULL) {
        lot->head = current->next;
      } else {
        prev->next = current->next;
      }

      /* 释放内存 */
      free_parking_slot(current);
      lot->total_slots--;

      return 0;
    }
    prev = current;
    current = current->next;
  }

  return -3; /* 车位不存在 */
}

/* 统计指定日期停车数量 */
int count_daily_parking(ParkingLot *lot, time_t date, ParkingType type) {
  ParkingSlot *current;
  int count = 0;
  struct tm *target_date;
  struct tm *entry_date;

  if (lot == NULL) {
    return -1;
  }

  target_date = localtime(&date);

  current = lot->head;
  while (current != NULL) {
    if (current->status == OCCUPIED_STATUS && current->type == type &&
        current->entry_time > 0) {

      entry_date = localtime(&current->entry_time);

      /* 检查是否为同一天 */
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

/* 统计指定月份停车数量 */
int count_monthly_parking(ParkingLot *lot, int year, int month,
                          ParkingType type) {
  ParkingSlot *current;
  int count = 0;
  struct tm *entry_date;

  if (lot == NULL || month < 1 || month > 12) {
    return -1;
  }

  current = lot->head;
  while (current != NULL) {
    if (current->status == OCCUPIED_STATUS && current->type == type &&
        current->entry_time > 0) {

      entry_date = localtime(&current->entry_time);

      /* 检查是否为指定年月 */
      if ((entry_date->tm_year + 1900) == year &&
          (entry_date->tm_mon + 1) == month) {
        count++;
      }
    }
    current = current->next;
  }

  return count;
}

/* 统计指定月份居民缴费总额 */
double get_monthly_payment_total(ParkingLot *lot, int year, int month) {
  ParkingSlot *current;
  PaymentRecord *payment;
  double total = 0.0;
  struct tm *payment_date;

  if (lot == NULL || month < 1 || month > 12) {
    return -1.0;
  }

  current = lot->head;
  while (current != NULL) {
    if (current->type == RESIDENT_TYPE) {
      payment = current->payment_head;
      while (payment != NULL) {
        payment_date = localtime(&payment->start_date);

        /* 检查缴费记录是否在指定年月 */
        if ((payment_date->tm_year + 1900) == year &&
            (payment_date->tm_mon + 1) == month) {
          total += payment->amount;
        }
        payment = payment->next;
      }
    }
    current = current->next;
  }

  return total;
}

/* 保存停车场数据到文件 */
int save_parking_data(ParkingLot *lot, const char *filename) {
  FILE *file;
  ParkingSlot *current;
  PaymentRecord *payment;

  if (lot == NULL || filename == NULL) {
    return -1;
  }

  file = fopen(filename, "w");
  if (file == NULL) {
    return -2;
  }

  /* 写入停车场基本信息 */
  fprintf(file, "PARKING_LOT,%d,%d\n", lot->total_slots, lot->occupied_slots);

  /* 写入每个停车位信息 */
  current = lot->head;
  while (current != NULL) {
    fprintf(file, "SLOT,%d,%s,%s,%s,%s,%d,%ld,%ld,%d\n", current->slot_id,
            current->location, current->owner_name, current->license_plate,
            current->contact, (int)current->type, (long)current->entry_time,
            (long)current->exit_time, (int)current->status);

    /* 写入缴费记录 */
    payment = current->payment_head;
    while (payment != NULL) {
      fprintf(file, "PAYMENT,%d,%ld,%ld,%.2f\n", current->slot_id,
              (long)payment->start_date, (long)payment->end_date,
              payment->amount);
      payment = payment->next;
    }

    current = current->next;
  }

  fclose(file);
  return 0;
}

/* 从文件加载停车场数据 */
ParkingLot *load_parking_data(const char *filename) {
  FILE *file;
  char line[512];
  char *token;
  ParkingLot *lot = NULL;
  ParkingSlot *slot = NULL;
  int total_slots, occupied_slots;

  if (filename == NULL) {
    return NULL;
  }

  file = fopen(filename, "r");
  if (file == NULL) {
    return NULL;
  }

  while (fgets(line, sizeof(line), file) != NULL) {
    /* 移除换行符 */
    line[strcspn(line, "\r\n")] = '\0';

    token = strtok(line, ",");
    if (token == NULL) {
      continue;
    }

    if (strcmp(token, "PARKING_LOT") == 0) {
      /* 解析停车场信息 */
      token = strtok(NULL, ",");
      if (token == NULL)
        continue;
      total_slots = atoi(token);

      token = strtok(NULL, ",");
      if (token == NULL)
        continue;
      occupied_slots = atoi(token);

      lot = init_parking_lot(total_slots);
      if (lot == NULL) {
        fclose(file);
        return NULL;
      }
      lot->occupied_slots = occupied_slots;

    } else if (strcmp(token, "SLOT") == 0 && lot != NULL) {
      /* 解析停车位信息 */
      int slot_id, type, status;
      long entry_time, exit_time;
      char location[MAX_LOCATION_LEN];
      char owner_name[MAX_NAME_LEN];
      char license_plate[MAX_LICENSE_LEN];
      char contact[MAX_CONTACT_LEN];

      token = strtok(NULL, ",");
      if (token == NULL)
        continue;
      slot_id = atoi(token);

      token = strtok(NULL, ",");
      if (token == NULL)
        continue;
      strncpy(location, token, MAX_LOCATION_LEN - 1);
      location[MAX_LOCATION_LEN - 1] = '\0';

      token = strtok(NULL, ",");
      if (token == NULL)
        continue;
      strncpy(owner_name, token, MAX_NAME_LEN - 1);
      owner_name[MAX_NAME_LEN - 1] = '\0';

      token = strtok(NULL, ",");
      if (token == NULL)
        continue;
      strncpy(license_plate, token, MAX_LICENSE_LEN - 1);
      license_plate[MAX_LICENSE_LEN - 1] = '\0';

      token = strtok(NULL, ",");
      if (token == NULL)
        continue;
      strncpy(contact, token, MAX_CONTACT_LEN - 1);
      contact[MAX_CONTACT_LEN - 1] = '\0';

      token = strtok(NULL, ",");
      if (token == NULL)
        continue;
      type = atoi(token);

      token = strtok(NULL, ",");
      if (token == NULL)
        continue;
      entry_time = atol(token);

      token = strtok(NULL, ",");
      if (token == NULL)
        continue;
      exit_time = atol(token);

      token = strtok(NULL, ",");
      if (token == NULL)
        continue;
      status = atoi(token);

      slot = create_parking_slot(slot_id, location);
      if (slot != NULL) {
        strncpy(slot->owner_name, owner_name, MAX_NAME_LEN - 1);
        slot->owner_name[MAX_NAME_LEN - 1] = '\0';
        strncpy(slot->license_plate, license_plate, MAX_LICENSE_LEN - 1);
        slot->license_plate[MAX_LICENSE_LEN - 1] = '\0';
        strncpy(slot->contact, contact, MAX_CONTACT_LEN - 1);
        slot->contact[MAX_CONTACT_LEN - 1] = '\0';
        slot->type = (ParkingType)type;
        slot->entry_time = (time_t)entry_time;
        slot->exit_time = (time_t)exit_time;
        slot->status = (ParkingStatus)status;

        add_parking_slot(lot, slot);
      }

    } else if (strcmp(token, "PAYMENT") == 0 && lot != NULL) {
      /* 解析缴费记录 */
      int slot_id;
      long start_date, end_date;
      double amount;

      token = strtok(NULL, ",");
      if (token == NULL)
        continue;
      slot_id = atoi(token);

      token = strtok(NULL, ",");
      if (token == NULL)
        continue;
      start_date = atol(token);

      token = strtok(NULL, ",");
      if (token == NULL)
        continue;
      end_date = atol(token);

      token = strtok(NULL, ",");
      if (token == NULL)
        continue;
      amount = atof(token);

      slot = find_slot_by_id(lot, slot_id);
      if (slot != NULL) {
        add_payment_record(slot, (time_t)start_date, (time_t)end_date, amount);
      }
    }
  }

  fclose(file);
  return lot;
}

/* 释放缴费记录链表 */
void free_payment_records(PaymentRecord *head) {
  PaymentRecord *current = head;
  PaymentRecord *next;

  while (current != NULL) {
    next = current->next;
    free(current);
    current = next;
  }
}

/* 释放停车位内存 */
void free_parking_slot(ParkingSlot *slot) {
  if (slot != NULL) {
    free_payment_records(slot->payment_head);
    free(slot);
  }
}

/* 释放停车场内存 */
void free_parking_lot(ParkingLot *lot) {
  ParkingSlot *current;
  ParkingSlot *next;

  if (lot != NULL) {
    current = lot->head;
    while (current != NULL) {
      next = current->next;
      free_parking_slot(current);
      current = next;
    }
    free(lot);
  }
}