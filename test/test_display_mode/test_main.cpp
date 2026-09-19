#include <unity.h>
#include "display_mode.h"

// Pure navigation logic needs no hardware setup.
void setUp(void) {}
void tearDown(void) {}

void test_forward_navigation()
{
    TEST_ASSERT_TRUE(
        nextDisplayMode(DisplayMode::TEMPERATURE) == DisplayMode::HUMIDITY);
    TEST_ASSERT_TRUE(
        nextDisplayMode(DisplayMode::HUMIDITY) == DisplayMode::LIGHT);
    TEST_ASSERT_TRUE(
        nextDisplayMode(DisplayMode::LIGHT) == DisplayMode::MOTION);
}

void test_backward_navigation()
{
    TEST_ASSERT_TRUE(
        previousDisplayMode(DisplayMode::MOTION) == DisplayMode::LIGHT);
    TEST_ASSERT_TRUE(
        previousDisplayMode(DisplayMode::LIGHT) == DisplayMode::HUMIDITY);
    TEST_ASSERT_TRUE(
        previousDisplayMode(DisplayMode::HUMIDITY) == DisplayMode::TEMPERATURE);
}

void test_forward_wraparound()
{
    // Moving forward from the last page returns to the first.
    TEST_ASSERT_TRUE(
        nextDisplayMode(DisplayMode::MOTION) == DisplayMode::TEMPERATURE);
}

void test_backward_wraparound()
{
    // Moving backward from the first page returns to the last.
    TEST_ASSERT_TRUE(
        previousDisplayMode(DisplayMode::TEMPERATURE) == DisplayMode::MOTION);
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(test_forward_navigation);
    RUN_TEST(test_backward_navigation);
    RUN_TEST(test_forward_wraparound);
    RUN_TEST(test_backward_wraparound);

    return UNITY_END();
}