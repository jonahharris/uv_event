#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>
#include <uv.h>

#include "uv_event.h"

#define NUM_THREADS 4
uv_waitevent_t *globalWaitEvent;

static void
worker (
  void                         *arg
) {

  printf("Waiting!!!!\n");

  uv_waitevent_wait_single(globalWaitEvent, -1);

  printf("Reading!!!!\n");
}


int
main (
  int                           argc,
  char                        **argv
) {
  uv_thread_t threads[4];

  //globalWaitEvent = uv_waitevent_create(true, false);
  globalWaitEvent = uv_waitevent_create(false, false);

  for (int ii = 0; ii < NUM_THREADS; ++ii) {
    uv_thread_create(&threads[ii], worker, NULL);
  }

  sleep(2);

  printf("set 1\n");
  uv_waitevent_set(globalWaitEvent);
  sleep(1);
  printf("set 2\n");
  uv_waitevent_set(globalWaitEvent);
  sleep(1);
  printf("set 3\n");
  uv_waitevent_set(globalWaitEvent);
  sleep(1);
  printf("set 4\n");
  uv_waitevent_set(globalWaitEvent);
  sleep(1);
  printf("set 5\n");
  uv_waitevent_set(globalWaitEvent);
  sleep(1);

  sleep(1);

  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  return 0;
}

