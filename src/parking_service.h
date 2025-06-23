#ifndef PARKING_SERVICE_H
#define PARKING_SERVICE_H

#include "parking_data.h"

/* 操作结果状态码 */
typedef enum {
  PARKING_SERVICE_SUCCESS = 0,         /* 操作成功 */
  PARKING_SERVICE_INVALID_PARAM = -1,  /* 无效参数 */
  PARKING_SERVICE_SLOT_EXISTS = -2,    /* 车位已存在 */
  PARKING_SERVICE_SLOT_NOT_FOUND = -3, /* 车位不存在 */
  PARKING_SERVICE_SLOT_OCCUPIED = -4,  /* 车位已占用 */
  PARKING_SERVICE_SLOT_FREE = -5,      /* 车位空闲 */
  PARKING_SERVICE_LICENSE_EXISTS = -6, /* 车牌号已存在 */
  PARKING_SERVICE_TIME_INVALID = -7,   /* 时间无效 */
  PARKING_SERVICE_MEMORY_ERROR = -8,   /* 内存错误 */
  PARKING_SERVICE_FILE_ERROR = -9      /* 文件操作错误 */
} ParkingServiceResult;

/* 操作结果结构体 */
typedef struct {
  ParkingServiceResult code;
  char message[256];
  void *data;
} ServiceResult;

/* 车位查询结果 */
typedef struct {
  ParkingSlot *slot;
  int total_found;
} SlotQueryResult;

/* 统计信息结构体 */
typedef struct {
  int total_slots;
  int occupied_slots;
  int free_slots;
  int resident_vehicles;
  int visitor_vehicles;
  double occupancy_rate;
  double today_revenue;
  double month_revenue;
} ParkingStatistics;

/* ===== 核心业务服务函数声明 ===== */

/* 添加停车位服务 */
ServiceResult parking_service_add_slot(ParkingLot *lot, int slot_id,
                                       const char *location);

/* 分配停车位服务 */
ServiceResult parking_service_allocate_slot(ParkingLot *lot, int slot_id,
                                            const char *owner_name,
                                            const char *license_plate,
                                            const char *contact,
                                            ParkingType type);

/* 释放停车位服务 */
ServiceResult parking_service_deallocate_slot(ParkingLot *lot, int slot_id);

/* 查询停车位服务 */
ServiceResult parking_service_find_slot_by_id(ParkingLot *lot, int slot_id);
ServiceResult parking_service_find_slot_by_license(ParkingLot *lot,
                                                   const char *license_plate);
ServiceResult parking_service_find_slot_by_owner(ParkingLot *lot,
                                                 const char *owner_name);

/* 获取车位列表服务 */
ServiceResult parking_service_get_free_slots(ParkingLot *lot);
ServiceResult parking_service_get_occupied_slots(ParkingLot *lot);

/* 添加缴费记录服务 */
ServiceResult parking_service_add_payment(ParkingLot *lot, int slot_id,
                                          double amount, int days);

/* 获取统计信息服务 */
ServiceResult parking_service_get_statistics(ParkingLot *lot);

/* 数据持久化服务 */
ServiceResult parking_service_save_data(ParkingLot *lot, const char *filename);
ServiceResult parking_service_load_data(const char *filename);

/* ===== 公共辅助函数声明 ===== */

/* 释放服务结果数据 */
void parking_service_free_result(ServiceResult *result);

/* 打印错误信息 */
void parking_service_print_error(ServiceResult result);

/* 检查结果是否成功 */
int parking_service_is_success(ServiceResult result);

#endif /* PARKING_SERVICE_H */
