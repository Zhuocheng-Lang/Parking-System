/**
 * @file test_parking_ui.c
 * @brief UI层单元测试文件
 * @details
 * 该文件使用 cmocka 测试框架，对 parking_ui.c 中的函数进行单元测试。
 * 测试覆盖了UI层的生命周期管理（初始化和清理）以及通过UI层接口
 * 触发的核心业务逻辑，例如添加车位。测试旨在确保UI层能够正确地
 * 调用服务层函数并处理其返回结果。
 */
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "../src/parking_service.h"
#include "../src/parking_ui.h"
#include "cmocka.h"

/* ========================================================================== */
/*                            测试的 Setup 和 Teardown */
/* ========================================================================== */

/**
 * @brief UI模块的全局停车场实例。
 * @details
 * 通过 `extern` 关键字引入，以便测试用例能够访问和验证
 * `parking_ui.c` 中定义的全局停车场对象的状态。
 */
extern ParkingLot *ui_parking_lot;

/**
 * @brief 每个测试用例运行前的设置函数 (Setup)。
 * @details
 * 此函数在每个测试用例执行之前被 cmocka 框架调用。
 * 它通过调用 `ui_initialize_parking_system` 来创建一个新的、干净的停车场环境，
 * 并将创建的停车场对象指针存储在 `state` 中，供测试用例使用。
 * @param[out] state cmocka 框架提供的状态指针的指针，用于在 setup, test,
 * teardown 之间传递数据。
 * @return 成功时返回 0，表示设置成功。
 */
static int setup(void **state) {
  ui_initialize_parking_system(10);
  *state = ui_get_parking_lot();
  return 0;
}

/**
 * @brief 每个测试用例运行后的清理函数 (Teardown)。
 * @details
 * 此函数在每个测试用例执行之后被 cmocka 框架调用。
 * 它负责调用 `ui_cleanup_and_exit` 来释放由 `setup` 函数或测试用例本身
 * 分配的所有资源，确保测试之间的独立性。
 * @param state cmocka 框架提供的状态指针的指针，包含由 setup 传递的数据。
 * @return 成功时返回 0，表示清理成功。
 */
static int teardown(void **state) {
  ui_cleanup_and_exit();
  return 0;
}

/* ========================================================================== */
/*                                 测试用例实现                               */
/* ========================================================================== */

/**
 * @brief 测试UI层的初始化和清理生命周期。
 * @details
 * 此测试验证 `setup` 和 `teardown` 函数是否能正确工作。
 * 它断言 `setup` 函数成功创建了一个非空的停车场对象，
 * 并且该对象的总车位数与初始化时指定的值（10）相符。
 * `teardown` 的成功执行则隐式地被框架所保证。
 * @param state 包含由 `setup` 函数初始化的停车场对象的指针。
 */
static void test_ui_lifecycle(void **state) {
  ParkingLot *lot = (ParkingLot *)*state;
  assert_non_null(lot);
  assert_int_equal(lot->total_slots, 10);
}

/**
 * @brief 测试通过UI层接口添加车位的逻辑。
 * @details
 * 此测试模拟用户通过UI添加一个新车位的场景。由于无法直接测试 `scanf`
 * 等输入函数，本测试直接调用服务层函数 `parking_service_add_slot`
 * 来模拟UI操作的后端效果。
 * 测试断言：
 * 1. 添加操作返回成功。
 * 2. 添加后，可以通过ID找到该车位。
 * 3. 找到的车位的位置信息与添加时提供的一致。
 * @param state 包含由 `setup` 函数初始化的停车场对象的指针。
 */
static void test_ui_add_slot_logic(void **state) {
  ParkingLot *lot = (ParkingLot *)*state;
  ServiceResult result = parking_service_add_slot(lot, 1, "Test-Location");
  assert_true(parking_service_is_success(result));

  ParkingSlot *found = find_slot_by_id(lot, 1);
  assert_non_null(found);
  assert_string_equal(found->location, "Test-Location");
}

/* ========================================================================== */
/*                                 主测试函数                                 */
/* ========================================================================== */

/**
 * @brief 测试执行入口点。
 * @details
 * `main` 函数是单元测试程序的入口。它定义了一个 `CMUnitTest`
 * 数组，其中包含了所有要执行的测试用例。
 * `cmocka_run_group_tests` 函数负责运行这个测试组，并报告结果。
 * @return 返回 cmocka 框架的测试结果。
 */
int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test_setup_teardown(test_ui_lifecycle, setup, teardown),
      cmocka_unit_test_setup_teardown(test_ui_add_slot_logic, setup, teardown),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
