/* ========================================================================= **
**                                                         __                **
**                 __  ___   __      ___ _   _____  ____  / /_               **
**                / / / / | / /_____/ _ \ | / / _ \/ __ \/ __/               **
**               / /_/ /| |/ /_____/  __/ |/ /  __/ / / / /_                 **
**               \__,_/ |___/      \___/|___/\___/_/ /_/\__/                 **
**                                                                           **
** ========================================================================= **
**                      SIMPLE EVENT OBJECTS FOR LIBUV                       **
** ========================================================================= **
**                                                                           **
** Copyright (c) 2016-2018 Jonah H. Harris.                                  **
**                                                                           **
** This library is free software; you can redistribute it and/or modify it   **
** under the terms of the GNU Lesser General Public License as published by  **
** the Free Software Foundation; either version 3 of the License, or (at     **
** your option) any later version.                                           **
**                                                                           **
** This library is distributed in the hope it will be useful, but WITHOUT    **
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or     **
** FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public       **
** License for more details.                                                 **
**                                                                           **
** You should have received a copy of the GNU Lesser General Public License  **
** along with this library; if not, write to the Free Software Foundation,   **
** Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                             **
** ========================================================================= */

/* ========================================================================= */
/* -- INCLUSIONS ----------------------------------------------------------- */
/* ========================================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#include "uv_event.h"

/* ========================================================================= */
/* -- DEFINITIONS ---------------------------------------------------------- */
/* ========================================================================= */

/* ========================================================================= */
/* -- MACROS --------------------------------------------------------------- */
/* ========================================================================= */

/* ========================================================================= */
/* -- PRIVATE TYPES -------------------------------------------------------- */
/* ========================================================================= */

struct uv_waitevent_t {
  uv_cond_t                     condition_variable;
  uv_mutex_t                    mutex;
  bool                          requires_manual_reset;
  bool                          is_set;
};

/* ========================================================================= */
/* -- PRIVATE METHOD PROTOTYPES -------------------------------------------- */
/* ========================================================================= */

/* ========================================================================= */
/* -- PRIVATE DATA --------------------------------------------------------- */
/* ========================================================================= */

/* ========================================================================= */
/* -- PUBLIC DATA ---------------------------------------------------------- */
/* ========================================================================= */

/* ========================================================================= */
/* -- EXTERNAL DATA -------------------------------------------------------- */
/* ========================================================================= */

/* ========================================================================= */
/* -- EXTERNAL FUNCTION PROTOTYPES ----------------------------------------- */
/* ========================================================================= */

/* ========================================================================= */
/* -- STATIC ASSERTIONS ---------------------------------------------------- */
/* ========================================================================= */

/* ========================================================================= */
/* -- PRIVATE METHODS ------------------------------------------------------ */
/* ========================================================================= */

/* ========================================================================= */
/* -- PUBLIC METHODS ------------------------------------------------------- */
/* ========================================================================= */

uv_waitevent_t *
uv_waitevent_create (
  bool                          requires_manual_reset,
  bool                          should_set_on_creation
) {
  uv_waitevent_t *event = calloc(1, sizeof(*event));
  int rc = 0;

  rc = uv_cond_init(&event->condition_variable);
  assert(0 == rc);

  rc = uv_mutex_init(&event->mutex);
  assert(0 == rc);

  event->requires_manual_reset = requires_manual_reset;
  event->is_set = false;

  if (true == should_set_on_creation) {
    rc = uv_waitevent_set(event);
    assert(0 == rc);
  }

  return event;
} /* uv_waitevent_create() */

/* ------------------------------------------------------------------------- */

int
uv_waitevent_set (
  uv_waitevent_t               *event
) {
  int rc = 0;

  uv_mutex_lock(&event->mutex);
  event->is_set = true;
  uv_mutex_unlock(&event->mutex);

  if (true == event->requires_manual_reset) {
    uv_cond_broadcast(&event->condition_variable);
  } else {
    uv_cond_signal(&event->condition_variable);
  }

  return 0;
} /* uv_waitevent_set() */

/* ------------------------------------------------------------------------- */

int
uv_waitevent_wait_single (
  uv_waitevent_t               *event,
  uint64_t                      timeout_in_milliseconds
) {
  int rc = 0;

  if (0 == timeout_in_milliseconds) {
    rc = uv_mutex_trylock(&event->mutex);
    if (EBUSY == -rc) {
      return EBUSY;
    }
  } else {
    uv_mutex_lock(&event->mutex);
  }

  assert(0 == rc);

  if (false == event->is_set) {
    if (timeout_in_milliseconds != (uint64_t) -1) {
      rc = uv_cond_timedwait(&event->condition_variable, &event->mutex,
        (timeout_in_milliseconds * 1000 * 1000));
    } else {
      uv_cond_wait(&event->condition_variable, &event->mutex);
    }

    if (0 == rc && false == event->requires_manual_reset) {
      event->is_set = false;
    }
  } else if (false == event->requires_manual_reset) {
    rc = 0;
    event->is_set = false;
  }

  uv_mutex_unlock(&event->mutex);

  return rc;
} /* uv_waitevent_set() */

/* :vi set ts=2 et sw=2: */

