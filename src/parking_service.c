/**
 * @file parking_service.c
 * @brief 服务层实现文件
 * @details
 * 该文件实现了在 parking_service.h 中声明的所有业务逻辑函数。
 * 服务层作为UI层和数据层之间的桥梁，封装了核心业务规则，
 * 如参数验证、费用计算、状态转换等，为上层提供统一、简洁的接口。
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "parking_service.h"

#ifdef _WIN32
#include <locale.h>
#include <windows.h>
#endif

/* ========================================================================== */
/*                                 内部常量定义                               */
/* ========================================================================== */

#define MAX_SLOT_ID 99999 /**< 允许的最大车位ID */
#define MIN_LICENSE_LEN 5 /**< 车牌号最小长度 */
#define MIN_CONTACT_LEN 8 /**< 联系方式最小长度 */
#define SECONDS_PER_MONTH                                                      \
  (30 * 24 * 3600) /**< 用于计算月费的秒数（按30天计） */

/* ========================================================================== */
/*                                内部辅助函数声明 */
/* ========================================================================== */

static ServiceResult create_service_result(ParkingServiceResultCode code,
                                           const char *message, void *data);
static int validate_slot_id(int slot_id);
static int validate_license_plate(const char *license);
static int validate_contact(const char *contact);
static const char *get_error_message(ParkingServiceResultCode code);
static void update_revenue_cycle(ParkingLot *lot);

/* ========================================================================== */
/*                                内部辅助函数实现 */
/* ========================================================================== */

/**
 * @brief 创建一个 ServiceResult 结构体实例。
 * @param code 操作结果的状态码。
 * @param message 描述结果的可读消息。
 * @param data 指向返回数据的通用指针。
 * @return 初始化后的 ServiceResult 结构体。
 */
static ServiceResult create_service_result(ParkingServiceResultCode code,
                                           const char *message, void *data) {
  ServiceResult result;
  result.code = code;
  result.data = data;

  if (message) {
    strncpy(result.message, message, sizeof(result.message) - 1);
    result.message[sizeof(result.message) - 1] = '\0';
  } else {
    strncpy(result.message, get_error_message(code),
            sizeof(result.message) - 1);
    result.message[sizeof(result.message) - 1] = '\0';
  }

  return result;
}

/**
 * @brief 验证车位ID是否在有效范围内。
 * @param slot_id 要验证的车位ID。
 * @return 如果有效返回1，否则返回0。
 */
static int validate_slot_id(int slot_id) {
  return (slot_id > 0 && slot_id <= MAX_SLOT_ID);
}

/**
 * @brief 验证车牌号格式是否基本合规。
 * @details 简单检查长度，并确认包含中文字符和字母/数字。
 * @param license 要验证的车牌号字符串。
 * @return 如果有效返回1，否则返回0。
 */
static int validate_license_plate(const char *license) {
  if (!license || strlen(license) < MIN_LICENSE_LEN ||
      strlen(license) >= MAX_LICENSE_LEN) {
    return 0;
  }
  /* TODO: 更完备的验证 */
  return 1;
}

/**
 * @brief 验证联系方式是否为纯数字且长度合规。
 * @param contact 要验证的联系方式字符串。
 * @return 如果有效返回1，否则返回0。
 */
static int validate_contact(const char *contact) {
  size_t i;
  if (!contact || strlen(contact) < MIN_CONTACT_LEN ||
      strlen(contact) >= MAX_CONTACT_LEN) {
    return 0;
  }
  for (i = 0; i < strlen(contact); i++) {
    if (contact[i] < '0' || contact[i] > '9') {
      return 0;
    }
  }
  return 1;
}

/**
 * @brief 根据错误码获取对应的默认错误消息。
 * @param code 错误码。
 * @return 指向错误消息字符串的指针。
 */
static const char *get_error_message(ParkingServiceResultCode code) {
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
    return "该车牌号已在场内";
  case PARKING_SERVICE_TIME_INVALID:
    return "访客入场时间不合规";
  case PARKING_SERVICE_MEMORY_ERROR:
    return "内存分配失败";
  case PARKING_SERVICE_FILE_ERROR:
    return "文件读写操作错误";
  case PARKING_SERVICE_SYSTEM_ERROR:
    return "其他系统级错误";
  default:
    return "未知错误";
  }
}

/**
 * @brief 检查并根据需要更新停车场的收入统计周期。
 * @details 如果当前时间进入了新的月份或新的一天，则重置相应的收入计数器。
 * @param lot 目标停车场。
 */
static void update_revenue_cycle(ParkingLot *lot) {
  if (lot == NULL) {
    return;
  }

  time_t now = time(NULL);
  if (lot->last_update_time == 0) {
    lot->last_update_time = now;
    return;
  }

  struct tm *tm_now = localtime(&now);
  struct tm *tm_last = localtime(&lot->last_update_time);

  if (tm_now->tm_year > tm_last->tm_year || tm_now->tm_mon > tm_last->tm_mon) {
    lot->month_revenue = 0.0;
    lot->today_revenue = 0.0;
  } else if (tm_now->tm_mday > tm_last->tm_mday) {
    lot->today_revenue = 0.0;
  }

  lot->last_update_time = now;
}

/* ========================================================================== */
/*                            核心业务服务函数实现                            */
/* ========================================================================== */

/**
 * @brief 添加一个新的停车位。
 * @details 验证参数后，创建一个新的停车位并将其添加到停车场。
 * @param lot 目标停车场。
 * @param slot_id 新车位的ID。
 * @param location 新车位的位置描述。
 * @return 返回一个 ServiceResult 结构，包含操作结果。
 */
ServiceResult parking_service_add_slot(ParkingLot *lot, int slot_id,
                                       const char *location) {
  ParkingSlot *slot;

  if (!lot || !location || !validate_slot_id(slot_id) ||
      strlen(location) == 0) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, NULL, NULL);
  }

  if (find_slot_by_id(lot, slot_id) != NULL) {
    return create_service_result(PARKING_SERVICE_SLOT_EXISTS, NULL, NULL);
  }

  slot = create_parking_slot(slot_id, location);
  if (!slot) {
    return create_service_result(PARKING_SERVICE_MEMORY_ERROR, NULL, NULL);
  }

  if (add_parking_slot(lot, slot) != 0) {
    free_parking_slot(slot);
    return create_service_result(PARKING_SERVICE_SYSTEM_ERROR,
                                 "添加车位到链表失败", NULL);
  }

  return create_service_result(PARKING_SERVICE_SUCCESS, "车位添加成功", NULL);
}

/**
 * @brief 为指定车位分配车辆（车辆入场）。
 * @details 验证参数后，调用数据层函数为车位分配车辆信息。
 *          处理并返回数据层可能出现的各种错误。
 * @param lot 目标停车场。
 * @param slot_id 要分配的车位ID。
 * @param owner_name 车主姓名。
 * @param license_plate 车牌号。
 * @param contact 联系方式。
 * @param type 停车类型 (居民/访客)。
 * @return 返回一个 ServiceResult 结构，包含操作结果。
 */
ServiceResult parking_service_allocate_slot(ParkingLot *lot, int slot_id,
                                            const char *owner_name,
                                            const char *license_plate,
                                            const char *contact,
                                            ParkingType type) {
  int data_result;

  if (!lot || !owner_name || !license_plate || !contact ||
      !validate_slot_id(slot_id) || !validate_license_plate(license_plate) ||
      !validate_contact(contact)) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, NULL, NULL);
  }

  data_result =
      allocate_slot(lot, slot_id, owner_name, license_plate, contact, type);

  switch (data_result) {
  case 0:
    return create_service_result(PARKING_SERVICE_SUCCESS, "车位分配成功", NULL);
  case -2:
    return create_service_result(PARKING_SERVICE_SLOT_NOT_FOUND, NULL, NULL);
  case -3:
    return create_service_result(PARKING_SERVICE_SLOT_OCCUPIED, NULL, NULL);
  case -4:
    return create_service_result(PARKING_SERVICE_LICENSE_EXISTS, NULL, NULL);
  case -5:
    return create_service_result(PARKING_SERVICE_TIME_INVALID, NULL, NULL);
  default:
    return create_service_result(PARKING_SERVICE_SYSTEM_ERROR,
                                 "未知的数据层错误", NULL);
  }
}

/**
 * @brief 释放一个停车位（车辆出场），并计算费用。
 * @details 验证车位存在且被占用。根据停车类型（居民/访客）计算停车费用，
 *          更新收入统计，然后调用数据层函数释放车位。
 * @param lot 目标停车场。
 * @param slot_id 要释放的车位ID。
 * @return 返回一个 ServiceResult 结构。成功时，如果产生费用，
 *         其 data 字段会指向一个包含费用值的 double 类型指针。
 */
ServiceResult parking_service_deallocate_slot(ParkingLot *lot, int slot_id) {
  ParkingSlot *slot;
  double fee = 0.0;
  double *fee_data;
  time_t now = time(NULL);

  if (!lot || !validate_slot_id(slot_id)) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, NULL, NULL);
  }

  slot = find_slot_by_id(lot, slot_id);
  if (!slot) {
    return create_service_result(PARKING_SERVICE_SLOT_NOT_FOUND, NULL, NULL);
  }
  if (slot->status == FREE_STATUS) {
    return create_service_result(PARKING_SERVICE_SLOT_FREE, NULL, NULL);
  }

  if (slot->type == RESIDENT_TYPE) {
    if (slot->resident_due_date > 0 && now > slot->resident_due_date) {
      int overdue_months =
          (int)ceil(difftime(now, slot->resident_due_date) / SECONDS_PER_MONTH);
      fee = overdue_months * RESIDENT_MONTHLY_FEE;
      slot->resident_due_date += (time_t)overdue_months * SECONDS_PER_MONTH;
    }
  } else {
    fee = calculate_visitor_fee(slot->entry_time, now);
  }

  if (fee > 0.0) {
    update_revenue_cycle(lot);
    lot->today_revenue += fee;
    lot->month_revenue += fee;
  }

  int data_result = deallocate_slot(lot, slot_id);
  if (data_result != 0) {
    // 假设数据层返回非0值表示错误，将其映射到服务层错误码
    return create_service_result(PARKING_SERVICE_SYSTEM_ERROR,
                                 "数据层释放车位失败", NULL);
  }

  if (fee > 0) {
    double *fee_ptr = (double *)malloc(sizeof(double));
    if (!fee_ptr) {
      return create_service_result(PARKING_SERVICE_MEMORY_ERROR, NULL, NULL);
    }
    *fee_ptr = fee;
    return create_service_result(PARKING_SERVICE_SUCCESS,
                                 "车辆出场成功，请缴费", fee_ptr);
  }
  return create_service_result(PARKING_SERVICE_SUCCESS,
                               "车辆出场成功，无费用产生", NULL);
}

/**
 * @brief 根据车位ID查找停车位。
 * @param lot 目标停车场。
 * @param slot_id 要查找的车位ID。
 * @return 返回一个 ServiceResult 结构。成功时，其 data 字段指向找到的
 * ParkingSlot 对象。
 */
ServiceResult parking_service_find_slot_by_id(ParkingLot *lot, int slot_id) {
  ParkingSlot *slot;
  if (!lot || !validate_slot_id(slot_id)) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, NULL, NULL);
  }
  slot = find_slot_by_id(lot, slot_id);
  if (!slot) {
    return create_service_result(PARKING_SERVICE_SLOT_NOT_FOUND, NULL, NULL);
  }
  return create_service_result(PARKING_SERVICE_SUCCESS, "查询成功", slot);
}

/**
 * @brief 根据车牌号查找停车位。
 * @param lot 目标停车场。
 * @param license_plate 要查找的车牌号。
 * @return 返回一个 ServiceResult 结构。成功时，其 data 字段指向找到的
 * ParkingSlot 对象。
 */
ServiceResult parking_service_find_slot_by_license(ParkingLot *lot,
                                                   const char *license_plate) {
  ParkingSlot *slot;
  if (!lot || !validate_license_plate(license_plate)) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, NULL, NULL);
  }
  slot = find_slot_by_license(lot, license_plate);
  if (!slot) {
    return create_service_result(PARKING_SERVICE_SLOT_NOT_FOUND, NULL, NULL);
  }
  return create_service_result(PARKING_SERVICE_SUCCESS, "查询成功", slot);
}

/**
 * @brief 根据车主姓名查找停车位。
 * @param lot 目标停车场。
 * @param owner_name 要查找的车主姓名。
 * @return 返回一个 ServiceResult 结构。成功时，其 data 字段指向找到的
 * ParkingSlot 对象。
 */
ServiceResult parking_service_find_slot_by_owner(ParkingLot *lot,
                                                 const char *owner_name) {
  ParkingSlot *slot;
  if (!lot || !owner_name || strlen(owner_name) == 0) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, NULL, NULL);
  }
  slot = find_slot_by_owner(lot, owner_name);
  if (!slot) {
    return create_service_result(PARKING_SERVICE_SLOT_NOT_FOUND, NULL, NULL);
  }
  return create_service_result(PARKING_SERVICE_SUCCESS, "查询成功", slot);
}

/**
 * @brief 获取所有空闲车位的列表。
 * @param lot 目标停车场。
 * @return 返回一个 ServiceResult 结构。成功时，其 data 字段指向一个
 * SlotQueryResult 对象， 其中包含了空闲车位的列表和数量。
 */
ServiceResult parking_service_get_free_slots(ParkingLot *lot) {
  int count = 0;
  ParkingSlot **slots;
  SlotQueryResult *result_data;

  if (!lot) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, NULL, NULL);
  }

  slots = get_free_slots(lot, &count);
  result_data = (SlotQueryResult *)malloc(sizeof(SlotQueryResult));
  if (!result_data) {
    free(slots); /* 如果 slots 不为 NULL */
    return create_service_result(PARKING_SERVICE_MEMORY_ERROR, NULL, NULL);
  }
  result_data->slot_list = slots;
  result_data->total_found = count;

  return create_service_result(PARKING_SERVICE_SUCCESS, "获取空闲车位列表成功",
                               result_data);
}

/**
 * @brief 获取所有已占用车位的列表。
 * @param lot 目标停车场。
 * @return 返回一个 ServiceResult 结构。成功时，其 data 字段指向一个
 * SlotQueryResult 对象， 其中包含了已占用车位的列表和数量。
 */
ServiceResult parking_service_get_occupied_slots(ParkingLot *lot) {
  int count = 0;
  ParkingSlot **slots;
  SlotQueryResult *result_data;

  if (!lot) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, NULL, NULL);
  }

  slots = get_occupied_slots(lot, &count);
  result_data = (SlotQueryResult *)malloc(sizeof(SlotQueryResult));
  if (!result_data) {
    free(slots); /* 如果slots不为NULL */
    return create_service_result(PARKING_SERVICE_MEMORY_ERROR, NULL, NULL);
  }
  result_data->slot_list = slots;
  result_data->total_found = count;

  return create_service_result(PARKING_SERVICE_SUCCESS,
                               "获取已占用车位列表成功", result_data);
}

/**
 * @brief 获取停车场中所有车位的列表。
 * @details 手动遍历停车场链表来构建车位列表，而不是调用数据层函数。
 * @param lot 目标停车场。
 * @return 返回一个 ServiceResult 结构。成功时，其 data 字段指向一个
 * SlotQueryResult 对象， 其中包含了所有车位的列表和数量。
 */
ServiceResult parking_service_get_all_slots(ParkingLot *lot) {
  int count = 0;
  ParkingSlot *current;
  ParkingSlot **slots;
  SlotQueryResult *result_data;

  if (!lot) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, NULL, NULL);
  }

  /* 第一次遍历：计算总数 */
  current = lot->slot_head;
  while (current != NULL) {
    count++;
    current = current->next;
  }

  if (count == 0) {
    /* 处理停车场为空的情况 */
    result_data = (SlotQueryResult *)calloc(1, sizeof(SlotQueryResult));
    if (!result_data) {
      return create_service_result(PARKING_SERVICE_MEMORY_ERROR, NULL, NULL);
    }
    return create_service_result(PARKING_SERVICE_SUCCESS,
                                 "获取所有车位列表成功", result_data);
  }

  /* 分配指针数组 */
  slots = (ParkingSlot **)malloc(sizeof(ParkingSlot *) * count);
  if (!slots) {
    return create_service_result(PARKING_SERVICE_MEMORY_ERROR, NULL, NULL);
  }

  /* 第二次遍历：填充指针 */
  int i = 0;
  current = lot->slot_head;
  while (current != NULL) {
    slots[i++] = current;
    current = current->next;
  }

  /* 创建并填充结果结构体 */
  result_data = (SlotQueryResult *)malloc(sizeof(SlotQueryResult));
  if (!result_data) {
    free(slots);
    return create_service_result(PARKING_SERVICE_MEMORY_ERROR, NULL, NULL);
  }
  result_data->slot_list = slots;
  result_data->total_found = count;

  return create_service_result(PARKING_SERVICE_SUCCESS, "获取所有车位列表成功",
                               result_data);
}

/**
 * @brief 获取停车场的统计信息。
 * @details 在获取信息前，会先调用 update_revenue_cycle
 * 确保收入数据是基于当前时间的。 然后将停车场的统计数据打包成 ParkingStatistics
 * 结构体返回。
 * @param lot 目标停车场。
 * @return 返回一个 ServiceResult 结构。成功时，其 data 字段指向一个
 * ParkingStatistics 对象。
 */
ServiceResult parking_service_get_statistics(ParkingLot *lot) {
  ParkingStatistics *stats;

  if (!lot) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, NULL, NULL);
  }

  update_revenue_cycle(lot);

  stats = (ParkingStatistics *)malloc(sizeof(ParkingStatistics));
  if (!stats) {
    return create_service_result(PARKING_SERVICE_MEMORY_ERROR, NULL, NULL);
  }

  stats->total_slots = lot->total_slots;
  stats->occupied_slots = lot->occupied_slots;
  stats->free_slots = lot->total_slots - lot->occupied_slots;
  stats->occupancy_rate =
      (lot->total_slots > 0)
          ? ((double)lot->occupied_slots / lot->total_slots) * 100.0
          : 0.0;
  stats->today_revenue = lot->today_revenue;
  stats->month_revenue = lot->month_revenue;

  return create_service_result(PARKING_SERVICE_SUCCESS, "获取统计信息成功",
                               stats);
}

/* ========================================================================== */
/*                            数据持久化服务函数实现                          */
/* ========================================================================== */

/**
 * @brief 将停车场数据保存到文件。
 * @details 验证参数后，调用数据层的 save_parking_data
 * 函数执行实际的文件写入操作。
 * @param lot 要保存的停车场对象。
 * @param filename 目标文件的路径。
 * @return 返回一个 ServiceResult 结构，指示操作是否成功。
 */
ServiceResult parking_service_save_data(ParkingLot *lot, const char *filename) {
  if (!lot || !filename || strlen(filename) == 0) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, NULL, NULL);
  }

  if (save_parking_data(lot, filename) != 0) {
    return create_service_result(PARKING_SERVICE_FILE_ERROR, NULL, NULL);
  }

  return create_service_result(PARKING_SERVICE_SUCCESS, "数据保存成功", NULL);
}

/**
 * @brief 从文件加载停车场数据。
 * @details 验证参数后，调用数据层的 load_parking_data
 * 函数执行实际的文件读取和解析。
 * @param filename 源文件的路径。
 * @return 返回一个 ServiceResult 结构。成功时，其 data 字段指向新创建的
 * ParkingLot 对象。
 */
ServiceResult parking_service_load_data(const char *filename) {
  ParkingLot *lot;

  if (!filename || strlen(filename) == 0) {
    return create_service_result(PARKING_SERVICE_INVALID_PARAM, NULL, NULL);
  }

  lot = load_parking_data(filename);
  if (!lot) {
    return create_service_result(PARKING_SERVICE_FILE_ERROR,
                                 "从文件加载数据失败", NULL);
  }

  return create_service_result(PARKING_SERVICE_SUCCESS, "数据加载成功", lot);
}

/* ========================================================================== */
/*                                公共辅助函数实现 */
/* ========================================================================== */

/**
 * @brief 释放由服务层函数返回的 ServiceResult 中动态分配的内存。
 * @details
 * 特别处理了返回列表（SlotQueryResult）的情况，会先释放内部的指针数组，
 * 然后再释放整个 data 指针。
 * @param result 指向要释放内存的 ServiceResult 结构的指针。
 */
void parking_service_free_result(ServiceResult *result) {
  if (result && result->data) {
    /* 对于返回列表的函数，需要额外释放内部的指针数组 */
    if (result->code == PARKING_SERVICE_SUCCESS &&
        (strstr(result->message, "列表") != NULL)) {
      SlotQueryResult *query_result = (SlotQueryResult *)result->data;
      if (query_result && query_result->slot_list) {
        free(query_result->slot_list);
      }
    }
    free(result->data);
    result->data = NULL;
  }
}

/**
 * @brief 如果 ServiceResult 表示一个错误，则打印错误消息。
 * @param result 要检查的 ServiceResult 结构。
 */
void parking_service_print_error(ServiceResult result) {
  if (result.code != PARKING_SERVICE_SUCCESS) {
    printf("[服务层错误] %s\n", result.message);
  }
}

/**
 * @brief 检查 ServiceResult 是否表示操作成功。
 * @param result 要检查的 ServiceResult 结构。
 * @return 如果操作成功返回 1，否则返回 0。
 */
int parking_service_is_success(ServiceResult result) {
  return result.code == PARKING_SERVICE_SUCCESS;
}
