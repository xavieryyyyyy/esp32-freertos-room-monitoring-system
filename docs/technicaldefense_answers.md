# BCA152 Oral Defense Answers

Course: BCA152 Microcontrollers · MSU-IIT · Asst. Prof. Paul Rodolf P. Castor, M.Sc.
Student: Yestin Paraguya
Project: Real-Time Multisensor Room Monitoring System with FreeRTOS

---

### Q1: Why did you create `SensorTask`?

The DHT22 sensor uses a 1-Wire protocol that blocks the CPU for up to 25 ms while decoding 40 electrical pulses on GPIO 4. The LDR on GPIO 34 also needs an ADC read. If this slow I/O ran inside the display or main loop, the entire system would freeze — missing knob turns and motion events.

By isolating sensor acquisition into its own Priority 1 task, it does its slow work independently and sleeps for ~1975 ms each cycle via `vTaskDelayUntil()`, leaving the CPU free for higher-priority tasks.

**Key code:**
- `src/main.cpp:48-50` — `xTaskCreate(sensorTask, "SensorTask", 2048, nullptr, 1, nullptr)`
- `src/sensors.cpp:42-43` — `dht_read_float_data()` on GPIO 4
- `src/sensors.cpp:52` — `adc_oneshot_read()` on ADC1 Ch6 (GPIO 34)
- `src/sensors.cpp:97` — `vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(2000))`

---

### Q2: Why does each task have its assigned priority?

Priorities follow deadline monotonic principles — tasks with shorter response deadlines get higher priority:

- **Priority 3** (`InputTask`, `MotionTask`): Encoder clicks on GPIO 18/19 and PIR pulses on GPIO 27 last only milliseconds. Missing them means lost inputs.
- **Priority 2** (`AlarmTask`, `StateTask`): Buzzer control on GPIO 25 and timeout checks take < 0.5 ms. Important but not microsecond-critical.
- **Priority 1** (`SensorTask`, `DisplayTask`): Slow I/O — DHT22 takes 25 ms, OLED I2C refresh takes 10-15 ms. These must be preemptible so they never block user inputs.

**Key code:** `src/main.cpp:48-75` — all six `xTaskCreate()` calls with priority numbers (1, 1, 3, 2, 3, 2).

---

### Q3: What does `vTaskDelayUntil()` do?

`vTaskDelay(N)` pauses relative to when called — if sensor reading takes 30 ms, the total cycle becomes 2030 ms, and timing drifts over time.

`vTaskDelayUntil(&lastWakeTime, N)` calculates an absolute tick target. It automatically subtracts execution time from the delay, keeping intervals exactly at 2000 ms regardless of how long the work took. Zero cumulative drift.

**Key code:**
- `src/sensors.cpp:35` — `TickType_t lastWakeTime = xTaskGetTickCount()` (captures baseline)
- `src/sensors.cpp:97` — `vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(2000))` (absolute 2-second target)

**Proof:** Serial logs show timestamps advancing by exactly 2000 ms each cycle.

---

### Q4: What happens to a task while it is delayed?

The task enters the **Blocked state** — it consumes 0% CPU. The kernel removes its TCB (Task Control Block) from `pxReadyTasksLists` and places it onto `xDelayedTaskList`. Its registers and stack pointer are preserved in SRAM.

When the tick counter reaches the wake target, the scheduler moves the TCB back to the Ready list. While all tasks are blocked, the Idle Task runs and executes the `waiti` instruction to save power.

**Key code:**
- `src/sensors.cpp:97` — `vTaskDelayUntil()` (blocks for ~1975 ms per cycle)
- `src/input.cpp:68` — `xQueueReceive(encoderQueue, &clockwise, portMAX_DELAY)` (blocks indefinitely until knob turns)

**Proof:** Fault Experiment 1 — removing the delay starved `IDLE0` and the Task Watchdog crashed the system at 5.2 seconds.

---

### Q5: What information crosses your queue?

Four queues, each carrying a specific data type:

| Queue | Data | Size | Sender → Receiver |
|-------|------|------|--------------------|
| `sensorQueue` | `SensorData` struct (temp, humidity, light) | 12 bytes × 5 items | `SensorTask` → `DisplayTask` |
| `alarmQueue` | `float` temperature | 4 bytes × 1 item | `SensorTask` → `AlarmTask` |
| `displayModeQueue` | `DisplayMode` enum | 4 bytes × 1 item | `InputTask` → `DisplayTask` |
| `encoderQueue` | `bool` direction | 1 byte × 16 items | GPIO ISR → `InputTask` |

**Key code:**
- `include/sensor_data.h:4-8` — `SensorData` struct definition
- `src/main.cpp:26-35` — `xQueueCreate()` calls
- `src/sensors.cpp:47` — `xQueueOverwrite(alarmQueue, &temperature)` (only freshest value)

---

### Q6: Why did you use a queue rather than global variables?

A global variable creates a **data race**. On the dual-core ESP32, writing a 12-byte `SensorData` struct takes three separate 32-bit store instructions. If `SensorTask` gets preempted mid-write, `DisplayTask` reads a **torn struct** — new temperature with old humidity.

FreeRTOS queues wrap memory copies inside critical sections and SMP spinlocks, making transfers atomic. They also provide buffer depth — if `DisplayTask` is busy drawing on I2C, `sensorQueue` holds up to 5 readings without data loss.

**Key code:**
- `src/sensors.cpp:88` — `xQueueSend(sensorQueue, &readings, 0)` (atomic copy in)
- `src/display.cpp:83` — `xQueueReceive(sensorQueue, &readings, ...)` (atomic copy out)
- `include/rtos_objects.h` — only queue handles are shared, never raw structs

---

### Q7: What resource does your mutex protect?

The **UART0 serial port** (GPIO 1 TX / GPIO 3 RX). When `SensorTask` prints a multi-line report and `AlarmTask` interrupts to print an alarm warning, characters from both tasks mix inside the UART hardware FIFO buffer, producing garbled output.

`serialMutex` ensures each task finishes its complete message before another can write. It also implements **Priority Inheritance** — if a low-priority task holds the mutex and a high-priority task wants it, the low-priority task temporarily inherits high priority to finish quickly.

**Key code:**
- `src/main.cpp:42-45` — `xSemaphoreCreateMutex()`
- `src/sensors.cpp:56-76` — `xSemaphoreTake/Give` around print blocks
- `src/alarm.cpp:47-64` — same pattern for alarm messages

**Proof:** Fault Experiment 3 — removing the mutex produced interleaved serial output.

---

### Q8: Where could a race condition occur?

Three critical points, all protected:

1. **Shared UART0 FIFO** — concurrent prints garble output → protected by `serialMutex`
2. **12-byte telemetry struct** — torn read across cores → protected by pass-by-value queues
3. **System state flags** — motion/inactive bits → protected by atomic Event Group operations

**Key code:**
- `src/sensors.cpp:56` — `xSemaphoreTake(serialMutex, portMAX_DELAY)`
- `src/sensors.cpp:88` — `xQueueSend(sensorQueue, &readings, 0)`
- `src/motion.cpp:32-34` — `xEventGroupSetBits/ClearBits`

**Proof:** Fault Experiment 3 — commenting out the mutex on lines 56 and 76 immediately scrambled the serial output.

---

### Q9: What does your event group represent?

`systemEvents` holds two atomic bit flags representing room and screen state:

- **BIT0 (`EVENT_MOTION`)**: 1 = PIR sensor on GPIO 27 detects someone moving; 0 = room is quiet.
- **BIT1 (`EVENT_INACTIVE`)**: 1 = 15 continuous seconds without motion, screen should turn off; 0 = screen stays on.

`MotionTask` sets/clears BIT0 based on GPIO 27. `StateTask` monitors BIT0 and sets BIT1 after 15000 ms of no motion. `DisplayTask` reads BIT1 non-destructively and blanks the OLED if set.

**Key code:**
- `include/rtos_objects.h:18-22` — `#define EVENT_MOTION (1 << 0)` and `EVENT_INACTIVE (1 << 1)`
- `src/motion.cpp:31-35` — set/clear BIT0
- `src/state_task.cpp:37-41` — set/clear BIT1
- `src/display.cpp:45-54` — read BIT1 to control OLED power

---

### Q10: Which task owns the OLED, and why?

**`DisplayTask` is the sole owner.** The SSD1306 is connected via I2C on GPIO 21 (SDA) and GPIO 22 (SCL) at 400 kHz. Refreshing the 1024-byte framebuffer takes 10-15 ms of uninterrupted bus transactions.

If multiple tasks called I2C driver functions simultaneously, the hardware I2C state machine would collide and crash. Using the **actor pattern** (single-owner), all rendering requests come through `sensorQueue` and `displayModeQueue`. `DisplayTask` processes them sequentially — no I2C bus mutex needed.

**Key code:**
- `src/display.cpp:18-27` — I2C init and OLED setup (only file that includes `ssd1306.h`)
- `src/display.cpp:125-129` — all rendering calls

**Proof:** `git grep "ssd1306_"` shows only `display.cpp` calls the driver.

---

### Q11: What happens if a high-priority task never blocks?

It **starves** every lower-priority task. The scheduler always runs the highest-priority Ready task. If `StateTask` (Priority 2) never calls a blocking function, it consumes 100% of the CPU core. The Idle Task (`IDLE0`, Priority 0) never runs, and the Task Watchdog Timer triggers a panic reboot at 5.0 seconds.

**Key code:**
- `src/state_task.cpp:55` — `vTaskDelayUntil(&lastWakeTick, pdMS_TO_TICKS(50))` (the line that prevents starvation)

**Proof:** Fault Experiment 1 — commenting out that line caused:
```
E (5278) task_wdt: IDLE0 (CPU 0) starved
E (5278) task_wdt: CPU 0: StateTask
```

---

### Q12: What is the difference between Ready and Blocked?

- **Ready**: The task is fully prepared to run and sits on `pxReadyTasksLists`. It only waits because a higher-priority task is currently using the CPU. As soon as the CPU is free, it runs.
- **Blocked**: The task cannot run even if the CPU is empty. It is waiting for a specific event — a timer, a queue message, or a hardware interrupt. It consumes 0% CPU cycles.

**Example:** `InputTask` sits Blocked on `encoderQueue` with `portMAX_DELAY` (line 68). When a user turns the knob, the GPIO ISR calls `xQueueSendFromISR()` (line 26), which moves `InputTask` from Blocked → Ready → Running instantly via `portYIELD_FROM_ISR()`.

---

### Q13: What functionality did your unit tests verify?

13 tests across three pure logic modules, run on the host PC with no hardware:

**Alarm thresholds** (5 tests): 17.9°C → ALARM_LOW, 18.0°C → OK, 24.0°C → OK, 30.0°C → OK, 30.1°C → ALARM_HIGH

**Screen navigation** (4 tests): Forward cycling (Temp→Hum→Light→Motion), backward cycling, forward wraparound (Motion→Temp), backward wraparound (Temp→Motion)

**Inactivity timeout** (4 tests): 14.99s → stays ACTIVE, 15.00s → INACTIVE, >15s → stays INACTIVE, motion resets timer → stays ACTIVE

These files (`alarm_logic.cpp`, `display_mode.cpp`, `system_state.cpp`) have zero hardware dependencies, enabling host-based testing via `pio test -e native`.

**Proof:** Run `pio test -e native` → `13 Tests 0 Failures 0 Ignored`

---

### Q14: What did static analysis discover?

Running `pio check --skip-packages` found:

- **9 `unusedFunction` style warnings**: Expected — FreeRTOS tasks are registered as function pointers via `xTaskCreate()`, not called directly. Pure logic functions have external linkage for unit testing. `cppcheck` cannot trace dynamic function pointer registration.
- **1 deprecation warning**: `src/alarm.cpp:26` — `channelConfig.intr_type = LEDC_INTR_DISABLE` was flagged as obsolete. Fixed by removing the line since the ESP-IDF driver disables interrupts by default.

**Result: 0 memory leaks, 0 buffer overflows, 0 null pointer dereferences.**

**Proof:** Run `pio check --skip-packages` live.

---

### Q15: What would differ on physical hardware?

Three key differences the simulator doesn't model:

1. **Mechanical contact bounce**: Real rotary encoder contacts bounce for 1-5 ms, producing dozens of fake interrupt spikes. Needs an RC low-pass filter (0.1 uF + 10k) on GPIO 18/19 or a software debounce timer in the ISR. Wokwi produces ideal square waves.

2. **ADC non-linearity**: The ESP32 SAR ADC is inaccurate near 0V and 3.3V. Physical builds need eFuse factory calibration via `esp_adc_cal_characterize()` for accurate light readings on GPIO 34.

3. **DHT22 thermal lag**: In simulation, temperature changes instantly via a slider. A real sensor takes seconds for heat to conduct through the plastic housing. Our code already handles the 2-second power-up delay (`src/sensors.cpp:31`).

Also: GPIO 34 is input-only with no internal pull-down, so the physical breadboard requires an external 10k resistor to GND.
