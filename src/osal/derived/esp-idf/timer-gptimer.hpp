#pragma once
#include "timer-base.hpp"
#include "driver/gptimer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <atomic>
#include <functional>

class TimerGptimer : public TimerBase<TimerGptimer>
{
public:
  TimerGptimer() = default;
  ~TimerGptimer()
  {
    if (handle_) {
      gptimer_stop(handle_);
      gptimer_disable(handle_);
      gptimer_del_timer(handle_);
      handle_ = nullptr;
    }
  }

  TimerGptimer(const TimerGptimer&) = delete;
  TimerGptimer& operator=(const TimerGptimer&) = delete;

  void InitializeImpl(unsigned period_ms, bool auto_reload)
  {
    period_us_ = period_ms * 1000;
    auto_reload_ = auto_reload;
    if (!handle_) {
      gptimer_config_t cfg{};
      cfg.clk_src = GPTIMER_CLK_SRC_DEFAULT;
      cfg.direction = GPTIMER_COUNT_UP;
      cfg.resolution_hz = 1000000;
      gptimer_new_timer(&cfg, &handle_);
      gptimer_event_callbacks_t cbs = { .on_alarm = &OnAlarm };
      gptimer_register_event_callbacks(handle_, &cbs, this);
      gptimer_enable(handle_);
    }
    gptimer_alarm_config_t alarm{};
    alarm.reload_count = 0;
    alarm.alarm_count = period_us_;
    alarm.flags.auto_reload_on_alarm = auto_reload_;
    gptimer_set_alarm_action(handle_, &alarm);
  }

  void StartImpl()
  {
    if (!handle_) return;
    running_.store(true);
    gptimer_start(handle_);
  }

  void StopImpl()
  {
    if (!handle_) return;
    gptimer_stop(handle_);
    running_.store(false);
  }

  void ResetImpl()
  {
    if (!handle_) return;
    gptimer_stop(handle_);
    gptimer_set_raw_count(handle_, 0);
    gptimer_start(handle_);
  }

  void SetPeriodImpl(unsigned ms)
  {
    period_us_ = ms * 1000;
    if (!handle_) return;
    gptimer_alarm_config_t alarm{};
    alarm.reload_count = 0;
    alarm.alarm_count = period_us_;
    alarm.flags.auto_reload_on_alarm = auto_reload_;
    gptimer_set_alarm_action(handle_, &alarm);
  }

  unsigned GetPeriodImpl()
  {
    return period_us_ / 1000;
  }

  bool IsRunningImpl()
  {
    return running_.load();
  }

  void TickImpl()
  {
    if (cb_) cb_();
    else if (c_cb_) c_cb_(c_user_);
  }

  void StartOnceImpl()
  {
    auto_reload_ = false;
    SetPeriodImpl(GetPeriodImpl());
    StartImpl();
  }

  void SetCallbackImpl(std::function<void()> cb)
  {
    cb_ = std::move(cb);
    c_cb_ = nullptr;
    c_user_ = nullptr;
    EnsureWorker();
  }

  void SetCallbackImpl(void (*cb)(void*), void* user)
  {
    c_cb_ = cb;
    c_user_ = user;
    cb_ = {};
  }

private:
  static void WorkerTask(void* pv)
  {
    auto* self = static_cast<TimerGptimer*>(pv);
    for (;;)
    {
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
      if (!self) continue;
      if (self->cb_) self->cb_();
    }
  }

  void EnsureWorker()
  {
    if (!worker_)
    {
      xTaskCreate(WorkerTask, "gpt_cb", 4096, this, 5, &worker_);
    }
  }

  static bool IRAM_ATTR OnAlarm(gptimer_handle_t, const gptimer_alarm_event_data_t*, void* user)
  {
    auto* self = static_cast<TimerGptimer*>(user);
    if (!self) return false;
    BaseType_t hpw = pdFALSE;
    if (self->c_cb_) self->c_cb_(self->c_user_);
    else if (self->worker_) vTaskNotifyGiveFromISR(self->worker_, &hpw);
    return hpw == pdTRUE;
  }

  gptimer_handle_t handle_{nullptr};
  unsigned period_us_{0};
  bool auto_reload_{false};
  std::atomic<bool> running_{false};
  std::function<void()> cb_{};
  void (*c_cb_)(void*){nullptr};
  void* c_user_{nullptr};
  TaskHandle_t worker_{nullptr};
};
