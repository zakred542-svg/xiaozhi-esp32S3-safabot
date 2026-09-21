#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

// ============================================================
// SafaBot PRO Master Edition
// Board: ESP32-S3-N16R8
// Flash: 16 MB
// PSRAM: OPI
// ============================================================


// ============================================================
// AUDIO
// ============================================================

#define AUDIO_INPUT_SAMPLE_RATE   16000
#define AUDIO_OUTPUT_SAMPLE_RATE  24000

// Use separate I2S input/output
#define AUDIO_I2S_METHOD_SIMPLEX


#ifdef AUDIO_I2S_METHOD_SIMPLEX

// ------------------------------------------------------------
// INMP441 Microphone
// ------------------------------------------------------------

#define AUDIO_I2S_MIC_GPIO_WS     GPIO_NUM_40
#define AUDIO_I2S_MIC_GPIO_SCK    GPIO_NUM_42
#define AUDIO_I2S_MIC_GPIO_DIN    GPIO_NUM_41


// ------------------------------------------------------------
// MAX98357A Speaker Amplifier
// ------------------------------------------------------------

#define AUDIO_I2S_SPK_GPIO_DOUT   GPIO_NUM_21
#define AUDIO_I2S_SPK_GPIO_BCLK   GPIO_NUM_47
#define AUDIO_I2S_SPK_GPIO_LRCK   GPIO_NUM_48

#else

// ------------------------------------------------------------
// Duplex I2S fallback
// ------------------------------------------------------------

#define AUDIO_I2S_GPIO_WS         GPIO_NUM_4
#define AUDIO_I2S_GPIO_BCLK       GPIO_NUM_5
#define AUDIO_I2S_GPIO_DIN        GPIO_NUM_6
#define AUDIO_I2S_GPIO_DOUT       GPIO_NUM_7

#endif


// ============================================================
// BOOT / USER BUTTONS
// ============================================================

// ESP32-S3 BOOT button
#define BOOT_BUTTON_GPIO          GPIO_NUM_0

// No physical touch button currently assigned
#define TOUCH_BUTTON_GPIO         GPIO_NUM_NC

// Volume buttons are not required.
// Volume will be controlled by SafaBot software/web/voice.
#define VOLUME_UP_BUTTON_GPIO     GPIO_NUM_NC
#define VOLUME_DOWN_BUTTON_GPIO   GPIO_NUM_NC


// ============================================================
// STATUS LED
// ============================================================

// No dedicated onboard LED assigned yet.
#define BUILTIN_LED_GPIO          GPIO_NUM_NC


// ============================================================
// DISPLAY
// SH1106 OLED 128x64 I2C
// ============================================================

#define DISPLAY_SDA_PIN           GPIO_NUM_8
#define DISPLAY_SCL_PIN           GPIO_NUM_9

#define DISPLAY_WIDTH             128
#define DISPLAY_HEIGHT            64

// Initial orientation.
// These can be changed after the real display is tested.
#define DISPLAY_MIRROR_X          true
#define DISPLAY_MIRROR_Y          true


// Tell the display driver which OLED controller is used.
#define SH1106


// ============================================================
// SafaBot MOTORCYCLE CONTROL
// ============================================================
//
// All relay outputs are intended for active-low relay modules.
//
// GPIO LOW  = relay active
// GPIO HIGH = relay inactive
//
// IMPORTANT:
// The ESP32 GPIO must NOT drive motorcycle loads directly.
// Use suitable relay/driver circuitry.
// ============================================================

#define SAFABOT_KONTAKT_GPIO       GPIO_NUM_10
#define SAFABOT_STARTER_GPIO       GPIO_NUM_11

#define SAFABOT_RIGHT_GPIO         GPIO_NUM_12
#define SAFABOT_LEFT_GPIO          GPIO_NUM_13

#define SAFABOT_HIGH_BEAM_GPIO     GPIO_NUM_14
#define SAFABOT_HORN_GPIO          GPIO_NUM_15


// ------------------------------------------------------------
// Relay 6
// Engine Stop
//
// Intended motorcycle circuit:
// Black/White wire <-> Green wire
//
// The actual motorcycle wiring will NOT be connected until
// the circuit is electrically verified.
// ------------------------------------------------------------

#define SAFABOT_ENGINE_STOP_GPIO   GPIO_NUM_16


// ------------------------------------------------------------
// Auxiliary relay
// ------------------------------------------------------------

#define SAFABOT_AUX2_GPIO          GPIO_NUM_17


// Reserved output for future SafaBot function
#define SAFABOT_AUX3_GPIO          GPIO_NUM_18


// ============================================================
// RELAY LOGIC
// ============================================================

// Active-low relay board
#define SAFABOT_RELAY_ACTIVE_LEVEL    GPIO_LOW
#define SAFABOT_RELAY_INACTIVE_LEVEL  GPIO_HIGH


// ============================================================
// SAFETY TIMERS
// ============================================================

// Maximum starter activation time.
// Prevents the starter relay remaining active indefinitely.
#define SAFABOT_STARTER_MAX_MS      2000


// Horn maximum activation time.
#define SAFABOT_HORN_MAX_MS         800


// Indicator flashing interval.
#define SAFABOT_FLASH_INTERVAL_MS   500


// Engine-stop pulse/activation limit.
// The motor-control layer will apply additional safety checks.
#define SAFABOT_ENGINE_STOP_MAX_MS  1000


// ============================================================
// MOTOR CONTROL FEATURES
// ============================================================

#define SAFABOT_HAS_MOTOR_CONTROL  1

#define SAFABOT_HAS_KONTAKT        1
#define SAFABOT_HAS_STARTER        1
#define SAFABOT_HAS_INDICATORS     1
#define SAFABOT_HAS_HIGH_BEAM      1
#define SAFABOT_HAS_HORN           1
#define SAFABOT_HAS_ENGINE_STOP    1
#define SAFABOT_HAS_AUX2           1


// Emergency lights use both indicators.
// No additional GPIO is required.
//
// RIGHT  + LEFT = HAZARD


// ============================================================
// SAFABOT SECURITY
// ============================================================

// Sensitive motorcycle commands must pass through the
// SafaBot security/control layer.
//
// This flag does NOT directly activate anything.
// It tells the motor-control subsystem that safety checks
// are required.
#define SAFABOT_SECURE_MOTOR_COMMANDS  1


// Engine stop is treated as a protected command.
#define SAFABOT_PROTECTED_ENGINE_STOP  1


// Starter is always time-limited.
#define SAFABOT_PROTECTED_STARTER      1


// ============================================================
// RESERVED GPIO DOCUMENTATION
// ============================================================
//
// GPIO 0   -> BOOT
// GPIO 1   -> reserved
//
// GPIO 8   -> OLED SDA
// GPIO 9   -> OLED SCL
//
// GPIO 10  -> Kontakt
// GPIO 11  -> Starter
// GPIO 12  -> Right indicator
// GPIO 13  -> Left indicator
// GPIO 14  -> High beam
// GPIO 15  -> Horn
// GPIO 16  -> Engine Stop / Relay 6
// GPIO 17  -> AUX2
// GPIO 18  -> AUX3
//
// GPIO 21  -> MAX98357A DOUT
//
// GPIO 40  -> INMP441 WS
// GPIO 41  -> INMP441 SD/DIN
// GPIO 42  -> INMP441 SCK
//
// GPIO 47  -> MAX98357A BCLK
// GPIO 48  -> MAX98357A LRCK
//
// ============================================================


// ============================================================
// END OF SAFABOT BOARD CONFIGURATION
// ============================================================

#endif // _BOARD_CONFIG_H_