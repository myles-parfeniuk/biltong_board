/**
 * @file OPEEngineTestSuite.hpp
 * @author Myles Parfeniuk
 *
 *
 * @warning YOU MUST ADD THE FOLLOWING LINE TO YOUR MAIN PROJECTS CMakeLists.txt IN ORDER FOR THIS TEST SUITE TO BE BUILT WITH PROJECT:
 *          set(TEST_COMPONENTS "OPEEngine" CACHE STRING "Components to test.")
 */
#pragma once

// third party
#include "UnityInterface.h"
#include "unity.h"
// OPEEngine
#include "CallbackTests.h"
#include "PoolManagerTests.h"
#include "SubscriberInterfaceTests.h"

/**
 * @class OPEEngineTestSuite
 *
 * @brief OPEEngineTestSuite unit test launch point class.
 * */
class OPEEngineTestSuite
{
private:
    static void print_begin_tests_banner(const char *test_set_name)
    {
        char banner_buff[150];
        sprintf(banner_buff, "####################### BEGIN TESTS: %s #######################", test_set_name);
        UnityPrint(banner_buff);
        UNITY_OUTPUT_CHAR('\n');
    }

    static void print_end_tests_banner(const char *test_set_name)
    {
        char banner_buff[150];
        sprintf(banner_buff, "####################### END TESTS: %s #######################", test_set_name);
        UnityPrint(banner_buff);
        UNITY_OUTPUT_CHAR('\n');
    }

public:
    static void run_all_tests()
    {
        UNITY_BEGIN();
        run_pool_manager_tests(false);
        run_cb_tests(false);
        run_sub_interface_tests(false);
        UNITY_END();
    }

    static void run_cb_tests(bool call_unity_end_begin = true)
    {
        static const constexpr char *TESTS_TAG = "cb_tests";
        static const constexpr char *TEST_FILE = "CallbackTests.cpp";

        print_begin_tests_banner(TESTS_TAG);

        if (call_unity_end_begin)
        {
            Unity.TestFile = TEST_FILE;
            UnityBegin(Unity.TestFile);
        }
        else
        {
            UnitySetTestFile(TEST_FILE);
        }

        CallbackTests_run_all();

        if (call_unity_end_begin)
            UNITY_END();

        print_end_tests_banner(TESTS_TAG);
    }

    static void run_pool_manager_tests(bool call_unity_end_begin = true)
    {
        static const constexpr char *TESTS_TAG = "pool_manager_tests";
        static const constexpr char *TEST_FILE = "PoolManagerTests.cpp";

        print_begin_tests_banner(TESTS_TAG);

        if (call_unity_end_begin)
        {
            Unity.TestFile = TEST_FILE;
            UnityBegin(Unity.TestFile);
        }
        else
        {
            UnitySetTestFile(TEST_FILE);
        }

        PoolManagerTests_run_all();

        if (call_unity_end_begin)
            UNITY_END();

        print_end_tests_banner(TESTS_TAG);
    }

    static void run_sub_interface_tests(bool call_unity_end_begin = true)
    {
        static const constexpr char *TESTS_TAG = "sub_interface_tests";
        static const constexpr char *TEST_FILE = "SubscriberInterfaceTests.cpp";

        print_begin_tests_banner(TESTS_TAG);

        if (call_unity_end_begin)
        {
            Unity.TestFile = TEST_FILE;
            UnityBegin(Unity.TestFile);
        }
        else
        {
            UnitySetTestFile(TEST_FILE);
        }

        SubscriberInterfaceTests_run_all();

        if (call_unity_end_begin)
            UNITY_END();

        print_end_tests_banner(TESTS_TAG);
    }
};