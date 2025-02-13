#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"

#define I2C_PORT       i2c1
#define I2C_SDA        14
#define I2C_SCL        15
#define DISP_ADDR      0x3C

// -------------------------
// DEFINES PARA O JOYSTICK
// -------------------------
#define JOYSTICK_X     27  // ADC0
#define JOYSTICK_Y     26  // ADC1

#define JOYSTICK_BTN   22
#define BTN_A          5

#define LED_R          13  // LED Vermelho (PWM) -> Eixo X
#define LED_G          11  // LED Verde (GPIO)
#define LED_B          12  // LED Azul (PWM) -> Eixo Y

#define DEADZONE       200

// Estrutura do display
ssd1306_t disp;

// Alterna borda simples/dupla
bool border_state = false;

// LED verde
bool led_green_state = false;

// Liga/desliga PWM dos LEDs R e B
bool led_pwm_state = true;

// Calibração do joystick (usada só nos LEDs)
uint16_t x_center, y_center;

// ------------------------------------------------------
// Inicializa pino em modo PWM e configura wrap
// ------------------------------------------------------
void pwm_init_gpio(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice, 4095);
    pwm_set_enabled(slice, true);
}

// ------------------------------------------------------
// Subtrai o centro e aplica zona morta
// (usado só para controlar intensidade dos LEDs)
// ------------------------------------------------------
int16_t adjust_value(int16_t raw, int16_t center) {
    int16_t diff = raw - center;
    if (abs(diff) < DEADZONE) {
        return 0;
    }
    return diff;
}

// ------------------------------------------------------
// Interrupção do botão do joystick (GPIO 22)
// - alterna LED verde e borda
// ------------------------------------------------------
void toggle_led_green(uint gpio, uint32_t events) {
    led_green_state = !led_green_state;
    gpio_put(LED_G, led_green_state);

    border_state = !border_state;  
    printf("Botão do joystick: border_state=%d\n", border_state);
}

// ------------------------------------------------------
// Interrupção do botão A (GPIO 5)
// - liga/desliga PWM nos LEDs
// ------------------------------------------------------
void toggle_led_pwm(uint gpio, uint32_t events) {
    led_pwm_state = !led_pwm_state;
    printf("Botão A: led_pwm_state=%d\n", led_pwm_state);
}

// ------------------------------------------------------
// ÚNICO CALLBACK para tratar ambos os botões
// ------------------------------------------------------
void my_gpio_callback(uint gpio, uint32_t events) {
    // Verifica qual pino gerou a interrupção
    if (gpio == JOYSTICK_BTN && (events & GPIO_IRQ_EDGE_FALL)) {
        toggle_led_green(gpio, events);
    }
    else if (gpio == BTN_A && (events & GPIO_IRQ_EDGE_FALL)) {
        toggle_led_pwm(gpio, events);
    }
}

// ------------------------------------------------------
// Calibra o joystick (média de N leituras)
// - somente para controlar LEDs
// ------------------------------------------------------
void calibrate_joystick() {
    const int samples = 100;
    uint32_t sum_x = 0, sum_y = 0;

    for (int i = 0; i < samples; i++) {
        adc_select_input(0); // canal 0 => pino 26 => X
        sum_x += adc_read();
        adc_select_input(1); // canal 1 => pino 27 => Y
        sum_y += adc_read();
        sleep_ms(5);
    }
    x_center = sum_x / samples;
    y_center = sum_y / samples;
    printf("Calibração: x_center=%d, y_center=%d\n", x_center, y_center);
}

int main() {
    stdio_init_all();

    // ---------- ADC -----------
    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);

    // ---------- Botão do joystick (GPIO 22) -----------
    gpio_init(JOYSTICK_BTN);
    gpio_set_dir(JOYSTICK_BTN, GPIO_IN);
    gpio_pull_up(JOYSTICK_BTN);

    // ---------- Botão A (GPIO 5) -----------
    gpio_init(BTN_A);
    gpio_set_dir(BTN_A, GPIO_IN);
    gpio_pull_up(BTN_A);

    // ---------- Define a MESMA função de callback p/ ambos os pinos -----------
    // Passamos a callback só para JOYSTICK_BTN:
    gpio_set_irq_enabled_with_callback(JOYSTICK_BTN, GPIO_IRQ_EDGE_FALL, true, &my_gpio_callback);
    // Habilitamos também para BTN_A, sem sobrescrever callback:
    gpio_set_irq_enabled(BTN_A, GPIO_IRQ_EDGE_FALL, true);

    // ---------- LEDs -----------
    // LED R -> PWM (Eixo X)
    pwm_init_gpio(LED_R);
    // LED B -> PWM (Eixo Y)
    pwm_init_gpio(LED_B);
    // LED G -> simples GPIO
    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_put(LED_G, false);

    // ---------- Display I2C -----------
    ssd1306_t disp;
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&disp, 128, 64, false, DISP_ADDR, I2C_PORT);
    ssd1306_config(&disp); // Configura o display
    ssd1306_send_data(&disp); // Envia os dados para o display

    // ---------- Calibra Joystick (para LEDs) -----------
    calibrate_joystick();

    while (true) {
        // 1) Ler canal 0 (X) e canal 1 (Y)
        adc_select_input(0); // canal 0 => pino 26 => X
        uint16_t raw_x = adc_read();
        adc_select_input(1); // canal 1 => pino 27 => Y
        uint16_t raw_y = adc_read();

        // 2) Controlar LEDs R e B usando calibração
        if (led_pwm_state) {
            int16_t adj_x = adjust_value(raw_x, x_center);
            int16_t adj_y = adjust_value(raw_y, y_center);

            // LED vermelho => X
            pwm_set_gpio_level(LED_R, abs(adj_y) * 2);
            // LED azul => Y
            pwm_set_gpio_level(LED_B, abs(adj_x) * 2);
        } else {
            pwm_set_gpio_level(LED_R, 0);
            pwm_set_gpio_level(LED_B, 0);
        }

        // 3) Mapeamento da bolinha no display
        int x_pos = ((4095 - raw_x) * 52)  / 4095; 
        int y_pos = (raw_y * 113) / 4095;

        // 4) Desenho no display
        ssd1306_fill(&disp, false);

        // Borda simples (border_state=false) ou dupla (border_state=true)
        if (!border_state) {
            ssd1306_rect(&disp, 0, 0, 127, 63, 1, false);
        } else {
            ssd1306_rect(&disp, 0, 0, 127, 63, 1, false);
            ssd1306_rect(&disp, 1, 1, 125, 61, 1, false);
        }

        // Bolinha 8x8
        ssd1306_rect(&disp, x_pos, y_pos, 8, 8, 1, true);

        ssd1306_send_data(&disp);

        sleep_ms(50);
    }

    return 0;
}
