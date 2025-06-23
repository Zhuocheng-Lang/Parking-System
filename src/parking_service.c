#include "parking_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <locale.h>
#include <windows.h>

#endif

/* 常量定义 */
#define MAX_SLOT_ID 99999
#define MIN_LICENSE_LEN 5
#define MIN_CONTACT_LEN 8
#define MAX_PAYMENT_DAYS 365
#define HOURS_PER_DAY 24
#define SECONDS_PER_HOUR 3600
#define TM_YEAR_OFFSET 1900
#define VISITOR_TIME_ERROR_CODE -5

/*
 * 停车服务层实现
 * 提供高级业务逻辑和操作封装
 * 分离UI层和数据层，提供清晰的业务接口
 */

/* ===== 内部辅助函数 ===== */

/* 创建服务结果 */
static ServiceResult create_service_result(ParkingServiceResult code,
                                           const char *message, void *data) {
  ServiceResult result;
  result.code = code;
  result.data = data;

  if (message) {
    strncpy(result.message, message, sizeof(result.message) - 1);
    result.message[sizeof(result.message) - 1] = '\0';
  } else {
    result.message[0] = '\0';
  }

  return result;
}

/* 验证车位编号 */
static int validate_slot_id(int slot_id) {
  return (slot_id > 0 && slot_id <= MAX_SLOT_ID);
}

/* 验证车牌号格式 */
static int validate_license_plate(const char *license) {
  size_t license_index;
  size_t len;
  int has_chinese = 0;
  int has_letter = 0;
  int has_digit = 0;

  if (!license) {
    return 0;
  }

  len = strlen(license);
  if (len < MIN_LICENSE_LEN || len >= MAX_LICENSE_LEN) {
    return 0;
  }

  /* 检查车牌号格式 */
  for (license_index = 0; license_index < len; license_index++) {
    unsigned char current_char = (unsigned char)license[license_index];

    /* 检查是否为中文字符（UTF-8编码，中文字符通常以0xE开头的3字节序列） */
    if (current_char >= 0xE0) {
      /* 简单的中文字符检测 - 检查是否为UTF-8中文字符开头 */
      if (license_index + 2 < len &&
          (unsigned char)license[license_index + 1] >= 0x80 &&
          (unsigned char)license[license_index + 1] <= 0xBF &&
          (unsigned char)license[license_index + 2] >= 0x80 &&
          (unsigned char)license[license_index + 2] <= 0xBF) {
        has_chinese = 1;
        license_index += 2; /* 跳过UTF-8中文字符的后两个字节 */
      } else {
        return 0; /* 无效的UTF-8序列 */
      }
    }
    /* 检查英文字母 */
    else if ((current_char >= 'A' && current_char <= 'Z') ||
             (current_char >= 'a' && current_char <= 'z')) {
      has_letter = 1;
    }
    /* 检查数字 */
    else if (current_char >= '0' && current_char <= '9') {
      has_digit = 1;
    }
    /* 允许的特殊字符（如·用于少数民族车牌） */
    else if (current_char == 0xC2 && license_index + 1 < len &&
             (unsigned char)license[license_index + 1] == 0xB7) {
      license_index++; /* 跳过·的第二个字节 */
    } else {
      return 0; /* 不允许的字符 */
    }
  }

  /* 中国车牌号应该包含中文（省份简称）和字母/数字 */
  return has_chinese && (has_letter || has_digit);
}

/* 验证联系方式格式 */
static int validate_contact(const char *contact) {
  size_t contact_index;

  if (!contact || strlen(contact) < MIN_CONTACT_LEN ||
      strlen(contact) >= MAX_CONTACT_LEN) {
    return 0;
  }

  /* 检查是否全为数字 */
  for (contact_index = 0; contact_index < strlen(contact); contact_index++) {
    if (contact[contact_index] < '0' || contact[contact_index] > '9') {
      return 0;
    }
  }

  return 1;
}

/* 获取错误信息 */
static const char *get_error_message(ParkingServiceResult code) {
  switch (code) {
  case PARKING_SERVICE_SUCCESS:
    return "操作成功";
  case PARKING_SERVICE_INVALID_PARAM:
    return "无效参数";
  case PARKING_SERVICE_SLOT_EXISTS:
    return "车位已存在";
  case PARKING_SERVICE_SLOT_NOT_FOUND:
    return "车位不存在";
  case PARKING_SERVICE_SLOT_OCCUPIED:
    return "车位已被占用";
  case PARKING_SERVICE_SLOT_FREE:
    return "车位当前为空闲状态";
  case PARKING_SERVICE_LICENSE_EXISTS:
    return "车牌号已存在";
  case PARKING_SERVICE_TIME_INVALID:
    return "访客入场时间无效（仅限9:00-17:00）";
  case PARKING_SERVICE_MEMORY_ERROR:
    return "内存分配失败";
  case PARKING_SERVICE_FILE_ERROR:
    return "文件操作失败";
  default:
    return "未知错误";
  }
}

/* ===== 核心业务服务函数 ===== */

/* 添加停车位服务 */
ServiceResult parking_service_add_slot(ParkingLot *lot, int slot_id,
                                       const char *location) {
  ParkingSlot *slot;
  int result;

  if (!lot || !location) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, "参数不能为空",
                                 NULL);
  }

  if (!validate_slot_id(slot_id)) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM,
                                 "车位编号必须在1-99999之间", NULL);
  }

  if (strlen(location) == 0 || strlen(location) >= MAX_LOCATION_LEN) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM,
                                 "位置描述长度无效", NULL);
  }

  /* 检查车位是否已存在 */
  if (find_slot_by_id(lot, slot_id) != NULL) {
    return create_service_result(PARKING_SERVICE_SLOT_EXISTS, "车位编号已存在",
                                 NULL);
  }

  /* 创建停车位 */
  slot = create_parking_slot(slot_id, location);
  if (!slot) {
    return create_service_result(PARKING_SERVICE_MEMORY_ERROR, "创建车位失败",
                                 NULL);
  }

  /* 添加到停车场 */
  result = add_parking_slot(lot, slot);
  if (result != 0) {
    free_parking_slot(slot);
    return create_service_result(PARKING_SERVICE_MEMORY_ERROR,
                                 "添加车位到停车场失败", NULL);
  }

  return create_service_result(PARKING_SERVICE_SUCCESS, "车位添加成功", NULL);
}

/* 分配停车位服务 */
ServiceResult parking_service_allocate_slot(ParkingLot *lot, int slot_id,
                                            const char *owner_name,
                                            const char *license_plate,
                                            const char *contact,
                                            ParkingType type) {
  ParkingSlot *slot;
  int result;

  if (!lot || !owner_name || !license_plate || !contact) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, "参数不能为空",
                                 NULL);
  }

  if (!validate_slot_id(slot_id)) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, "车位编号无效",
                                 NULL);
  }

  if (!validate_license_plate(license_plate)) {
    return create_service_result(
        PARKING_SERVICE_INVALID_PARAM,
        "车牌号格式无效（支持中文车牌号，如：京A12345）", NULL);
  }

  if (!validate_contact(contact)) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM,
                                 "联系方式格式无效", NULL);
  }

  if (strlen(owner_name) == 0 || strlen(owner_name) >= MAX_NAME_LEN) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM,
                                 "车主姓名长度无效", NULL);
  }

  /* 检查车位是否存在 */
  slot = find_slot_by_id(lot, slot_id);
  if (!slot) {
    return create_service_result(PARKING_SERVICE_SLOT_NOT_FOUND, "车位不存在",
                                 NULL);
  }

  /* 检查车位是否已占用 */
  if (slot->status == OCCUPIED_STATUS) {
    return create_service_result(PARKING_SERVICE_SLOT_OCCUPIED, "车位已被占用",
                                 NULL);
  }

  /* 检查车牌号是否已存在 */
  if (find_slot_by_license(lot, license_plate) != NULL) {
    return create_service_result(PARKING_SERVICE_LICENSE_EXISTS, "车牌号已存在",
                                 NULL);
  }

  /* 分配车位 */
  result =
      allocate_slot(lot, slot_id, owner_name, license_plate, contact, type);
  switch (result) {
  case 0:
    return create_service_result(PARKING_SERVICE_SUCCESS, "车位分配成功", NULL);
  case VISITOR_TIME_ERROR_CODE:
    return create_service_result(PARKING_SERVICE_TIME_INVALID,
                                 "访客车辆只能在9:00-17:00之间入场", NULL);
  default:
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, "分配失败",
                                 NULL);
  }
}

/* 释放停车位服务 */
ServiceResult parking_service_deallocate_slot(ParkingLot *lot, int slot_id) {
  ParkingSlot *slot;
  double fee = 0.0;
  double *fee_data;
  int result;

  if (!lot) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM,
                                 "停车场参数不能为空", NULL);
  }

  if (!validate_slot_id(slot_id)) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, "车位编号无效",
                                 NULL);
  }

  /* 检查车位是否存在 */
  slot = find_slot_by_id(lot, slot_id);
  if (!slot) {
    return create_service_result(PARKING_SERVICE_SLOT_NOT_FOUND, "车位不存在",
                                 NULL);
  }

  /* 检查车位是否为空闲状态 */
  if (slot->status == FREE_STATUS) {
    return create_service_result(PARKING_SERVICE_SLOT_FREE,
                                 "车位当前为空闲状态", NULL);
  }

  /* 计算访客停车费用 */
  if (slot->type == VISITOR_TYPE) {
    time_t now = time(NULL);
    fee = calculate_visitor_fee(slot->entry_time, now);
  }

  /* 释放车位 */
  result = deallocate_slot(lot, slot_id);
  if (result != 0) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, "释放车位失败",
                                 NULL);
  }

  /* 创建包含费用信息的结果 */
  fee_data = malloc(sizeof(double));
  if (fee_data) {
    *fee_data = fee;
  }

  return create_service_result(PARKING_SERVICE_SUCCESS, "车位释放成功",
                               fee_data);
}

/* 查询停车位服务 */
ServiceResult parking_service_find_slot_by_id(ParkingLot *lot, int slot_id) {
  ParkingSlot *slot;

  if (!lot) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM,
                                 "停车场参数不能为空", NULL);
  }

  if (!validate_slot_id(slot_id)) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, "车位编号无效",
                                 NULL);
  }

  slot = find_slot_by_id(lot, slot_id);
  if (!slot) {
    return create_service_result(PARKING_SERVICE_SLOT_NOT_FOUND, "车位不存在",
                                 NULL);
  }

  return create_service_result(PARKING_SERVICE_SUCCESS, "查询成功", slot);
}

/* 根据车牌号查询 */
ServiceResult parking_service_find_slot_by_license(ParkingLot *lot,
                                                   const char *license_plate) {
  ParkingSlot *slot;

  if (!lot || !license_plate) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, "参数不能为空",
                                 NULL);
  }

  if (!validate_license_plate(license_plate)) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM,
                                 "车牌号格式无效", NULL);
  }

  slot = find_slot_by_license(lot, license_plate);
  if (!slot) {
    return create_service_result(PARKING_SERVICE_SLOT_NOT_FOUND,
                                 "未找到该车牌号对应的车位", NULL);
  }

  return create_service_result(PARKING_SERVICE_SUCCESS, "查询成功", slot);
}

/* 根据车主姓名查询 */
ServiceResult parking_service_find_slot_by_owner(ParkingLot *lot,
                                                 const char *owner_name) {
  ParkingSlot *slot;

  if (!lot || !owner_name) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, "参数不能为空",
                                 NULL);
  }

  if (strlen(owner_name) == 0) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM,
                                 "车主姓名不能为空", NULL);
  }

  slot = find_slot_by_owner(lot, owner_name);
  if (!slot) {
    return create_service_result(PARKING_SERVICE_SLOT_NOT_FOUND,
                                 "未找到该车主对应的车位", NULL);
  }

  return create_service_result(PARKING_SERVICE_SUCCESS, "查询成功", slot);
}

/* 获取车位列表服务 */
ServiceResult parking_service_get_free_slots(ParkingLot *lot) {
  int count = 0;
  ParkingSlot **slots;
  SlotQueryResult *result;

  if (!lot) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM,
                                 "停车场参数不能为空", NULL);
  }

  slots = get_free_slots(lot, &count);

  result = malloc(sizeof(SlotQueryResult));
  if (!result) {
    if (slots) {
      free(slots);
    }
    return create_service_result(PARKING_SERVICE_MEMORY_ERROR, "内存分配失败",
                                 NULL);
  }

  result->slot = (ParkingSlot *)slots; /* 存储指针数组 */
  result->total_found = count;

  return create_service_result(PARKING_SERVICE_SUCCESS, "查询成功", result);
}

ServiceResult parking_service_get_occupied_slots(ParkingLot *lot) {
  int count = 0;
  ParkingSlot **slots;
  SlotQueryResult *result;

  if (!lot) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM,
                                 "停车场参数不能为空", NULL);
  }

  slots = get_occupied_slots(lot, &count);

  result = malloc(sizeof(SlotQueryResult));
  if (!result) {
    if (slots) {
      free(slots);
    }
    return create_service_result(PARKING_SERVICE_MEMORY_ERROR, "内存分配失败",
                                 NULL);
  }

  result->slot = (ParkingSlot *)slots; /* 存储指针数组 */
  result->total_found = count;

  return create_service_result(PARKING_SERVICE_SUCCESS, "查询成功", result);
}

/* 添加缴费记录服务 */
ServiceResult parking_service_add_payment(ParkingLot *lot, int slot_id,
                                          double amount, int days) {
  ParkingSlot *slot;
  time_t start_date, end_date;
  int result;

  if (!lot) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM,
                                 "停车场参数不能为空", NULL);
  }

  if (!validate_slot_id(slot_id)) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, "车位编号无效",
                                 NULL);
  }

  if (amount <= 0) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM,
                                 "缴费金额必须大于0", NULL);
  }

  if (days <= 0 || days > MAX_PAYMENT_DAYS) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM,
                                 "缴费天数必须在1-365之间", NULL);
  }

  /* 检查车位是否存在且已占用 */
  slot = find_slot_by_id(lot, slot_id);
  if (!slot) {
    return create_service_result(PARKING_SERVICE_SLOT_NOT_FOUND, "车位不存在",
                                 NULL);
  }

  if (slot->status != OCCUPIED_STATUS || slot->type != RESIDENT_TYPE) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM,
                                 "只有居民已占用车位才能缴费", NULL);
  }

  /* 添加缴费记录 */
  start_date = time(NULL);
  end_date = start_date + (time_t)(days * HOURS_PER_DAY * SECONDS_PER_HOUR);

  result = add_payment_record(slot, start_date, end_date, amount);
  if (result != 0) {
    return create_service_result(PARKING_SERVICE_MEMORY_ERROR,
                                 "添加缴费记录失败", NULL);
  }

  return create_service_result(PARKING_SERVICE_SUCCESS, "缴费记录添加成功",
                               slot);
}

/* 获取统计信息服务 */
ServiceResult parking_service_get_statistics(ParkingLot *lot) {
  ParkingStatistics *stats;

  if (!lot) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM,
                                 "停车场参数不能为空", NULL);
  }

  stats = malloc(sizeof(ParkingStatistics));
  if (!stats) {
    return create_service_result(PARKING_SERVICE_MEMORY_ERROR, "内存分配失败",
                                 NULL);
  }

  /* 基础统计 */
  stats->total_slots = lot->total_slots;
  stats->occupied_slots = lot->occupied_slots;
  stats->free_slots = lot->total_slots - lot->occupied_slots;

  if (lot->total_slots > 0) {
    stats->occupancy_rate =
        (double)lot->occupied_slots / lot->total_slots * 100.0;
  } else {
    stats->occupancy_rate = 0.0;
  }

  /* 简化统计 - 暂时设为0避免问题 */
  stats->resident_vehicles = 0;
  stats->visitor_vehicles = 0;
  stats->month_revenue = 0.0;
  stats->today_revenue = 0.0;

  return create_service_result(PARKING_SERVICE_SUCCESS, "统计信息获取成功",
                               stats);
}

/* 数据持久化服务 */
ServiceResult parking_service_save_data(ParkingLot *lot, const char *filename) {
  int result;

  if (!lot || !filename) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, "参数不能为空",
                                 NULL);
  }

  if (strlen(filename) == 0) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM,
                                 "文件名不能为空", NULL);
  }

  result = save_parking_data(lot, filename);
  if (result != 0) {
    return create_service_result(PARKING_SERVICE_FILE_ERROR, "保存数据失败",
                                 NULL);
  }

  return create_service_result(PARKING_SERVICE_SUCCESS, "数据保存成功", NULL);
}

ServiceResult parking_service_load_data(const char *filename) {
  ParkingLot *lot;

  if (!filename) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM,
                                 "文件名不能为空", NULL);
  }

  if (strlen(filename) == 0) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM,
                                 "文件名不能为空", NULL);
  }

  lot = load_parking_data(filename);
  if (!lot) {
    return create_service_result(PARKING_SERVICE_FILE_ERROR, "加载数据失败",
                                 NULL);
  }

  return create_service_result(PARKING_SERVICE_SUCCESS, "数据加载成功", lot);
}

/* ===== 公共辅助函数 ===== */

/* 释放服务结果数据 */
void parking_service_free_result(ServiceResult *result) {
  if (result && result->data) {
    free(result->data);
    result->data = NULL;
  }
}

/* 打印错误信息 */
void parking_service_print_error(ServiceResult result) {
  if (result.code != PARKING_SERVICE_SUCCESS) {
    printf("错误：%s\n",
           result.message[0] ? result.message : get_error_message(result.code));
  }
}

/* 检查结果是否成功 */
int parking_service_is_success(ServiceResult result) {
  return result.code == PARKING_SERVICE_SUCCESS;
}