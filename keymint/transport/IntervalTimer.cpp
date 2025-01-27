/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/******************************************************************************
 *
 *  The original Work has been changed by NXP.
 *
 *  Copyright 2019, 2023 NXP
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

/*
 *  Asynchronous interval timer.
 */
#define LOG_TAG "IntervalTimer"

#include <android-base/stringprintf.h>
#include <android-base/logging.h>

#include <IntervalTimer.h>

using android::base::StringPrintf;

IntervalTimer::IntervalTimer() {
  mTimerId = 0;
  mCb = NULL;
}

bool IntervalTimer::set(int ms, void* ptr, TIMER_FUNC cb) {
  if (mTimerId == 0) {
    if (cb == NULL) return false;

    if (!create(ptr,cb)) return false;
  }
  if (cb != mCb) {
    kill();
    if (!create(ptr,cb)) return false;
  }

  int stat = 0;
  struct itimerspec ts;
  ts.it_value.tv_sec = ms / 1000;
  ts.it_value.tv_nsec = (ms % 1000) * 1000000;

  ts.it_interval.tv_sec = 0;
  ts.it_interval.tv_nsec = 0;

  stat = timer_settime(mTimerId, 0, &ts, 0);
  if (stat == -1) LOG(ERROR) << StringPrintf("fail set timer");
  return stat == 0;
}

IntervalTimer::~IntervalTimer() { kill(); }

void IntervalTimer::kill() {
  if (mTimerId == 0) return;

  timer_delete(mTimerId);
  mTimerId = 0;
  mCb = NULL;
}

bool IntervalTimer::create(void* ptr , TIMER_FUNC cb) {
  struct sigevent se;
  int stat = 0;

  /*
   * Set the sigevent structure to cause the signal to be
   * delivered by creating a new thread.
   */
  se.sigev_notify = SIGEV_THREAD;
  //se.sigev_value.sival_ptr = &mTimerId;
  se.sigev_value.sival_ptr = ptr;
  se.sigev_notify_function = cb;
  se.sigev_notify_attributes = NULL;
#ifdef NXP_EXTNS
  se.sigev_signo = 0;
#endif
  mCb = cb;
  stat = timer_create(CLOCK_BOOTTIME_ALARM, &se, &mTimerId);
  if (stat == -1) LOG(ERROR) << StringPrintf("fail create timer");
  return stat == 0;
}
