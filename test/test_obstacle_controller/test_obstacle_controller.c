// Host-based unit tests for the obstacle controller.
#include <unity.h>

#include "app/obstacle_controller.h"
#include "interfaces/obstacle_sensor.h"
#include "interfaces/status_indicator.h"

typedef struct {
    bool detected;
} MockSensorContext;

static bool mock_is_detected(void *context)
{
    MockSensorContext *ctx = (MockSensorContext *)context;
    return ctx->detected;
}

typedef struct {
    SystemStatus last_status;
    int call_count;
} MockIndicatorContext;

static void mock_set_status(void *context, SystemStatus status)
{
    MockIndicatorContext *ctx = (MockIndicatorContext *)context;
    ctx->last_status = status;
    ctx->call_count += 1;
}

void setUp(void)
{
}

void tearDown(void)
{
}

static void test_sets_obstacle_when_detected(void)
{
    MockSensorContext sensor_ctx = { .detected = true };
    MockIndicatorContext indicator_ctx = {
        .last_status = SYSTEM_STATUS_CLEAR,
        .call_count = 0
    };

    ObstacleSensor sensor = {
        .is_detected = mock_is_detected,
        .context = &sensor_ctx
    };

    StatusIndicator indicator = {
        .set_status = mock_set_status,
        .context = &indicator_ctx
    };

    ObstacleController controller = {
        .sensor = sensor,
        .indicator = indicator
    };

    obstacle_controller_update(&controller);

    TEST_ASSERT_EQUAL(1, indicator_ctx.call_count);
    TEST_ASSERT_EQUAL(SYSTEM_STATUS_OBSTACLE, indicator_ctx.last_status);
}

static void test_sets_clear_when_not_detected(void)
{
    MockSensorContext sensor_ctx = { .detected = false };
    MockIndicatorContext indicator_ctx = {
        .last_status = SYSTEM_STATUS_OBSTACLE,
        .call_count = 0
    };

    ObstacleSensor sensor = {
        .is_detected = mock_is_detected,
        .context = &sensor_ctx
    };

    StatusIndicator indicator = {
        .set_status = mock_set_status,
        .context = &indicator_ctx
    };

    ObstacleController controller = {
        .sensor = sensor,
        .indicator = indicator
    };

    obstacle_controller_update(&controller);

    TEST_ASSERT_EQUAL(1, indicator_ctx.call_count);
    TEST_ASSERT_EQUAL(SYSTEM_STATUS_CLEAR, indicator_ctx.last_status);
}

static void test_safe_when_unwired(void)
{
    MockSensorContext sensor_ctx = { .detected = true };
    MockIndicatorContext indicator_ctx = {
        .last_status = SYSTEM_STATUS_CLEAR,
        .call_count = 0
    };

    ObstacleSensor sensor = {
        .is_detected = mock_is_detected,
        .context = &sensor_ctx
    };

    // Simulate a partially wired system: indicator function pointer is NULL.
    StatusIndicator indicator = {
        .set_status = NULL,
        .context = &indicator_ctx
    };

    ObstacleController controller = {
        .sensor = sensor,
        .indicator = indicator
    };

    obstacle_controller_update(&controller);

    // Expect no crash and no calls recorded.
    TEST_ASSERT_EQUAL(0, indicator_ctx.call_count);
}

static void test_edge_triggered_only_on_change(void)
{
    MockSensorContext sensor_ctx = { .detected = true };
    MockIndicatorContext indicator_ctx = {
        .last_status = SYSTEM_STATUS_CLEAR,
        .call_count = 0
    };

    ObstacleSensor sensor = {
        .is_detected = mock_is_detected,
        .context = &sensor_ctx
    };

    StatusIndicator indicator = {
        .set_status = mock_set_status,
        .context = &indicator_ctx
    };

    ObstacleController controller = {
        .sensor = sensor,
        .indicator = indicator
    };

    // First update publishes the current state.
    obstacle_controller_update(&controller);
    TEST_ASSERT_EQUAL(1, indicator_ctx.call_count);
    TEST_ASSERT_EQUAL(SYSTEM_STATUS_OBSTACLE, indicator_ctx.last_status);

    // Same input -> no new edge -> no extra call.
    obstacle_controller_update(&controller);
    TEST_ASSERT_EQUAL(1, indicator_ctx.call_count);

    // Change input -> edge -> indicator updates.
    sensor_ctx.detected = false;
    obstacle_controller_update(&controller);
    TEST_ASSERT_EQUAL(2, indicator_ctx.call_count);
    TEST_ASSERT_EQUAL(SYSTEM_STATUS_CLEAR, indicator_ctx.last_status);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_sets_obstacle_when_detected);
    RUN_TEST(test_sets_clear_when_not_detected);
    RUN_TEST(test_safe_when_unwired);
    RUN_TEST(test_edge_triggered_only_on_change);
    return UNITY_END();
}
