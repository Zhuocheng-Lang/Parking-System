#ifndef PARKING_SERVICE_H
#define PARKING_SERVICE_H

#include "parking_data.h"

/**
 * @file parking_service.h
 * @brief 服务层接口声明文件
 * @details
 * 定义了停车场管理系统的核心业务逻辑接口。
 * 服务层封装了数据层的具体操作，为UI层提供统一、简洁的调用接口。
 * 所有服务函数都返回一个 ServiceResult 结构体，用于表示操作结果和返回数据。
 */

/**
 *********************************************************************************
 *                                 枚举和结构体定义
 *********************************************************************************
 */

/**
 * @brief 服务层操作结果的状态码
 */
typedef enum {
  PARKING_SERVICE_SUCCESS = 0,         /**< 操作成功 */
  PARKING_SERVICE_INVALID_PARAM = -1,  /**< 无效参数（如NULL指针、非法值） */
  PARKING_SERVICE_SLOT_EXISTS = -2,    /**< 尝试添加已存在的车位 */
  PARKING_SERVICE_SLOT_NOT_FOUND = -3, /**< 指定的车位不存在 */
  PARKING_SERVICE_SLOT_OCCUPIED = -4,  /**< 车位已被占用 */
  PARKING_SERVICE_SLOT_FREE = -5,      /**< 车位当前为空闲状态 */
  PARKING_SERVICE_LICENSE_EXISTS = -6, /**< 车牌号已在场内 */
  PARKING_SERVICE_TIME_INVALID = -7,   /**< 访客入场时间不合规 */
  PARKING_SERVICE_MEMORY_ERROR = -8,   /**< 内存分配失败 */
  PARKING_SERVICE_FILE_ERROR = -9,     /**< 文件读写操作错误 */
  PARKING_SERVICE_SYSTEM_ERROR = -10   /**< 其他系统级错误 */
} ParkingServiceResultCode;

/**
 * @brief 服务层函数的统一返回结构体
 * @details 封装了操作的结果码、可读的消息以及一个通用的数据指针。
 */
typedef struct {
  ParkingServiceResultCode code; /**< 操作结果的状态码 */
  char message[256];             /**< 描述操作结果的可读消息 */
  void *data;                    /**< 指向返回数据的通用指针。
                                  * @note 成功时，可能指向单个对象（如ParkingSlot）或
                                  *       动态分配的结构体（如ParkingStatistics）。
                                  *       调用者需要根据函数文档进行类型转换。
                                  *       对于动态分配的数据，调用者有责任在用完后
                                  *       调用 parking_service_free_result() 或手动 free() 来释放。
                                  */
} ServiceResult;

/**
 * @brief 车位列表查询的结果结构体
 * @details 当查询函数返回多个车位时，此结构体作为 ServiceResult.data 的内容。
 */
typedef struct {
  ParkingSlot **slot_list; /**< 指向 ParkingSlot 指针数组的指针 */
  int total_found;         /**< 数组中找到的车位总数 */
} SlotQueryResult;

/**
 * @brief 停车场统计信息结构体
 * @details 用于封装从服务层获取的各类统计数据。
 */
typedef struct ParkingStatistics {
  int total_slots;       /**< 总车位数 */
  int occupied_slots;    /**< 已占用车位数 */
  int free_slots;        /**< 空闲车位数 */
  double occupancy_rate; /**< 车位使用率 (%) */
  double today_revenue;  /**< 当日总收入 */
  double month_revenue;  /**< 当月总收入 */
} ParkingStatistics;

/**
 *********************************************************************************
 *                            核心业务服务API声明
 *********************************************************************************
 */

/** @name 车位管理服务 */
///@{

/**
 * @brief 添加一个新的停车位。
 * @param lot 目标停车场。
 * @param slot_id 要添加的车位编号。
 * @param location 车位的位置描述。
 * @return 返回一个 ServiceResult 结构体，表示操作结果。
 */
ServiceResult parking_service_add_slot(ParkingLot *lot, int slot_id,
                                       const char *location);

///@}

/** @name 车辆出入场服务 */
///@{

/**
 * @brief 为车辆分配一个停车位（车辆入场）。
 * @param lot 目标停车场。
 * @param slot_id 要分配的车位编号。
 * @param owner_name 车主姓名。
 * @param license_plate 车牌号。
 * @param contact 联系方式。
 * @param type 停车类型 (居民/访客)。
 * @return 返回一个 ServiceResult 结构体，表示操作结果。
 */
ServiceResult parking_service_allocate_slot(ParkingLot *lot, int slot_id,
                                            const char *owner_name,
                                            const char *license_plate,
                                            const char *contact,
                                            ParkingType type);

/**
 * @brief 释放一个停车位（车辆出场），并计算费用。
 * @param lot 目标停车场。
 * @param slot_id 要释放的车位编号。
 * @return 返回一个 ServiceResult 结构体。
 *         成功时，其 data 字段可能指向一个 double 类型的费用值，使用后需释放。
 */
ServiceResult parking_service_deallocate_slot(ParkingLot *lot, int slot_id);

///@}

/** @name 查询服务 */
///@{

/**
 * @brief 根据车位编号查找停车位。
 * @param lot 目标停车场。
 * @param slot_id 要查找的车位编号。
 * @return 返回一个 ServiceResult 结构体。
 *         成功时，其 data 字段指向找到的 ParkingSlot 对象（无需释放）。
 */
ServiceResult parking_service_find_slot_by_id(ParkingLot *lot, int slot_id);

/**
 * @brief 根据车牌号查找停车位。
 * @param lot 目标停车场。
 * @param license_plate 要查找的车牌号。
 * @return 返回一个 ServiceResult 结构体。
 *         成功时，其 data 字段指向找到的 ParkingSlot 对象（无需释放）。
 */
ServiceResult parking_service_find_slot_by_license(ParkingLot *lot,
                                                   const char *license_plate);

/**
 * @brief 根据车主姓名（模糊）查找停车位。
 * @param lot 目标停车场。
 * @param owner_name 要查找的车主姓名。
 * @return 返回一个 ServiceResult 结构体。
 *         成功时，其 data 字段指向找到的 ParkingSlot 对象（无需释放）。
 */
ServiceResult parking_service_find_slot_by_owner(ParkingLot *lot,
                                                 const char *owner_name);

///@}

/** @name 列表查询服务 */
///@{

/**
 * @brief 获取所有空闲车位的列表。
 * @param lot 目标停车场。
 * @return 返回一个 ServiceResult 结构体。
 *         成功时，其 data 字段指向一个 SlotQueryResult 结构体，使用后需释放。
 */
ServiceResult parking_service_get_free_slots(ParkingLot *lot);

/**
 * @brief 获取所有已占用车位的列表。
 * @param lot 目标停车场。
 * @return 返回一个 ServiceResult 结构体。
 *         成功时，其 data 字段指向一个 SlotQueryResult 结构体，使用后需释放。
 */
ServiceResult parking_service_get_occupied_slots(ParkingLot *lot);

/**
 * @brief 获取所有车位的列表。
 * @param lot 目标停车场。
 * @return 返回一个 ServiceResult 结构体。
 *         成功时，其 data 字段指向一个 SlotQueryResult 结构体，使用后需释放。
 */
ServiceResult parking_service_get_all_slots(ParkingLot *lot);

///@}

/** @name 统计分析服务 */
///@{

/**
 * @brief 获取停车场的整体统计信息。
 * @param lot 目标停车场。
 * @return 返回一个 ServiceResult 结构体。
 *         成功时，其 data 字段指向一个 ParkingStatistics 结构体，使用后需释放。
 */
ServiceResult parking_service_get_statistics(ParkingLot *lot);

///@}

/**
 *********************************************************************************
 *                            数据持久化服务API声明
 *********************************************************************************
 */

/** @name 数据持久化服务 */
///@{

/**
 * @brief 将停车场数据保存到文件。
 * @param lot 要保存的停车场。
 * @param filename 目标文件名。
 * @return 返回一个 ServiceResult 结构体，表示操作结果。
 */
ServiceResult parking_service_save_data(ParkingLot *lot, const char *filename);

/**
 * @brief 从文件加载停车场数据。
 * @param filename 源文件名。
 * @return 返回一个 ServiceResult 结构体。
 *         成功时，其 data 字段指向新创建的 ParkingLot 对象，
 *         调用者负责在后续操作中管理其生命周期，并在程序结束时释放。
 */
ServiceResult parking_service_load_data(const char *filename);

///@}

/**
 *********************************************************************************
 *                                公共辅助函数声明
 *********************************************************************************
 */

/** @name 公共辅助函数 */
///@{

/**
 * @brief 释放由服务层函数动态分配在 ServiceResult.data 中的内存。
 * @note 对于返回指向现有数据（如 find_slot_by_id）的结果，不应使用此函数。
 *       通常用于释放 get_statistics, deallocate_slot 等函数返回的数据。
 * @param result 指向要清理的 ServiceResult 的指针。
 */
void parking_service_free_result(ServiceResult *result);

/**
 * @brief 打印服务层返回的错误信息到标准错误输出。
 * @param result 包含错误信息的操作结果。
 */
void parking_service_print_error(ServiceResult result);

/**
 * @brief 检查一个 ServiceResult 是否表示操作成功。
 * @param result 要检查的操作结果。
 * @return 如果操作成功，返回1；否则返回0。
 */
int parking_service_is_success(ServiceResult result);

///@}

#endif /* PARKING_SERVICE_H */
