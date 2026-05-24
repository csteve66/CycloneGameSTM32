#include "main.h"

typedef enum {
    STATE_IDLE = 0,
    STATE_MOVE_RIGHT = 1,
    STATE_MOVE_LEFT = 2,
    STATE_LOSE = 3,
    STATE_WIN = 4
} GameState;

#define NUM_LEDS 7
#define CENTER_LED 3
#define MOVE_DELAY_MS 150
#define BLINK_DELAY_MS 400
#define DEBOUNCE_MS 50

volatile uint8_t button_pressed = 0;

static void GPIO_Init(void);
static void set_led(uint8_t index, uint8_t state);
static void set_all_leds(uint8_t state);
static void set_led_position(uint8_t pos);
static uint8_t button_was_pressed(void);
static void run_idle_state(void);
static void run_move_right_state(uint8_t *pos);
static void run_move_left_state(uint8_t *pos);
static void run_lose_state(void);
static void run_win_state(void);


int main(void) {
    HAL_Init();
    GPIO_Init();

    GameState state = STATE_IDLE;
    uint8_t led_pos = 0;

    while (1) {
        switch (state) {
            case STATE_IDLE:
                led_pos = 0;
                set_led_position(led_pos);
                run_idle_state();
                state = STATE_MOVE_RIGHT;
                led_pos = 0;
                break;

            case STATE_MOVE_RIGHT:
                run_move_right_state(&led_pos);

                if (button_was_pressed()) {
                    if (led_pos == CENTER_LED)
                        state = STATE_WIN;
                    else
                        state = STATE_LOSE;
                } else if (led_pos >= NUM_LEDS - 1) {
                    state = STATE_MOVE_LEFT;
                }
                break;

            case STATE_MOVE_LEFT:
                run_move_left_state(&led_pos);

                if (button_was_pressed()) {
                    if (led_pos == CENTER_LED)
                        state = STATE_WIN;
                    else
                        state = STATE_LOSE;
                } else if (led_pos == 0) {
                    state = STATE_MOVE_RIGHT;
                }
                break;

            case STATE_LOSE:
                set_led_position(led_pos);
                run_lose_state();
                state = STATE_IDLE;
                led_pos = 0;
                break;

            case STATE_WIN:
                run_win_state();
                state = STATE_IDLE;
                led_pos = 0;
                break;

            default:
                state = STATE_IDLE;
                break;
        }
    }
}

static void run_idle_state(void) {
    button_pressed = 0;

    while (!button_was_pressed()) {
        set_led(0, 1);
        HAL_Delay(500);
        set_led(0, 0);
        HAL_Delay(500);
    }
}

static void run_move_right_state(uint8_t *pos) {
    set_led_position(*pos);
    HAL_Delay(MOVE_DELAY_MS);

    if (*pos < NUM_LEDS - 1) {
        (*pos)++;
    }
}

static void run_move_left_state(uint8_t *pos) {
    set_led_position(*pos);
    HAL_Delay(MOVE_DELAY_MS);

    if (*pos > 0) {
        (*pos)--;
    }
}

static void run_lose_state(void) {
    button_pressed = 0;
    while (!button_was_pressed()) {
    	HAL_Delay(10);
    }
}

static void run_win_state(void) {
    button_pressed = 0;

    while (!button_was_pressed()) {
        set_all_leds(1);
        HAL_Delay(BLINK_DELAY_MS);
        set_all_leds(0);
        HAL_Delay(BLINK_DELAY_MS);

        // Even LEDs Blinking
        for (uint8_t i = 0; i < NUM_LEDS; i++) {
            set_led(i, (i % 2 == 0) ? 1 : 0);
        }
        HAL_Delay(BLINK_DELAY_MS);

        // Odd LEDs Blinking
        for (uint8_t i = 0; i < NUM_LEDS; i++) {
            set_led(i, (i % 2 == 1) ? 1 : 0);
        }
        HAL_Delay(BLINK_DELAY_MS);

        if (button_was_pressed()) {
        	break;
        }
    }

    set_all_leds(0);
}

static uint8_t button_was_pressed(void) {
    if (button_pressed) {
        button_pressed = 0;
        return 1;
    }
    return 0;
}

static void set_led(uint8_t index, uint8_t on_state) {
    GPIO_PinState pin_state = on_state ? GPIO_PIN_SET : GPIO_PIN_RESET;

    switch (index) {
        case 0:
        	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, pin_state); //PA0 pin
        	break;
        case 1:
        	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, pin_state); //PA1 pin
        	break;
        case 2:
        	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, pin_state); //PA4 pin
        	break;
        case 3:
        	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, pin_state); //PA5 pin
        	break;
        case 4:
        	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, pin_state); //PB4 pin
        	break;
        case 5:
        	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, pin_state); //PB5 pin
        	break;
        case 6:
        	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, pin_state); //PB3 pin
        	break;
        default:
        	break;
    }
}

static void set_all_leds(uint8_t on_state) {
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        set_led(i, on_state);
    }
}

static void set_led_position(uint8_t pos) {
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        set_led(i, (i == pos) ? 1 : 0);
    }
}

void EXTI4_15_IRQHandler(void) {
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_13) != RESET) {
        HAL_Delay(DEBOUNCE_MS);

        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) {
            button_pressed = 1;
        }
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);
    }
}

static void GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    set_all_leds(0);

    GPIO_InitStruct.Pin   = GPIO_PIN_13;
    GPIO_InitStruct.Mode  = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI4_15_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}