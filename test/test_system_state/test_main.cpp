#include <unity.h>        // Test runner and assertions.
#include "system_state.h" // The real function we are testing.

// Unity calls these before and after each test.
// Our pure function needs no setup or cleanup.
void setUp(void) {}
void tearDown(void) {}

void test_active_before_timeout()
{
    TEST_ASSERT_TRUE(
        determineSystemState(false, 14999) == SystemState::ACTIVE);
}

void test_inactive_at_timeout()
{
    TEST_ASSERT_TRUE(
        determineSystemState(false, 15000) == SystemState::INACTIVE);
}

void test_inactive_after_timeout()
{
    TEST_ASSERT_TRUE(
        determineSystemState(false, 15001) == SystemState::INACTIVE);
}

void test_motion_overrides_timeout()
{
    TEST_ASSERT_TRUE(
        determineSystemState(true, 15001) == SystemState::ACTIVE);
}

// PC tests start here instead of ESP-IDF's app_main().
int main()
{
    UNITY_BEGIN();

    RUN_TEST(test_active_before_timeout);
    RUN_TEST(test_inactive_at_timeout);
    RUN_TEST(test_inactive_after_timeout);
    RUN_TEST(test_motion_overrides_timeout);

    return UNITY_END();
}