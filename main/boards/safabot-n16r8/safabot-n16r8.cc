#include "wifi_board.h"
#include "codecs/no_audio_codec.h"
#include "display/oled_display.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "led/single_led.h"
#include "assets/lang_config.h"

#include <esp_log.h>
#include <driver/i2c_master.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>


#define TAG "SafaBotN16R8"


// ============================================================
// SafaBot N16R8 Board
// ESP32-S3-N16R8
// ============================================================

class SafaBotN16R8 : public WifiBoard {
private:

    // --------------------------------------------------------
    // Buttons
    // --------------------------------------------------------

    Button boot_button_;

    // Not physically assigned yet.
    Button volume_up_button_;

    // Not physically assigned yet.
    Button volume_down_button_;


    // --------------------------------------------------------
    // OLED
    // --------------------------------------------------------

    Display* display_ = nullptr;

    i2c_master_bus_handle_t display_i2c_bus_ = nullptr;

    esp_lcd_panel_io_handle_t panel_io_ = nullptr;

    esp_lcd_panel_handle_t panel_ = nullptr;


    // ========================================================
    // OLED I2C
    // ========================================================

    void InitializeDisplayI2c() {

        i2c_master_bus_config_t bus_config = {};

        bus_config.i2c_port = I2C_NUM_0;
        bus_config.sda_io_num = DISPLAY_SDA_PIN;
        bus_config.scl_io_num = DISPLAY_SCL_PIN;
        bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
        bus_config.glitch_ignore_cnt = 7;
        bus_config.intr_priority = 0;
        bus_config.trans_queue_depth = 0;

        bus_config.flags.enable_internal_pullup = 1;

        ESP_ERROR_CHECK(
            i2c_new_master_bus(
                &bus_config,
                &display_i2c_bus_
            )
        );

        ESP_LOGI(
            TAG,
            "OLED I2C initialized: SDA=%d SCL=%d",
            DISPLAY_SDA_PIN,
            DISPLAY_SCL_PIN
        );
    }


    // ========================================================
    // SH1106 128x64
    // ========================================================

    void InitializeDisplay() {

        esp_lcd_panel_io_i2c_config_t io_config = {};

        io_config.dev_addr = 0x3C;
        io_config.scl_speed_hz = 400 * 1000;

        io_config.control_phase_bytes = 1;
        io_config.dc_bit_offset = 6;

        io_config.lcd_cmd_bits = 8;
        io_config.lcd_param_bits = 8;

        io_config.on_color_trans_done = nullptr;
        io_config.user_ctx = nullptr;

        io_config.flags.dc_low_on_data = 0;
        io_config.flags.disable_control_phase = 0;


        ESP_ERROR_CHECK(
            esp_lcd_new_panel_io_i2c(
                display_i2c_bus_,
                &io_config,
                &panel_io_
            )
        );


        // ----------------------------------------------------
        // OLED panel configuration
        // ----------------------------------------------------

        esp_lcd_panel_dev_config_t panel_config = {};

        panel_config.reset_gpio_num = GPIO_NUM_NC;
        panel_config.bits_per_pixel = 1;


        esp_lcd_panel_ssd1306_config_t oled_config = {
            .height = static_cast<uint8_t>(DISPLAY_HEIGHT)
        };

        panel_config.vendor_config = &oled_config;


        // ----------------------------------------------------
        // SH1106 driver
        // ----------------------------------------------------

#ifdef SH1106

        ESP_LOGI(TAG, "Installing SH1106 OLED driver");

        ESP_ERROR_CHECK(
            esp_lcd_new_panel_sh1106(
                panel_io_,
                &panel_config,
                &panel_
            )
        );

#else

        ESP_LOGI(TAG, "Installing SSD1306 OLED driver");

        ESP_ERROR_CHECK(
            esp_lcd_new_panel_ssd1306(
                panel_io_,
                &panel_config,
                &panel_
            )
        );

#endif


        // ----------------------------------------------------
        // Reset / initialize
        // ----------------------------------------------------

        ESP_ERROR_CHECK(
            esp_lcd_panel_reset(panel_)
        );

        esp_err_t ret = esp_lcd_panel_init(panel_);

        if (ret != ESP_OK) {

            ESP_LOGE(
                TAG,
                "OLED initialization failed: %s",
                esp_err_to_name(ret)
            );

            display_ = new NoDisplay();

            return;
        }


        // ----------------------------------------------------
        // Display orientation
        // ----------------------------------------------------

        ESP_ERROR_CHECK(
            esp_lcd_panel_invert_color(
                panel_,
                false
            )
        );


        ESP_ERROR_CHECK(
            esp_lcd_panel_disp_on_off(
                panel_,
                true
            )
        );


        // ----------------------------------------------------
        // XiaoZhi OLED display object
        // ----------------------------------------------------

        display_ = new OledDisplay(
            panel_io_,
            panel_,
            DISPLAY_WIDTH,
            DISPLAY_HEIGHT,
            DISPLAY_MIRROR_X,
            DISPLAY_MIRROR_Y
        );


        ESP_LOGI(
            TAG,
            "SH1106 OLED ready: %dx%d",
            DISPLAY_WIDTH,
            DISPLAY_HEIGHT
        );
    }


    // ========================================================
    // Motorcycle GPIO initialization
    // ========================================================

    void InitializeMotorcycleGPIO() {

        const gpio_num_t relay_gpios[] = {

            SAFABOT_KONTAKT_GPIO,
            SAFABOT_STARTER_GPIO,

            SAFABOT_RIGHT_GPIO,
            SAFABOT_LEFT_GPIO,

            SAFABOT_HIGH_BEAM_GPIO,
            SAFABOT_HORN_GPIO,

            SAFABOT_ENGINE_STOP_GPIO,

            SAFABOT_AUX2_GPIO,
            SAFABOT_AUX3_GPIO
        };


        for (gpio_num_t gpio : relay_gpios) {

            if (gpio == GPIO_NUM_NC) {
                continue;
            }

            gpio_reset_pin(gpio);

            gpio_set_direction(
                gpio,
                GPIO_MODE_OUTPUT
            );

            // ------------------------------------------------
            // IMPORTANT:
            // Active-low relay board.
            // HIGH = relay OFF.
            // ------------------------------------------------

            gpio_set_level(
                gpio,
                SAFABOT_RELAY_INACTIVE_LEVEL
            );
        }


        ESP_LOGI(
            TAG,
            "SafaBot motorcycle outputs initialized safely"
        );
    }


    // ========================================================
    // Buttons
    // ========================================================

    void InitializeButtons() {

        // ----------------------------------------------------
        // BOOT button
        // ----------------------------------------------------

        boot_button_.OnClick([this]() {

            auto& app = Application::GetInstance();

            if (
                app.GetDeviceState()
                == kDeviceStateStarting
            ) {

                EnterWifiConfigMode();

                return;
            }

            app.ToggleChatState();
        });


        // ----------------------------------------------------
        // Volume buttons
        //
        // Currently GPIO_NUM_NC.
        // No physical volume buttons are required.
        // ----------------------------------------------------

        volume_up_button_.OnClick([this]() {

            auto codec = GetAudioCodec();

            auto volume =
                codec->output_volume() + 10;

            if (volume > 100) {
                volume = 100;
            }

            codec->SetOutputVolume(volume);

            if (GetDisplay()) {

                GetDisplay()->ShowNotification(
                    Lang::Strings::VOLUME
                    + std::to_string(volume)
                );
            }
        });


        volume_up_button_.OnLongPress([this]() {

            GetAudioCodec()->SetOutputVolume(100);

            if (GetDisplay()) {

                GetDisplay()->ShowNotification(
                    Lang::Strings::MAX_VOLUME
                );
            }
        });


        volume_down_button_.OnClick([this]() {

            auto codec = GetAudioCodec();

            auto volume =
                codec->output_volume() - 10;

            if (volume < 0) {
                volume = 0;
            }

            codec->SetOutputVolume(volume);

            if (GetDisplay()) {

                GetDisplay()->ShowNotification(
                    Lang::Strings::VOLUME
                    + std::to_string(volume)
                );
            }
        });


        volume_down_button_.OnLongPress([this]() {

            GetAudioCodec()->SetOutputVolume(0);

            if (GetDisplay()) {

                GetDisplay()->ShowNotification(
                    Lang::Strings::MUTED
                );
            }
        });
    }


public:

    // ========================================================
    // Constructor
    // ========================================================

    SafaBotN16R8() :

        boot_button_(
            BOOT_BUTTON_GPIO
        ),

        volume_up_button_(
            VOLUME_UP_BUTTON_GPIO
        ),

        volume_down_button_(
            VOLUME_DOWN_BUTTON_GPIO
        )

    {

        // ----------------------------------------------------
        // Display
        // ----------------------------------------------------

        InitializeDisplayI2c();

        InitializeDisplay();


        // ----------------------------------------------------
        // Motorcycle GPIO
        // ----------------------------------------------------

        InitializeMotorcycleGPIO();


        // ----------------------------------------------------
        // Buttons
        // ----------------------------------------------------

        InitializeButtons();


        ESP_LOGI(
            TAG,
            "SafaBot N16R8 initialized"
        );
    }


    // ========================================================
    // LED
    // ========================================================

    virtual Led* GetLed() override {

        static SingleLed led(
            BUILTIN_LED_GPIO
        );

        return &led;
    }


    // ========================================================
    // AUDIO
    // INMP441 + MAX98357A
    // ========================================================

    virtual AudioCodec* GetAudioCodec() override {

#ifdef AUDIO_I2S_METHOD_SIMPLEX

        static NoAudioCodecSimplex audio_codec(

            AUDIO_INPUT_SAMPLE_RATE,

            AUDIO_OUTPUT_SAMPLE_RATE,


            // ------------------------------------------------
            // MAX98357A
            // ------------------------------------------------

            AUDIO_I2S_SPK_GPIO_BCLK,
            AUDIO_I2S_SPK_GPIO_LRCK,
            AUDIO_I2S_SPK_GPIO_DOUT,

            I2S_STD_SLOT_RIGHT,


            // ------------------------------------------------
            // INMP441
            // ------------------------------------------------

            AUDIO_I2S_MIC_GPIO_SCK,
            AUDIO_I2S_MIC_GPIO_WS,
            AUDIO_I2S_MIC_GPIO_DIN,

            I2S_STD_SLOT_LEFT
        );

#else

        static NoAudioCodecDuplex audio_codec(

            AUDIO_INPUT_SAMPLE_RATE,

            AUDIO_OUTPUT_SAMPLE_RATE,

            AUDIO_I2S_GPIO_BCLK,
            AUDIO_I2S_GPIO_WS,
            AUDIO_I2S_GPIO_DOUT,
            AUDIO_I2S_GPIO_DIN
        );

#endif

        return &audio_codec;
    }


    // ========================================================
    // DISPLAY
    // ========================================================

    virtual Display* GetDisplay() override {

        return display_;
    }
};


// ============================================================
// Register SafaBot board
// ============================================================

DECLARE_BOARD(SafaBotN16R8);