#include <unity.h>
#include "alarm.h"

// No hardware setup or cleanup is needed.
void setUp(void) {}
void tearDown(void) {}

void test_below_lower_limit()
{
    TEST_ASSERT_TRUE(
        evaluateTemperature(17.9f) == AlarmState::LOW_TEMPERATURE);
}

void test_at_lower_limit()
{
    // Exactly 18 C is inside the normal range.
    TEST_ASSERT_TRUE(
        evaluateTemperature(18.0f) == AlarmState::NORMAL);
}

void test_normal_temperature()
{
    TEST_ASSERT_TRUE(
        evaluateTemperature(24.0f) == AlarmState::NORMAL);
}

void test_at_upper_limit()
{
    // Exactly 30 C is also inside the normal range.
    TEST_ASSERT_TRUE(
        evaluateTemperature(30.0f) == AlarmState::NORMAL);
}

void test_above_upper_limit()
{
    TEST_ASSERT_TRUE(
        evaluateTemperature(30.1f) == AlarmState::HIGH_TEMPERATURE);
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(test_below_lower_limit);
    RUN_TEST(test_at_lower_limit);
    RUN_TEST(test_normal_temperature);
    RUN_TEST(test_at_upper_limit);
    RUN_TEST(test_above_upper_limit);

    return UNITY_END();
}