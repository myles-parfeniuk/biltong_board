// third-party
#include "UnityInterface.h"
#include "unity.h"
// OPEEngine
#include "CallbackTests.h"
#include "OPEEngineTestHelper.h"
#include "OPEEngineConfig.h"
#include "OPEEngine_freeRTOS_hook.h"
#include "OPEEngine.h"
#include "DataWatch.h"

namespace opee
{
    class CallbackTests
    {
    public:
        static bool single_cb_test()
        {
            const constexpr char *TEST_TAG = "single_cb_test";
            const constexpr TickType_t CB_EXECUTION_TIMEOUT_MS = 5UL / portTICK_PERIOD_MS;

            opee::OPEEngine_init();

            SemaphoreHandle_t sem = xSemaphoreCreateBinary();

            opee::DataWatch<bool, 32, 1> data_to_set(false);
            opee::CbHelper<OPEEconfigMAX_DATA_WATCH_CNT>::init();

            if (data_to_set.get() != false)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: incorrect inital data in DataWatch object.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: DataWatch data initial value check.");
            }

            OPEEngineRes_t OPEEres = data_to_set.subscribe<16>([&sem](bool new_data)
                                                               { xSemaphoreGive(sem); });

            if (OPEEres != OPEE_OK)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL:  subscribe() did not return successfully: %s.", OPEEngineRes_to_str(OPEEres));
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS:  successfully subscribed callback.");
            }

            if (data_to_set.set(true) != OPEE_OK)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL:  set() did not return successfully.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: set data & queue cb check.");
            }

            if (xSemaphoreTake(sem, CB_EXECUTION_TIMEOUT_MS) != pdTRUE)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: callback never executed");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: cb execution check.");
            }

            if (data_to_set.get() != true)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: no new data after cb execution.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: new data after cb execution check.");
            }

            vSemaphoreDelete(sem);

            return true;
        }

        static bool multi_cb_test()
        {
            const constexpr char *TEST_TAG = "multi_cb_test";

            const constexpr EventBits_t CB_0_EXECUTED_BIT = 1U << 0U;
            const constexpr EventBits_t CB_1_EXECUTED_BIT = 1U << 1U;
            const constexpr EventBits_t CB_2_EXECUTED_BIT = 1U << 2U;
            const constexpr EventBits_t CB_ALL_EXECUTED_BITS = (CB_0_EXECUTED_BIT | CB_1_EXECUTED_BIT | CB_2_EXECUTED_BIT);
            const constexpr TickType_t CB_EXECUTION_TIMEOUT_MS = 5UL / portTICK_PERIOD_MS;

            opee::OPEEngine_init();

            EventGroupHandle_t evt_grp_data_to_set = xEventGroupCreate();
            OPEEngineRes_t OPEEres[3] = {OPEE_CB_WRPR_CREATION_FAILED};

            opee::DataWatch<bool, 128, 3> data_to_set(false);
            opee::CbHelper<OPEEconfigMAX_DATA_WATCH_CNT>::init();

            if (data_to_set.get() != false)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: incorrect inital data in DataWatch object.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: DataWatch data initial value check.");
            }

            OPEEres[0] = data_to_set.subscribe<16>(
                [&evt_grp_data_to_set](bool new_data)
                {
                    xEventGroupSetBits(evt_grp_data_to_set, CB_0_EXECUTED_BIT);
                    OPEEngineTestHelper::print_test_msg(TEST_TAG, "CB_0 executed...");
                });
            OPEEres[1] = data_to_set.subscribe<16>(
                [&evt_grp_data_to_set](bool new_data)
                {
                    xEventGroupSetBits(evt_grp_data_to_set, CB_1_EXECUTED_BIT);
                    OPEEngineTestHelper::print_test_msg(TEST_TAG, "CB_1 executed...");
                });
            OPEEres[2] = data_to_set.subscribe<16>(
                [&evt_grp_data_to_set](bool new_data)
                {
                    xEventGroupSetBits(evt_grp_data_to_set, CB_2_EXECUTED_BIT);
                    OPEEngineTestHelper::print_test_msg(TEST_TAG, "CB_2 executed...");
                });

            for (opee_ssize_t i = 0; i < 3; i++)
            {
                if (OPEEres[i] != OPEE_OK)
                {
                    OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL:  subscribe() did not return successfully for callback %d: %s.", i, OPEEngineRes_to_str(OPEEres[i]));
                    return false;
                }
            }

            OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS:  sub check.");

            if (data_to_set.set(true) != OPEE_OK)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL:  set() did not return successfully.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: set data & queue cb check.");
            }

            EventBits_t res = xEventGroupWaitBits(evt_grp_data_to_set, CB_ALL_EXECUTED_BITS, pdFALSE, pdTRUE, CB_EXECUTION_TIMEOUT_MS);

            if (res != CB_ALL_EXECUTED_BITS)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL:  not all callbacks executed.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: cb execution check.");
            }

            if (data_to_set.get() != true)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: no new data after cb execution.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: new data after cb execution check.");
            }

            vEventGroupDelete(evt_grp_data_to_set);

            return true;
        }

        static bool multi_task_cb_test()
        {
            const constexpr char *TEST_TAG = "multi_task_cb_test";

            const constexpr EventBits_t SUBSCRIBED_BIT = 1U << 0U;
            const constexpr EventBits_t DATA_SET_BIT = 1U << 1U;
            const constexpr EventBits_t CB_EXECUTED_BIT = 1U << 2U;
            const constexpr TickType_t DATA_SET_TIMEOUT_MS = 5UL / portTICK_PERIOD_MS;
            const constexpr TickType_t CB_EXECUTION_TIMEOUT_MS = 5UL / portTICK_PERIOD_MS;
            const constexpr TickType_t SUB_TIMEOUT_MS = 5UL / portTICK_PERIOD_MS;

            opee::OPEEngine_init();

            EventGroupHandle_t evt_grp_data_to_set = xEventGroupCreate();

            opee::DataWatch<bool, 128, 3> data_to_set(false);
            opee::CbHelper<OPEEconfigMAX_DATA_WATCH_CNT>::init();

            multi_task_cb_test_task_ctx_t ctx = {&data_to_set, evt_grp_data_to_set};

            if (xTaskCreate(multi_task_cb_test_task_1, "multi_task_cb_test_task_0", 256U, &ctx, 2, NULL) != pdTRUE)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: could not create test task 0.");
                return false;
            }

            if (xTaskCreate(multi_task_cb_test_task_0, "multi_task_cb_test_task_1", 256U, &ctx, 2, NULL) != pdTRUE)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: could not create test task 1.");
                return false;
            }

            OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: test task creation check.");

            EventBits_t res = xEventGroupWaitBits(evt_grp_data_to_set, SUBSCRIBED_BIT, pdFALSE, pdFALSE, SUB_TIMEOUT_MS);

            if (!(res & SUBSCRIBED_BIT))
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: task 1 did not successfully subscribe callback.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: sub check");
            }

            res = xEventGroupWaitBits(evt_grp_data_to_set, DATA_SET_BIT, pdFALSE, pdFALSE, DATA_SET_TIMEOUT_MS);

            if (!(res & DATA_SET_BIT))
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: task 0 did not successfully set data.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: set data and queue cb check");
            }

            res = xEventGroupWaitBits(evt_grp_data_to_set, CB_EXECUTED_BIT, pdFALSE, pdFALSE, CB_EXECUTION_TIMEOUT_MS);

            if (!(res & CB_EXECUTED_BIT))
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: task 1 did not successfully detect cb execution.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: cb execution check");
            }

            if (data_to_set.get() != true)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: no new data after cb execution.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: new data after cb execution check.");
            }

            vEventGroupDelete(evt_grp_data_to_set);
            return true;
        }

        static bool self_set_test()
        {
            const constexpr char *TEST_TAG = "self_set_test";
            const constexpr EventBits_t CB_EXECUTED_BIT = 1U << 0U;
            const constexpr EventBits_t CB_EXECUTED_RECURSIVELY_BIT = 1U << 1U;
            const constexpr TickType_t CB_EXECUTION_TIMEOUT_MS = 5UL / portTICK_PERIOD_MS;

            opee::OPEEngine_init();

            EventGroupHandle_t evt_grp_data_to_set = xEventGroupCreate();

            opee::DataWatch<opee_uint8_t, 32, 1> data_to_set(0U);
            opee::CbHelper<OPEEconfigMAX_DATA_WATCH_CNT>::init();

            if (data_to_set.get() != 0U)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: incorrect inital data in DataWatch object.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: DataWatch data initial value check.");
            }

            OPEEngineRes_t OPEEres = data_to_set.subscribe<16>(
                [&data_to_set, &evt_grp_data_to_set](opee_uint8_t new_data)
                {
                    static opee_uint8_t execution_count = 0U;

                    execution_count++;

                    if (execution_count == new_data)
                    {
                        OPEEngineTestHelper::print_test_msg(TEST_TAG, "Cb Executed: new_data: %d", new_data);

                        if (new_data == 1U)
                        {
                            xEventGroupSetBits(evt_grp_data_to_set, CB_EXECUTED_BIT);
                            data_to_set.set(2U);
                        }
                        else if (new_data == 2U)
                        {
                            execution_count = 0U;
                            xEventGroupSetBits(evt_grp_data_to_set, CB_EXECUTED_RECURSIVELY_BIT);
                        }
                    }
                });

            if (OPEEres != OPEE_OK)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL:  subscribe() did not return successfully: %s.", OPEEngineRes_to_str(OPEEres));
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS:  successfully subscribed callback.");
            }

            if (data_to_set.set(1U) != OPEE_OK)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL:  set() did not return successfully.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: set data & queue cb check.");
            }

            EventBits_t res = xEventGroupWaitBits(evt_grp_data_to_set, CB_EXECUTED_BIT, pdFALSE, pdFALSE, CB_EXECUTION_TIMEOUT_MS);
            if (!(res & CB_EXECUTED_BIT))
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: first callback execution not detected");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: cb execution check.");
            }

            res = xEventGroupWaitBits(evt_grp_data_to_set, CB_EXECUTED_RECURSIVELY_BIT, pdFALSE, pdFALSE, CB_EXECUTION_TIMEOUT_MS);
            if (!(res & CB_EXECUTED_BIT))
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: recursive callback execution not detected");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: cb recursive execution check.");
            }

            if (data_to_set.get() != 2U)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: incorrect data after callback execution: Actual %d Expected %d", data_to_set.get(), 2U);
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: data check.");
            }

            vEventGroupDelete(evt_grp_data_to_set);

            return true;
        }

        static bool
        data_is_set_after_cbs_test()
        {
            const constexpr char *TEST_TAG = "data_is_set_after_cbs_test";
            const constexpr TickType_t CB_EXECUTION_TIMEOUT_MS = 5UL / portTICK_PERIOD_MS;

            opee::OPEEngine_init();

            SemaphoreHandle_t sem = xSemaphoreCreateBinary();

            opee::DataWatch<opee_uint8_t, 32, 1> data_to_set(0U);
            opee::CbHelper<OPEEconfigMAX_DATA_WATCH_CNT>::init();

            if (data_to_set.get() != 0U)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: incorrect inital data in DataWatch object.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: DataWatch data initial value check.");
            }

            OPEEngineRes_t OPEEres = data_to_set.subscribe<16>(
                [&data_to_set, &sem](opee_uint8_t new_data)
                {
                    opee_uint8_t current_data = data_to_set.get();

                    OPEEngineTestHelper::print_test_msg(TEST_TAG, "Cb Executed: current_data: %d new_data: %d", current_data, new_data);
                    if ((current_data == 0U) && (new_data == 1U))
                    {
                        xSemaphoreGive(sem);
                    }
                });

            if (OPEEres != OPEE_OK)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL:  subscribe() did not return successfully: %s.", OPEEngineRes_to_str(OPEEres));
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS:  successfully subscribed callback.");
            }

            if (data_to_set.set(1U) != OPEE_OK)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL:  set() did not return successfully.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: set data & queue cb check.");
            }

            if (xSemaphoreTake(sem, CB_EXECUTION_TIMEOUT_MS) != pdTRUE)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: callback execution not detected");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: cb execution check.");
            }

            if (data_to_set.get() != 1U)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: incorrect data after callback execution");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: data check.");
            }

            vSemaphoreDelete(sem);

            return true;
        }

    private:
        typedef struct multi_task_cb_test_task_ctx_t
        {
            opee::DataWatch<bool, 128, 3> *_data_to_set;
            EventGroupHandle_t evt_grp_data_to_set;
        } multi_task_cb_test_task_ctx_t;

        static void multi_task_cb_test_task_0(void *arg)
        {
            const constexpr char *TEST_TAG = "multi_task_cb_test";

            const constexpr EventBits_t SUBSCRIBED_BIT = 1U << 0U;
            const constexpr EventBits_t DATA_SET_BIT = 1U << 1U;
            const constexpr EventBits_t CB_EXECUTED_BIT = 1U << 2U;
            const constexpr TickType_t SUB_TIMEOUT_MS = 5UL / portTICK_PERIOD_MS;
            const constexpr TickType_t CB_EXECUTION_TIMEOUT_MS = 5UL / portTICK_PERIOD_MS;

            multi_task_cb_test_task_ctx_t *_ctx = static_cast<multi_task_cb_test_task_ctx_t *>(arg);

            EventBits_t res = xEventGroupWaitBits(_ctx->evt_grp_data_to_set, SUBSCRIBED_BIT, pdFALSE, pdFALSE, SUB_TIMEOUT_MS);

            if (res & SUBSCRIBED_BIT)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "Sub detected...");

                if (_ctx->_data_to_set->set(true))
                    OPEEngineTestHelper::print_test_msg(TEST_TAG, "Set...");

                xEventGroupSetBits(_ctx->evt_grp_data_to_set, DATA_SET_BIT);
            }

            xEventGroupWaitBits(_ctx->evt_grp_data_to_set, CB_EXECUTED_BIT, pdFALSE, pdFALSE, CB_EXECUTION_TIMEOUT_MS);
            OPEEngineTestHelper::print_test_msg(TEST_TAG, "Test task 0 deleting...");
            vTaskDelete(NULL);
        }

        static void multi_task_cb_test_task_1(void *arg)
        {
            const constexpr char *TEST_TAG = "multi_task_cb_test";

            const constexpr EventBits_t SUBSCRIBED_BIT = 1U << 0U;
            const constexpr EventBits_t DATA_SET_BIT = 1U << 1U;
            const constexpr EventBits_t CB_EXECUTED_BIT = 1U << 2U;
            const constexpr TickType_t DATA_SET_TIMEOUT_MS = 5UL / portTICK_PERIOD_MS;
            const constexpr TickType_t CB_EXECUTION_TIMEOUT_MS = 5UL / portTICK_PERIOD_MS;

            multi_task_cb_test_task_ctx_t *_ctx = static_cast<multi_task_cb_test_task_ctx_t *>(arg);

            OPEEngineRes_t OPEEres = _ctx->_data_to_set->subscribe<16>(
                [&_ctx](bool new_data)
                {
                    OPEEngineTestHelper::print_test_msg(TEST_TAG, "CB executed...");
                    xEventGroupSetBits(_ctx->evt_grp_data_to_set, CB_EXECUTED_BIT);
                });

            if (OPEEres == OPEE_OK)
            {
                xEventGroupSetBits(_ctx->evt_grp_data_to_set, SUBSCRIBED_BIT);
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "Subbed...");

                EventBits_t res = xEventGroupWaitBits(_ctx->evt_grp_data_to_set, DATA_SET_BIT, pdFALSE, pdFALSE, DATA_SET_TIMEOUT_MS);

                if (res & DATA_SET_BIT)
                {
                    OPEEngineTestHelper::print_test_msg(TEST_TAG, "Set detected...");
                    xEventGroupWaitBits(_ctx->evt_grp_data_to_set, CB_EXECUTED_BIT, pdFALSE, pdFALSE, CB_EXECUTION_TIMEOUT_MS);
                }
            }

            OPEEngineTestHelper::print_test_msg(TEST_TAG, "Test task 1 deleting...");
            vTaskDelete(NULL);
        }
    };

} // namespace opee

void CallbackTests_single_cb_test()
{
    TEST_ASSERT_EQUAL_MESSAGE(true, opee::CallbackTests::single_cb_test(), "FAILED single_cb_test");
}

void CallbackTests_multi_cb_test()
{
    TEST_ASSERT_EQUAL_MESSAGE(true, opee::CallbackTests::multi_cb_test(), "FAILED multi_cb_test");
}

void CallbackTests_multi_task_cb_test()
{
    TEST_ASSERT_EQUAL_MESSAGE(true, opee::CallbackTests::multi_task_cb_test(), "FAILED multi_task_cb_test");
}

void CallbackTests_self_set_test()
{
    TEST_ASSERT_EQUAL_MESSAGE(true, opee::CallbackTests::self_set_test(), "FAILED self_set_test");
}

void CallbackTests_data_is_set_after_cbs_test()
{
    TEST_ASSERT_EQUAL_MESSAGE(true, opee::CallbackTests::data_is_set_after_cbs_test(), "FAILED data_is_set_after_cbs_test");
}

void CallbackTests_run_all()
{
    RUN_TEST(CallbackTests_single_cb_test);
    RUN_TEST(CallbackTests_multi_cb_test);
    RUN_TEST(CallbackTests_multi_task_cb_test);
    RUN_TEST(CallbackTests_self_set_test);
    RUN_TEST(CallbackTests_data_is_set_after_cbs_test);
}
