/**
 * @file main.c
 * @brief 主程序入口文件
 * @details
 * 这是整个停车管理系统的入口点。
 * 它的唯一职责是调用UI层的启动函数 `ui_run_parking_system()` 来运行程序。
 * 所有的业务逻辑、数据管理和用户交互都由其他模块处理。
 */

#include "parking_ui.h"

/**
 * @brief 主函数
 * @details
 * 程序从这里开始执行。
 * @return 程序正常退出时返回0。
 */
int main(void) {
  /* 调用UI层来启动和管理整个应用程序 */
  ui_run_parking_system();
  return 0;
}
