/*
 * GW1NS-4C FreeRTOS Intuitive Two-Task Example: Button-Controlled LED Patterns
 * - High-priority task: Button watcher with debounce, sends events via queue
 * - Low-priority task: LED controller receives events and changes blink patterns
 * - Demonstrates inter-task communication with FreeRTOS queue
 * - Uses only official Gowin SDK functions
 * - No stdio.h, no sprintf, no float — minimal flash usage
 */

#include "gw1ns4c.h"
#include <stdbool.h>
#include "APB/apb_regbank_test.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// Pins (your physical pins)
#define LED_PIN    GPIO_Pin_0   // Physical pin 10 – status LED
#define BUTTON_PIN GPIO_Pin_1   // Physical pin 14 – button (active LOW)

// Task priorities & stack sizes (minimal)
#define BUTTON_TASK_PRIO     2    // Higher priority – real-time button handling
#define BUTTON_STK_SIZE      30

#define LED_TASK_PRIO        1    // Lower priority – LED pattern control
#define LED_STK_SIZE         30

// Queue for button events (0 = pressed, 1 = released)
QueueHandle_t buttonQueue;

// Task handles
TaskHandle_t ButtonTask_Handler;
TaskHandle_t LedTask_Handler;

// Prototypes
static void button_task(void *pvParameters);
static void led_task(void *pvParameters);
static void print_str(const char *str);
static void stars_print(uint8_t n);
static void sw_edition_print(void);
static void GPIOInit(void);
static void UARTInit(uint32_t baudrate);
static void sys_tick_init(void);

// Main
int main(void)
{
    SystemInit();
    UARTInit(921600);
    GPIOInit();



    print_str("Button released - slow blink\r\n");
    test_APB_registers_comprehensive();
  /*  sw_edition_print();

    // Create queue for button events (size 5, int items)
    buttonQueue = xQueueCreate(5, sizeof(int));

    sys_tick_init();

    taskENTER_CRITICAL();
    xTaskCreate(button_task, "Button", BUTTON_STK_SIZE, NULL, BUTTON_TASK_PRIO, &ButtonTask_Handler);
    xTaskCreate(led_task, "LED", LED_STK_SIZE, NULL, LED_TASK_PRIO, &LedTask_Handler);
    taskEXIT_CRITICAL();

    vTaskStartScheduler();*/

    while(1); // Should never reach here
}

// High-priority button task: Debounces button, sends events to queue
static void button_task(void *pvParameters)
{
    int button_state = 1;  // Initial state: not pressed (1 = HIGH)
    int last_state = 1;
    int event;

    while (1)
    {
        // Read button (active LOW)
        uint32_t input_data = GPIO_ReadBits(GPIO0);
        button_state = (input_data & BUTTON_PIN) ? 1 : 0;  // 0 = pressed

        if (button_state != last_state)
        {
            vTaskDelay(pdMS_TO_TICKS(50));  // 50 ms debounce

            // Re-read after debounce
            input_data = GPIO_ReadBits(GPIO0);
            button_state = (input_data & BUTTON_PIN) ? 1 : 0;

            if (button_state != last_state)
            {
                event = button_state;  // 0 = pressed, 1 = released
                xQueueSend(buttonQueue, &event, portMAX_DELAY);  // Send to LED task
                last_state = button_state;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));  // Poll every 10 ms
    }
}

// Low-priority LED task: Receives events, changes blink pattern
static void led_task(void *pvParameters)
{
    int pattern = 0;    // 0 = slow blink (normal), 1 = fast blink (after press)
    int delay_ms = 500; // Initial: slow blink 500 ms

    while (1)
    {
        int event;

        // Check for button event (timeout 10 ms)
        if (xQueueReceive(buttonQueue, &event, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            if (event == 0) {  // Pressed
                pattern = 1;   // Fast blink
                delay_ms = 100;
                print_str("Button pressed - fast blink\r\n");
            } else {           // Released
                pattern = 0;   // Slow blink
                delay_ms = 500;
                print_str("Button released - slow blink\r\n");
            }
        }

        // Blink LED with current pattern
        GPIO_SetBit(GPIO0, LED_PIN);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
        GPIO_ResetBit(GPIO0, LED_PIN);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Minimal helpers – no heavy libraries
// ─────────────────────────────────────────────────────────────────────────────

// Print simple string
static void print_str(const char *str)
{
    UART_SendString(UART0, (char *)str);
}

// Print stars line
static void stars_print(uint8_t n)
{
    while (n--) print_str("*");
}

// Startup banner (short to save flash)
static void sw_edition_print(void)
{
    print_str("\r\n");
    stars_print(40); print_str("\r\n");
    print_str("GW1NS-4C FreeRTOS Button-LED Demo\r\n");
    print_str("V1.0.0  "__DATE__"\r\n");
    print_str("----------------------------------------\r\n\r\n");
}

// GPIO initialization – per-pin
static void GPIOInit(void)
{
    GPIO_InitTypeDef gpioInitStruct = {0};

    // LED: output
    gpioInitStruct.GPIO_Pin = LED_PIN;
    gpioInitStruct.GPIO_Mode = GPIO_Mode_OUT;
    gpioInitStruct.GPIO_Int = GPIO_Int_Disable;
    GPIO_Init(GPIO0, &gpioInitStruct);

    // Button: input
    gpioInitStruct.GPIO_Pin = BUTTON_PIN;
    gpioInitStruct.GPIO_Mode = GPIO_Mode_IN;
    gpioInitStruct.GPIO_Int = GPIO_Int_Disable;
    GPIO_Init(GPIO0, &gpioInitStruct);
}

// UART initialization
static void UARTInit(uint32_t baudrate)
{
    UART_InitTypeDef UART_InitStruct = {0};
    UART_InitStruct.UART_Mode.UARTMode_Tx = ENABLE;
    UART_InitStruct.UART_Mode.UARTMode_Rx = ENABLE;
    UART_InitStruct.UART_BaudRate = baudrate;
    UART_Init(UART0, &UART_InitStruct);
}

// Systick setup
static void sys_tick_init(void)
{
    uint32_t temp = (1000 / configTICK_RATE_HZ) * (SystemCoreClock / 1000) - 1;
    SysTick->LOAD = temp;
    SysTick->VAL  = temp;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler(void)
{
    if (taskSCHEDULER_NOT_STARTED != xTaskGetSchedulerState())
    {
        xPortSysTickHandler();
    }
}
