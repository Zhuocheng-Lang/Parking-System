#ifndef PARKING_UI_H
#define PARKING_UI_H

#include "parking_data.h"
#include "parking_service.h"

/*
 * 停车管理系统UI层头文件
 * 提供用户界面相关的所有功能声明
 * 实现UI与业务逻辑的完全分离
 */

/* ===== 系统初始化和主程序入口 ===== */

/* 运行停车管理系统主程序 */
void ui_run_parking_system(void);

/* 初始化停车管理系统 */
void ui_initialize_parking_system(void);

/* 清理资源并退出系统 */
void ui_cleanup_and_exit(void);

/* ===== 基础UI功能函数 ===== */

/* 设置控制台编码以支持中文显示 */
void ui_setup_console_encoding(void);

/* 清除输入缓冲区 */
void ui_clear_input_buffer(void);

/* 等待用户按键继续 */
void ui_wait_for_continue(void);

/* 安全的整数输入函数 */
int ui_safe_read_int(int *value);

/* 安全的字符串输入函数 */
int ui_safe_read_string(char *buffer, size_t buffer_size);

/* ===== 主菜单相关函数 ===== */

/* 显示主菜单 */
void ui_show_main_menu(void);

/* 处理菜单选择 */
void ui_handle_menu_choice(int choice);

/* ===== 停车位管理菜单 ===== */

/* 添加停车位菜单 */
void ui_add_parking_slot_menu(void);

/* 车辆入场（分配车位）菜单 */
void ui_allocate_slot_menu(void);

/* 车辆出场（释放车位）菜单 */
void ui_deallocate_slot_menu(void);

/* 查询车位信息菜单 */
void ui_query_slot_menu(void);

/* 显示车位列表菜单 */
void ui_list_slots_menu(void);

/* ===== 支付和统计菜单 ===== */

/* 缴费管理菜单 */
void ui_payment_menu(void);

/* 统计信息菜单 */
void ui_statistics_menu(void);

/* ===== 数据管理菜单 ===== */

/* 保存数据菜单 */
void ui_save_data_menu(void);

/* 加载数据菜单 */
void ui_load_data_menu(void);

/* ===== 辅助显示函数 ===== */

/* 显示系统标题 */
void ui_show_system_title(void);

/* 显示车位状态 */
void ui_show_slot_status(const ParkingSlot *slot);

/* 显示错误信息 */
void ui_show_error(const char *message);

/* 显示成功信息 */
void ui_show_success(const char *message);

/* 显示分隔线 */
void ui_show_separator(void);

/* ===== 演示程序功能 ===== */

/* 运行内置演示程序 */
void ui_run_demo_program(void);

/* ===== 获取全局停车场对象 ===== */

/* 获取UI层管理的停车场对象指针 */
ParkingLot *ui_get_parking_lot(void);

/* 清屏函数 */
void ui_clear_screen(void);

#endif /* PARKING_UI_H */
