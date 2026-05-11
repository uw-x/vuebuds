#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include "nrf_nvic.h"
#include "app_error.h"
#include "event.h"
#include "nrf_log.h"

#define EVENT_QUEUE_DEPTH 100

static uint8_t head = 0;
static uint8_t tail = 0;
static event_t eventQueue[EVENT_QUEUE_DEPTH] = {0};

void eventQueueInit(void) { head = tail = 0; }
bool eventQueueEmpty(void) { return (head == tail); }
event_t eventQueueFront(void) { return eventQueue[head]; }

event_t eventQueuePop(void)
{
  // uint8_t dummy = 0;
  // uint32_t err_code = sd_nvic_critical_region_enter(&dummy);
  // if (err_code != NRF_SUCCESS) {
  //   NRF_LOG_RAW_INFO("lock error 3 %d\n", err_code);
  // }
  // APP_ERROR_CHECK(err_code);

  event_t event = eventQueue[head];
  head = (head + 1) % EVENT_QUEUE_DEPTH;

  // err_code = sd_nvic_critical_region_exit(0);
  // if (err_code != NRF_SUCCESS) {
  //   NRF_LOG_RAW_INFO("lock error 4 %d\n", err_code);
  // }
  // APP_ERROR_CHECK(err_code);

  return event;
}

void eventQueuePush(event_t event)
{
  // uint8_t dummy = 0;
  // uint32_t err_code = sd_nvic_critical_region_enter(&dummy);
  // if (err_code != NRF_SUCCESS) {
  //   NRF_LOG_RAW_INFO("lock error 5 %d\n", err_code);
  // }

  // APP_ERROR_CHECK(err_code);

  eventQueue[eventQueueEmpty() ? head : tail]  = event;
  tail = (tail + 1) % EVENT_QUEUE_DEPTH;
  assert(head != tail); // overflow

  // err_code = sd_nvic_critical_region_exit(0);
  // if (err_code != NRF_SUCCESS) {
  //   NRF_LOG_RAW_INFO("lock error 6 %d\n", err_code);
  // }
  // APP_ERROR_CHECK(err_code);
}