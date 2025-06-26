#ifndef PARKING_DATA_H
#define PARKING_DATA_H

#include <time.h>

/**
 * @file parking_data.h
 * @brief 定义停车场管理系统的核心数据结构和数据层API。
 * @details
 * 该头文件包含了停车场、停车位、用户类型等关键实体的定义，
 * 并声明了用于创建、查询、修改和持久化这些数据的所有函数原型。
 */

/**
 *********************************************************************************
 *                                 常量定义
 *********************************************************************************
 */

#define MAX_LOCATION_LEN 100 /**< 车位位置描述的最大长度 */
#define MAX_NAME_LEN 50      /**< 车主姓名的最大长度 */
#define MAX_LICENSE_LEN 50   /**< 车牌号的最大长度 */
#define MAX_CONTACT_LEN 50   /**< 联系方式的最大长度 */

#define RESIDENT_MONTHLY_FEE 200.0 /**< 居民用户的月度固定费用（元） */
#define VISITOR_HOURLY_FEE 10.0    /**< 访客用户的单位小时费率（元/小时） */
#define VISITOR_START_HOUR 9       /**< 访客允许入场的最早小时（24小时制） */
#define VISITOR_END_HOUR 17        /**< 访客允许入场的最晚小时（24小时制） */

/**
 *********************************************************************************
 *                                 枚举类型
 *********************************************************************************
 */

/**
 * @brief 定义停车用户的类型。
 */
typedef enum {
  RESIDENT_TYPE = 0, /**< 居民用户，通常享受月度计费策略。 */
  VISITOR_TYPE = 1   /**< 访客用户，通常按小时计费。 */
} ParkingType;

/**
 * @brief 定义停车位的占用状态。
 */
typedef enum {
  FREE_STATUS = 0,    /**< 车位当前为空闲，可供分配。 */
  OCCUPIED_STATUS = 1 /**< 车位当前已被占用。 */
} ParkingStatus;

/**
 *********************************************************************************
 *                                 结构体定义
 *********************************************************************************
 */

/**
 * @brief 描述一个停车位的完整信息。
 * @details 此结构体是系统中管理车位信息的基本单元，通过单向链表组织。
 */
typedef struct ParkingSlot {
  int slot_id;                     /**< 车位的唯一数字标识符。 */
  char location[MAX_LOCATION_LEN]; /**< 车位的物理位置描述字符串 (例如 "A-01")。
                                    */
  char owner_name[MAX_NAME_LEN];       /**< 当前占用该车位的车主姓名。 */
  char license_plate[MAX_LICENSE_LEN]; /**< 当前停放车辆的车牌号码。 */
  char contact[MAX_CONTACT_LEN];       /**< 车主的联系方式（电话号码等）。 */
  ParkingType type;                    /**< 停车类型，区分居民或访客。 */
  time_t entry_time;                   /**< 车辆入场的时间戳。 */
  time_t exit_time;                    /**< 车辆出场的时间戳。 */
  time_t resident_due_date; /**< 若为居民车位，其月费的到期时间戳。 */
  ParkingStatus status;     /**< 车位当前的占用状态 (空闲/占用)。 */
  struct ParkingSlot *next; /**< 指向链表中下一个停车位节点的指针。 */
} ParkingSlot;

/**
 * @brief 描述整个停车场的状态和统计信息。
 * @details
 * 此结构体是管理整个停车场的根对象，包含了所有车位数据和关键的统计指标。
 */
typedef struct ParkingLot {
  int total_slots;         /**< 停车场设计的总车位数。 */
  int occupied_slots;      /**< 当前已被占用的车位数。 */
  ParkingSlot *slot_head;  /**< 指向车位信息链表的头节点。 */
  double today_revenue;    /**< 当日产生的总收入。 */
  double month_revenue;    /**< 当月产生的总收入。 */
  time_t last_update_time; /**< 收入统计信息的最后更新时间戳。 */
} ParkingLot;

/**
 *********************************************************************************
 *                            数据层核心API声明
 *********************************************************************************
 */

/** @name 基础管理函数 */
///@{

/**
 * @brief 初始化一个新的停车场对象。
 * @details 为停车场分配内存并设置其初始状态，包括总车位数、占用数和链表头。
 * @param total_slots 停车场的总容量。
 * @return 成功时返回指向新分配的 ParkingLot 对象的指针，若内存分配失败则返回
 * NULL。
 */
ParkingLot *init_parking_lot(int total_slots);

/**
 * @brief 创建一个新的停车位对象。
 * @details 为单个停车位分配内存并初始化其属性。新创建的车位状态默认为空闲。
 * @param slot_id 要创建的车位的唯一编号。
 * @param location 车位的物理位置描述。
 * @return 成功时返回指向新分配的 ParkingSlot 对象的指针，若内存分配失败则返回
 * NULL。
 */
ParkingSlot *create_parking_slot(int slot_id, const char *location);

/**
 * @brief 将一个已创建的停车位添加到停车场链表中。
 * @param lot 目标停车场。
 * @param slot 要添加的停车位节点。
 * @return 成功返回 0，若参数无效则返回 -1。
 */
int add_parking_slot(ParkingLot *lot, ParkingSlot *slot);

///@}

/** @name 查询函数 */
///@{

/**
 * @brief 根据车位编号查找停车位。
 * @param lot 目标停车场。
 * @param slot_id 要查找的车位编号。
 * @return 若找到，返回对应的 ParkingSlot 指针；否则返回 NULL。
 */
ParkingSlot *find_slot_by_id(ParkingLot *lot, int slot_id);

/**
 * @brief 根据车牌号查找停车位。
 * @param lot 目标停车场。
 * @param license_plate 要查找的车牌号。
 * @return 若找到，返回对应的 ParkingSlot 指针；否则返回 NULL。
 */
ParkingSlot *find_slot_by_license(ParkingLot *lot, const char *license_plate);

/**
 * @brief 根据车主姓名查找停车位（模糊查找）。
 * @param lot 目标停车场。
 * @param owner_name 要查找的车主姓名（或姓名的一部分）。
 * @return 若找到，返回第一个匹配的 ParkingSlot 指针；否则返回 NULL。
 */
ParkingSlot *find_slot_by_owner(ParkingLot *lot, const char *owner_name);

///@}

/** @name 车辆出入场函数 */
///@{

/**
 * @brief 分配一个停车位给车辆（车辆入场）。
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
                  ParkingType type);

/**
 * @brief 释放一个停车位（车辆出场）。
 * @param lot 目标停车场。
 * @param slot_id 要释放的车位编号。
 * @return 返回码：0 成功, -1 停车场对象为空, -2 车位不存在, -3
 * 车位本就是空闲状态。
 */
int deallocate_slot(ParkingLot *lot, int slot_id);

///@}

/** @name 列表查询函数 */
///@{

/**
 * @brief 获取所有空闲车位的列表。
 * @param lot 目标停车场。
 * @param[out] count 用于接收空闲车位数量的指针。
 * @return 返回一个动态分配的 ParkingSlot 指针数组。使用后需要调用 free()
 * 释放。如果无空闲车位或内存分配失败，返回 NULL。
 */
ParkingSlot **get_free_slots(ParkingLot *lot, int *count);

/**
 * @brief 获取所有已占用车位的列表。
 * @param lot 目标停车场。
 * @param[out] count 用于接收已占用车位的数量。
 * @return 返回一个动态分配的 ParkingSlot 指针数组。使用后需要调用 free()
 * 释放。如果无已占用车位或内存分配失败，返回 NULL。
 */
ParkingSlot **get_occupied_slots(ParkingLot *lot, int *count);

/**
 * @brief 获取所有车位的列表。
 * @param lot 目标停车场。
 * @param[out] count 用于接收找到的车位数量。
 * @return 返回一个动态分配的 ParkingSlot 指针数组。使用后需要调用 free()
 * 释放。如果停车场为空或内存分配失败，返回 NULL。
 */
ParkingSlot **get_all_slots(ParkingLot *lot, int *count);

/**
 * @brief 按停车时长排序后，获取已占用车辆列表。
 * @param lot 目标停车场。
 * @param[out] count 用于接收已占用车位的数量。
 * @param ascending 是否按升序排列 (1: 升序, 0: 降序)。
 * @return 返回排序后的 ParkingSlot 指针数组。数组使用后需要调用 free() 释放。
 */
ParkingSlot **get_slots_by_duration(ParkingLot *lot, int *count, int ascending);

///@}

/** @name 计费函数 */
///@{

/**
 * @brief 计算访客车辆的停车费用。
 * @details 根据入场和出场时间计算访客的停车费用。计费规则为按小时向上取整。
 * @param entry_time 车辆入场时间戳。
 * @param exit_time 车辆出场时间戳。
 * @return 计算出的停车费用。
 */
double calculate_visitor_fee(time_t entry_time, time_t exit_time);

///@}

/** @name 车位信息管理 */
///@{

/**
 * @brief 更新一个停车位的信息。
 * @param slot 要更新的停车位。
 * @param location 新的位置描述 (如果为NULL则不更新)。
 * @param owner_name 新的车主姓名 (如果为NULL则不更新, 仅在占用时可更新)。
 * @param contact 新的联系方式 (如果为NULL则不更新, 仅在占用时可更新)。
 * @return 成功返回 0，失败（参数为空）返回 -1。
 */
int update_slot_info(ParkingSlot *slot, const char *location,
                     const char *owner_name, const char *contact);

/**
 * @brief 从停车场中删除一个车位。
 * @details 只有空闲车位才能被删除。
 * @param lot 目标停车场。
 * @param slot_id 要删除的车位编号。
 * @return 返回码：0 成功, -1 停车场对象为空, -2 车位正在被使用，无法删除, -3
 * 车位不存在。
 */
int delete_slot(ParkingLot *lot, int slot_id);

///@}

/** @name 统计分析函数 */
///@{

/**
 * @brief 统计指定日期内某类型车辆的入场总数。
 * @param lot 目标停车场。
 * @param date 指定的日期 (time_t)。
 * @param type 车辆类型 (居民/访客)。
 * @return 指定日期和类型的车辆入场总数。
 */
int count_daily_parking(ParkingLot *lot, time_t date, ParkingType type);

/**
 * @brief 统计指定月份内某类型车辆的入场总数。
 * @param lot 目标停车场。
 * @param year 年份。
 * @param month 月份 (1-12)。
 * @param type 车辆类型 (居民/访客)。
 * @return 指定月份和类型的车辆入场总数。
 */
int count_monthly_parking(ParkingLot *lot, int year, int month,
                          ParkingType type);

/**
 * @brief 获取指定月份的总收入。
 * @note 此函数当前未实现，仅为示例。实际收入统计在ParkingLot结构体中。
 * @param lot 目标停车场。
 * @param year 年份。
 * @param month 月份 (1-12)。
 * @return 当月的总收入。
 */
double get_monthly_payment_total(ParkingLot *lot, int year, int month);

///@}

/** @name 数据持久化函数 */
///@{

/**
 * @brief 将停车场的所有数据保存到文本文件。
 * @param lot 要保存的停车场。
 * @param filename 目标文件名。
 * @return 成功返回 0，失败（文件无法打开）返回 -1。
 */
int save_parking_data(ParkingLot *lot, const char *filename);

/**
 * @brief 从文本文件中加载停车场数据。
 * @param filename 源文件名。
 * @return 成功时返回重建的 ParkingLot 指针；失败（文件不存在或格式错误）时返回
 * NULL。
 */
ParkingLot *load_parking_data(const char *filename);

///@}

/** @name 内存管理函数 */
///@{

/**
 * @brief 释放单个停车位对象占用的内存。
 * @param slot 要释放的停车位。
 */
void free_parking_slot(ParkingSlot *slot);

/**
 * @brief 释放整个停车场（包括所有车位）占用的内存。
 * @details 遍历车位链表，逐个释放所有停车位节点，最后释放停车场本身。
 * @param lot 要释放的停车场。
 */
void free_parking_lot(ParkingLot *lot);

///@}

#endif /* PARKING_DATA_H */
