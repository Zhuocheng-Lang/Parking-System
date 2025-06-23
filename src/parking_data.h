#ifndef PARKING_DATA_H
#define PARKING_DATA_H

#include <time.h>

/* ===== 常量定义 ===== */
#define MAX_LOCATION_LEN 100 /**< 位置描述字符串最大长度 */
#define MAX_NAME_LEN 50      /**< 姓名字符串最大长度 */
#define MAX_LICENSE_LEN 50   /**< 车牌号字符串最大长度 */
#define MAX_CONTACT_LEN 50   /**< 联系方式字符串最大长度 */

/* ===== 枚举类型定义 ===== */

/**
 * @brief 车位类型枚举
 */
typedef enum {
  RESIDENT_TYPE = 0, /**< 居民车位 */
  VISITOR_TYPE = 1   /**< 外来车位 */
} ParkingType;

/**
 * @brief 车位状态枚举
 */
typedef enum {
  FREE_STATUS = 0,    /**< 空闲 */
  OCCUPIED_STATUS = 1 /**< 已占用 */
} ParkingStatus;

/* ===== 结构体定义 ===== */

/**
 * @brief 缴费记录结构体
 * @details 用于存储车位的缴费历史记录，以链表形式组织
 */
typedef struct PaymentRecord {
  time_t start_date;          /**< 缴费起始日期 */
  time_t end_date;            /**< 缴费结束日期 */
  double amount;              /**< 缴费金额 */
  struct PaymentRecord *next; /**< 指向下一条缴费记录 */
} PaymentRecord;

/**
 * @brief 停车位结构体
 * @details 停车位的完整信息，包含车位基本信息、使用人信息、状态和缴费记录
 */
typedef struct ParkingSlot {
  int slot_id;                         /**< 车位编号（唯一标识） */
  char location[MAX_LOCATION_LEN];     /**< 位置描述 */
  char owner_name[MAX_NAME_LEN];       /**< 使用人姓名 */
  char license_plate[MAX_LICENSE_LEN]; /**< 车牌号 */
  char contact[MAX_CONTACT_LEN];       /**< 联系方式 */
  ParkingType type;                    /**< 车位类型 */
  time_t entry_time;                   /**< 入场时间 */
  time_t exit_time;                    /**< 出场时间 */
  ParkingStatus status;                /**< 当前状态 */
  PaymentRecord *payment_head;         /**< 缴费记录链表头 */
  struct ParkingSlot *next;            /**< 指向下一个车位节点 */
} ParkingSlot;

/**
 * @brief 停车场管理结构体
 * @details 停车场的整体管理信息，包含所有车位的链表和统计信息
 */
typedef struct ParkingLot {
  ParkingSlot *head;  /**< 停车位链表头 */
  int total_slots;    /**< 总车位数 */
  int occupied_slots; /**< 已占用车位数 */
} ParkingLot;

/* ===== 函数声明 ===== */

/**
 * @brief 停车场管理基础函数
 */

/**
 * @brief 初始化停车场
 * @param total_slots 停车场总车位数
 * @return 成功返回停车场指针，失败返回NULL
 */
ParkingLot *init_parking_lot(int total_slots);

/**
 * @brief 创建停车位
 * @param slot_id 车位编号
 * @param location 车位位置描述
 * @return 成功返回停车位指针，失败返回NULL
 */
ParkingSlot *create_parking_slot(int slot_id, const char *location);

/**
 * @brief 添加停车位到停车场
 * @param lot 停车场指针
 * @param slot 停车位指针
 * @return 成功返回0，失败返回-1
 */
int add_parking_slot(ParkingLot *lot, ParkingSlot *slot);

/**
 * @brief 查找函数
 */

/**
 * @brief 根据车位编号查找停车位
 * @param lot 停车场指针
 * @param slot_id 车位编号
 * @return 找到返回停车位指针，未找到返回NULL
 */
ParkingSlot *find_slot_by_id(ParkingLot *lot, int slot_id);

/**
 * @brief 根据车牌号查找停车位
 * @param lot 停车场指针
 * @param license_plate 车牌号
 * @return 找到返回停车位指针，未找到返回NULL
 */
ParkingSlot *find_slot_by_license(ParkingLot *lot, const char *license_plate);

/**
 * @brief 根据车主姓名查找停车位
 * @param lot 停车场指针
 * @param owner_name 车主姓名
 * @return 找到返回停车位指针，未找到返回NULL
 */
ParkingSlot *find_slot_by_owner(ParkingLot *lot, const char *owner_name);

/**
 * @brief 车位分配和释放函数
 */

/**
 * @brief 分配停车位（车辆入场）
 * @param lot 停车场指针
 * @param slot_id 车位编号
 * @param owner_name 车主姓名
 * @param license_plate 车牌号
 * @param contact 联系方式
 * @param type 停车类型
 * @return 成功返回0，失败返回-1
 */
int allocate_slot(ParkingLot *lot, int slot_id, const char *owner_name,
                  const char *license_plate, const char *contact,
                  ParkingType type);

/**
 * @brief 释放停车位（车辆出场）
 * @param lot 停车场指针
 * @param slot_id 车位编号
 * @return 成功返回0，失败返回-1
 */
int deallocate_slot(ParkingLot *lot, int slot_id);

/**
 * @brief 列表查询函数
 */

/**
 * @brief 获取空闲停车位列表
 * @param lot 停车场指针
 * @param count 返回空闲车位数量
 * @return 空闲车位指针数组，使用后需要释放
 */
ParkingSlot **get_free_slots(ParkingLot *lot, int *count);

/**
 * @brief 获取已占用停车位列表
 * @param lot 停车场指针
 * @param count 返回已占用车位数量
 * @return 已占用车位指针数组，使用后需要释放
 */
ParkingSlot **get_occupied_slots(ParkingLot *lot, int *count);

/**
 * @brief 按停车时长排序获取车辆列表
 * @param lot 停车场指针
 * @param count 返回车位数量
 * @param ascending 是否升序排列（1升序，0降序）
 * @return 排序后的车位指针数组，使用后需要释放
 */
ParkingSlot **get_slots_by_duration(ParkingLot *lot, int *count, int ascending);

/**
 * @brief 缴费和计费函数
 */

/**
 * @brief 添加缴费记录
 * @param slot 停车位指针
 * @param start_date 缴费起始日期
 * @param end_date 缴费结束日期
 * @param amount 缴费金额
 * @return 成功返回0，失败返回-1
 */
int add_payment_record(ParkingSlot *slot, time_t start_date, time_t end_date,
                       double amount);

/**
 * @brief 计算外来车辆停车费用
 * @param entry_time 入场时间
 * @param exit_time 出场时间
 * @return 应收费用金额
 */
double calculate_visitor_fee(time_t entry_time, time_t exit_time);

/**
 * @brief 车位信息管理函数
 */

/**
 * @brief 修改停车位信息
 * @param slot 停车位指针
 * @param location 新位置描述
 * @param owner_name 新车主姓名
 * @param contact 新联系方式
 * @return 成功返回0，失败返回-1
 */
int update_slot_info(ParkingSlot *slot, const char *location,
                     const char *owner_name, const char *contact);

/**
 * @brief 删除停车位
 * @param lot 停车场指针
 * @param slot_id 车位编号
 * @return 成功返回0，失败返回-1
 */
int delete_slot(ParkingLot *lot, int slot_id);

/**
 * @brief 统计分析函数
 */

/**
 * @brief 统计每日停车数量
 * @param lot 停车场指针
 * @param date 指定日期
 * @param type 停车类型
 * @return 当日指定类型的停车数量
 */
int count_daily_parking(ParkingLot *lot, time_t date, ParkingType type);

/**
 * @brief 统计每月停车数量
 * @param lot 停车场指针
 * @param year 年份
 * @param month 月份
 * @param type 停车类型
 * @return 当月指定类型的停车数量
 */
int count_monthly_parking(ParkingLot *lot, int year, int month,
                          ParkingType type);

/**
 * @brief 获取月度缴费总额
 * @param lot 停车场指针
 * @param year 年份
 * @param month 月份
 * @return 当月缴费总额
 */
double get_monthly_payment_total(ParkingLot *lot, int year, int month);

/**
 * @brief 数据持久化函数
 */

/**
 * @brief 保存停车场数据到文件
 * @param lot 停车场指针
 * @param filename 文件名
 * @return 成功返回0，失败返回-1
 */
int save_parking_data(ParkingLot *lot, const char *filename);

/**
 * @brief 从文件加载停车场数据
 * @param filename 文件名
 * @return 成功返回停车场指针，失败返回NULL
 */
ParkingLot *load_parking_data(const char *filename);

/**
 * @brief 内存管理函数
 */

/**
 * @brief 释放缴费记录链表
 * @param head 缴费记录链表头
 */
void free_payment_records(PaymentRecord *head);

/**
 * @brief 释放停车位内存
 * @param slot 停车位指针
 */
void free_parking_slot(ParkingSlot *slot);

/**
 * @brief 释放停车场内存
 * @param lot 停车场指针
 */
void free_parking_lot(ParkingLot *lot);

#endif /* PARKING_DATA_H */
