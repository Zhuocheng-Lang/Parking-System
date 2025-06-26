#ifndef PARKING_UI_H
#define PARKING_UI_H

#include <stddef.h>

#include "parking_data.h"
#include "parking_service.h"


/**
 * @file parking_ui.h
 * @brief UI层接口声明文件
 * @details
 * 该文件声明了所有与用户界面（UI）相关的功能。
 * 它负责处理用户输入、显示菜单、渲染数据以及调用服务层来执行业务逻辑。
 * UI层与核心业务逻辑完全分离，仅通过服务层接口进行交互。
 */

/**
 *********************************************************************************
 *                        系统初始化和主程序入口
 *********************************************************************************
 */

/** @name 系统生命周期函数 */
///@{

/**
 * @brief 运行停车管理系统的主程序。
 * @details 这是程序的总入口点，负责初始化、运行主循环和最终清理。
 */
void ui_run_parking_system(void);

/**
 * @brief 初始化停车管理系统。
 * @details 创建全局的停车场对象 `ui_parking_lot`，为系统运行做准备。
 * @param total_slots 要初始化的停车场总容量。
 */
void ui_initialize_parking_system(int total_slots);

/**
 * @brief 清理资源并准备退出系统。
 * @details 负责在程序退出前自动保存数据并释放所有动态分配的内存。
 */
void ui_cleanup_and_exit(void);

///@}

/**
 *********************************************************************************
 *                            基础UI及交互函数
 *********************************************************************************
 */

/** @name 基础交互函数 */
///@{

/**
 * @brief 设置控制台编码以支持中文字符的正确显示。
 * @note 针对Windows和类Unix系统进行了适配。
 */
void ui_setup_console_encoding(void);

/**
 * @brief 清除标准输入缓冲区中剩余的字符。
 * @details 读取并丢弃所有字符，直到遇到换行符或文件结尾，
 *          防止后续的输入函数读取到意外的字符。
 */
void ui_clear_input_buffer(void);

/**
 * @brief 安全地从标准输入读取一个整数。
 * @details 循环提示直到用户输入一个有效的整数。会处理无效输入和缓冲区溢出问题。
 * @param[out] value 指向用于存储读取到的整数的变量的指针。
 * @return 成功返回0，若读取失败（如遇到EOF）则返回-1。
 */
int ui_safe_read_int(int *value);

/**
 * @brief 安全地从标准输入读取一个字符串。
 * @details 读取一行文本，处理缓冲区溢出，并移除末尾的换行符。
 * @param buffer 指向用于存储字符串的缓冲区的指针。
 * @param buffer_size 缓冲区的大小。
 * @return 成功返回0，若读取失败（如遇到EOF）则返回-1。
 */
int ui_safe_read_string(char *buffer, size_t buffer_size);

/**
 * @brief (跨平台) 清除控制台屏幕。
 */
void ui_clear_screen(void);

/**
 * @brief 暂停程序执行，等待用户按Enter键继续。
 * @details 通常在显示完信息后调用，给用户阅读时间，然后清屏以准备显示新内容。
 */
void ui_wait_for_continue(void);

///@}

/**
 *********************************************************************************
 *                            UI菜单处理函数
 *********************************************************************************
 */

/** @name 主菜单及分发函数 */
///@{

/**
 * @brief 显示系统的主菜单选项。
 */
void ui_show_main_menu(void);

/**
 * @brief 根据用户输入的选择，分发到对应的子菜单处理函数。
 * @param choice 用户从主菜单选择的选项编号。
 */
void ui_handle_menu_choice(int choice);

///@}

/** @name 业务功能菜单 */
///@{

/**
 * @brief 显示并处理“添加停车位”的交互流程。
 */
void ui_add_parking_slot_menu(void);

/**
 * @brief 显示并处理“车辆入场”（分配车位）的交互流程。
 */
void ui_allocate_slot_menu(void);

/**
 * @brief 显示并处理“车辆出场”（释放车位）的交互流程。
 */
void ui_deallocate_slot_menu(void);

/**
 * @brief 显示并处理“查询车位信息”的交互流程。
 */
void ui_query_slot_menu(void);

/**
 * @brief 显示并处理“显示车位列表”的交互流程（所有/空闲/已占用）。
 */
void ui_list_slots_menu(void);

/**
 * @brief 显示并处理“统计信息”的交互流程。
 */
void ui_statistics_menu(void);

///@}

/** @name 数据管理菜单 */
///@{

/**
 * @brief 显示并处理“保存数据到文件”的交互流程。
 */
void ui_save_data_menu(void);

/**
 * @brief 显示并处理“从文件加载数据”的交互流程。
 */
void ui_load_data_menu(void);

///@}

/**
 *********************************************************************************
 *                            辅助显示函数
 *********************************************************************************
 */

/** @name 信息显示函数 */
///@{

/**
 * @brief 显示系统的欢迎标题。
 */
void ui_show_system_title(void);

/**
 * @brief 以标准格式显示单个停车位的详细信息。
 * @param slot 指向要显示的 ParkingSlot 对象的指针。
 */
void ui_show_slot_status(const ParkingSlot *slot);

/**
 * @brief 以标准格式显示一条错误消息。
 * @param message 要显示的错误信息字符串。
 */
void ui_show_error(const char *message);

/**
 * @brief 以标准格式显示一条成功操作的消息。
 * @param message 要显示的成功信息字符串。
 */
void ui_show_success(const char *message);

/**
 * @brief 打印一条分隔线，用于美化界面布局。
 */
void ui_show_separator(void);

///@}

/**
 *********************************************************************************
 *                            其他功能
 *********************************************************************************
 */

/** @name 演示及内部接口 */
///@{

/**
 * @brief 运行一个内置的演示程序，展示系统的核心功能。
 * @details 此函数会添加、分配车位并显示统计信息，用于快速验证系统。
 */
void ui_run_demo_program(void);

/**
 * @brief 获取UI层当前管理的停车场对象的指针。
 * @details 主要用于测试或需要直接访问全局停车场对象的场景。
 * @return 指向全局 ParkingLot 对象的指针。
 */
ParkingLot *ui_get_parking_lot(void);

///@}

#endif /* PARKING_UI_H */
