// third-party
#include "UnityInterface.h"
#include "unity.h"
// OPEEngine
#include "OPEEngineTestHelper.h"
#include "OPEEngineConfig.h"
#include "OPEEngine_freeRTOS_hook.h"
#include "OPEEngine.h"
#include "DataWatch.h"
#include "Subscriber.h"

namespace opee
{
    class SubscriberInterfaceTests
    {
    public:
        static bool validate_checksum_test()
        {
            const constexpr char *TEST_TAG = "validate_checksum_test";
            const constexpr TickType_t CB_EXECUTION_TIMEOUT_MS = 5UL / portTICK_PERIOD_MS;

            opee::OPEEngine_init();

            SemaphoreHandle_t sem = xSemaphoreCreateBinary();

            opee::Subscriber<bool> sub;
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
                                                               { xSemaphoreGive(sem); }, &sub);

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

            if (sub.validate_checksum() != true)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: validate_checksum() returned false.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: checksum validation check.");
            }
            vSemaphoreDelete(sem);

            return true;
        }

        static bool notify_test()
        {
            const constexpr char *TEST_TAG = "notify_test";
            const constexpr EventBits_t CB_EXECUTED_BIT = 1UL << 0UL;
            const constexpr EventBits_t NOTIFY_DETECT_BIT = 1UL << 1UL;
            const constexpr TickType_t CB_EXECUTION_TIMEOUT_MS = 5UL / portTICK_PERIOD_MS;
            const constexpr TickType_t NOTIFY_DETECT_TIMEOUT_MS = 5UL / portTICK_PERIOD_MS;

            opee::OPEEngine_init();

            EventGroupHandle_t evt_grp_data_to_set = xEventGroupCreate();

            opee::Subscriber<uint8_t> sub;
            opee::DataWatch<uint8_t, 32, 1> data_to_set(0U);
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
                [&evt_grp_data_to_set](uint8_t new_data)
                {
                    OPEEngineTestHelper::print_test_msg(TEST_TAG, "CB Executed: new_data: %d.", new_data);
                    if (new_data == 1U)
                        xEventGroupSetBits(evt_grp_data_to_set, CB_EXECUTED_BIT);
                    else if (new_data == 2U)
                        xEventGroupSetBits(evt_grp_data_to_set, NOTIFY_DETECT_BIT);
                },
                &sub);

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
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: callback never executed");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: cb execution check.");
            }

            if (data_to_set.get() != 1U)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: no new data after cb execution.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: new data after cb execution check.");
            }

            sub.notify(2U);
            res = xEventGroupWaitBits(evt_grp_data_to_set, NOTIFY_DETECT_BIT, pdFALSE, pdFALSE, NOTIFY_DETECT_TIMEOUT_MS);
            if (!(res & NOTIFY_DETECT_BIT))
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: callback never notified");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: cb notify check.");
            }

            if (data_to_set.get() != 1U)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: data changed after cb notification.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: data after cb notification check");
            }

            vEventGroupDelete(evt_grp_data_to_set);

            return true;
        }

        static bool mute_test()
        {
            const constexpr char *TEST_TAG = "mute_test";
            const constexpr TickType_t CB_EXECUTION_TIMEOUT_MS = 5UL / portTICK_PERIOD_MS;

            opee::OPEEngine_init();

            SemaphoreHandle_t sem = xSemaphoreCreateBinary();

            opee::Subscriber<uint8_t> sub;
            opee::DataWatch<uint8_t, 32, 1> data_to_set(0U);
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
                [&sem](uint8_t new_data)
                {
                    OPEEngineTestHelper::print_test_msg(TEST_TAG, "CB Executed: new_data: %d.", new_data);
                    xSemaphoreGive(sem);
                },
                &sub);

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
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: callback never executed");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: cb execution check.");
            }

            if (data_to_set.get() != 1U)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: no new data after cb execution.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: new data after cb execution check.");
            }

            sub.mute();
            data_to_set.set(2U);

            if (xSemaphoreTake(sem, CB_EXECUTION_TIMEOUT_MS) != pdFALSE)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: callback executed in muted state.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: mute check.");
            }

            if (data_to_set.get() != 2U)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: no new data after empty Cb list execution.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: data after mute cb and set check");
            }

            sub.un_mute();
            data_to_set.set(3U);

            if (xSemaphoreTake(sem, CB_EXECUTION_TIMEOUT_MS) != pdTRUE)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: callback callback never executed after unmute.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: unmute check.");
            }

            if (data_to_set.get() != 3U)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: no new data after unmute.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: data after unmute cb and set check");
            }

            vSemaphoreDelete(sem);

            return true;
        }

        static bool mute_multi_cb_test()
        {
            const constexpr char *TEST_TAG = "mute_multi_cb_test";
            const constexpr EventBits_t CB_0_EXECUTED_BIT = 1UL << 0UL;
            const constexpr EventBits_t CB_1_EXECUTED_BIT = 1UL << 1UL;
            const constexpr EventBits_t CB_2_EXECUTED_BIT = 1UL << 2UL;
            const constexpr EventBits_t ALL_CB_EXECUTED_BITS = (CB_0_EXECUTED_BIT | CB_1_EXECUTED_BIT | CB_2_EXECUTED_BIT);
            const constexpr TickType_t CB_EXECUTION_TIMEOUT_MS = 5UL / portTICK_PERIOD_MS;

            opee::OPEEngine_init();

            EventGroupHandle_t evt_grp_data_to_set = xEventGroupCreate();
            OPEEngineRes_t OPEEres;

            opee::Subscriber<uint8_t> subs[3];
            opee::DataWatch<uint8_t, 64, 3> data_to_set(0U);
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

            OPEEres = data_to_set.subscribe<16>(
                [&evt_grp_data_to_set](uint8_t new_data)
                {
                    OPEEngineTestHelper::print_test_msg(TEST_TAG, "CB_0 Executed: new_data: %d.", new_data);
                    xEventGroupSetBits(evt_grp_data_to_set, CB_0_EXECUTED_BIT);
                },
                &subs[0]);

            if (OPEEres != OPEE_OK)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL:  subscribe() did not return successfully on CB_0: %s.", OPEEngineRes_to_str(OPEEres));
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS:  successfully subscribed CB_0.");
            }

            OPEEres = data_to_set.subscribe<16>(
                [&evt_grp_data_to_set](uint8_t new_data)
                {
                    OPEEngineTestHelper::print_test_msg(TEST_TAG, "CB_1 Executed: new_data: %d.", new_data);
                    xEventGroupSetBits(evt_grp_data_to_set, CB_1_EXECUTED_BIT);
                },
                &subs[1]);

            if (OPEEres != OPEE_OK)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL:  subscribe() did not return successfully on CB_1: %s.", OPEEngineRes_to_str(OPEEres));
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS:  successfully subscribed CB_1.");
            }

            OPEEres = data_to_set.subscribe<16>(
                [&evt_grp_data_to_set](uint8_t new_data)
                {
                    OPEEngineTestHelper::print_test_msg(TEST_TAG, "CB_2 Executed: new_data: %d.", new_data);
                    xEventGroupSetBits(evt_grp_data_to_set, CB_2_EXECUTED_BIT);
                },
                &subs[2]);

            if (OPEEres != OPEE_OK)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL:  subscribe() did not return successfully on CB_2: %s.", OPEEngineRes_to_str(OPEEres));
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS:  successfully subscribed CB_2.");
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

            EventBits_t res = xEventGroupWaitBits(evt_grp_data_to_set, ALL_CB_EXECUTED_BITS, pdFALSE, pdTRUE, CB_EXECUTION_TIMEOUT_MS);
            if (!(res & ALL_CB_EXECUTED_BITS))
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: not all callbacks executed. CB_EXECUTED_BITS: %ld", res);
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: cb execution check.");
            }

            if (data_to_set.get() != 1U)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: no new data after cb execution.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: new data after cb execution check.");
            }

            xEventGroupClearBits(evt_grp_data_to_set, ALL_CB_EXECUTED_BITS);
            subs[0].mute();

            if (data_to_set.set(2U) != OPEE_OK)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL:  set() did not return successfully.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: set data & queue cb check.");
            }

            res = xEventGroupWaitBits(evt_grp_data_to_set, ALL_CB_EXECUTED_BITS, pdFALSE, pdTRUE, CB_EXECUTION_TIMEOUT_MS);
            if (!(res & (CB_1_EXECUTED_BIT | CB_2_EXECUTED_BIT)) || (res & CB_0_EXECUTED_BIT))
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: not all callbacks or muted callback executed. CB_EXECUTED_BITS: %ld", res);
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: cb execution check.");
            }

            if (data_to_set.get() != 2U)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: no new data after cb execution.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: new data after cb execution check.");
            }

            xEventGroupClearBits(evt_grp_data_to_set, ALL_CB_EXECUTED_BITS);
            subs[2].mute();

            if (data_to_set.set(3U) != OPEE_OK)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL:  set() did not return successfully.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: set data & queue cb check.");
            }

            res = xEventGroupWaitBits(evt_grp_data_to_set, ALL_CB_EXECUTED_BITS, pdFALSE, pdTRUE, CB_EXECUTION_TIMEOUT_MS);
            if (!(res & CB_1_EXECUTED_BIT) || (res & (CB_0_EXECUTED_BIT | CB_2_EXECUTED_BIT)))
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: not all callbacks or muted callback executed. CB_EXECUTED_BITS: %ld", res);
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: cb execution check.");
            }

            if (data_to_set.get() != 3U)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: no new data after cb execution.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: new data after cb execution check.");
            }

            xEventGroupClearBits(evt_grp_data_to_set, ALL_CB_EXECUTED_BITS);
            subs[1].mute();
            subs[0].un_mute();
            subs[2].un_mute();

            if (data_to_set.set(4U) != OPEE_OK)
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL:  set() did not return successfully.");
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: set data & queue cb check.");
            }

            res = xEventGroupWaitBits(evt_grp_data_to_set, ALL_CB_EXECUTED_BITS, pdFALSE, pdTRUE, CB_EXECUTION_TIMEOUT_MS);
            if (!(res & (CB_0_EXECUTED_BIT | CB_2_EXECUTED_BIT) || (res & CB_1_EXECUTED_BIT)))
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "FAIL: not all callbacks or muted callback executed. CB_EXECUTED_BITS: %ld", res);
                return false;
            }
            else
            {
                OPEEngineTestHelper::print_test_msg(TEST_TAG, "PASS: cb execution check.");
            }

            if (data_to_set.get() != 4U)
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
    };

} // namespace opee

void SubscriberInterfaceTests_validate_checksum_test()
{
    TEST_ASSERT_EQUAL_MESSAGE(true, opee::SubscriberInterfaceTests::validate_checksum_test(), "FAILED validate_checksum_test");
}

void SubscriberInterfaceTests_notify_test()
{
    TEST_ASSERT_EQUAL_MESSAGE(true, opee::SubscriberInterfaceTests::notify_test(), "FAILED notify_test");
}

void SubscriberInterfaceTests_mute_test()
{
    TEST_ASSERT_EQUAL_MESSAGE(true, opee::SubscriberInterfaceTests::mute_test(), "FAILED mute_test");
}

void SubscriberInterfaceTests_mute_multi_cb_test()
{
    TEST_ASSERT_EQUAL_MESSAGE(true, opee::SubscriberInterfaceTests::mute_multi_cb_test(), "FAILED mute_multi_cb_test");
}

void SubscriberInterfaceTests_run_all()
{
    RUN_TEST(SubscriberInterfaceTests_validate_checksum_test);
    RUN_TEST(SubscriberInterfaceTests_notify_test);
    RUN_TEST(SubscriberInterfaceTests_mute_test);
    RUN_TEST(SubscriberInterfaceTests_mute_multi_cb_test);
}