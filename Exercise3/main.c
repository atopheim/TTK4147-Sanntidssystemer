#include <asf.h>
#include <board.h>
#include <gpio.h>
#include <sysclk.h>
#include "busy_delay.h"

#define CONFIG_USART_IF (AVR32_USART2)

// defines for BRTT interface
#define TEST_A      AVR32_PIN_PA31
#define RESPONSE_A  AVR32_PIN_PA30
#define TEST_B      AVR32_PIN_PA29
#define RESPONSE_B  AVR32_PIN_PA28
#define TEST_C      AVR32_PIN_PA27
#define RESPONSE_C  AVR32_PIN_PB00

volatile int g_testA_intr_flag = 0;
volatile int g_testB_intr_flag = 0;
volatile int g_testC_intr_flag = 0;


__attribute__((__interrupt__)) static void interrupt_J3(void);

void init(){
    sysclk_init();
    board_init();
    busy_delay_init(BOARD_OSC0_HZ);
    
    cpu_irq_disable();
    INTC_init_interrupts();
    INTC_register_interrupt(&interrupt_J3, AVR32_GPIO_IRQ_3, AVR32_INTC_INT1);
    cpu_irq_enable();
    
    stdio_usb_init(&CONFIG_USART_IF);

    #if defined(__GNUC__) && defined(__AVR32__)
        setbuf(stdout, NULL);
        setbuf(stdin,  NULL);
    #endif
}

__attribute__((__interrupt__)) static void interrupt_J3(void){ 
	
	if (gpio_get_pin_interrupt_flag(TEST_A)) {
		g_testA_intr_flag = 1;
		gpio_clear_pin_interrupt_flag(TEST_A);
	}
	
	if (gpio_get_pin_interrupt_flag(TEST_B)) {
		g_testB_intr_flag = 1;
		gpio_clear_pin_interrupt_flag(TEST_B);
	}
	
	if (gpio_get_pin_interrupt_flag(TEST_C)) {
		g_testC_intr_flag = 1;
		gpio_clear_pin_interrupt_flag(TEST_C);
	}		
	
	/*
	if (gpio_get_pin_interrupt_flag(TEST_A)) {
		gpio_set_pin_low(RESPONSE_A);
		busy_delay_us(5); // Hold signal low for a while, so the BRTT can see it
		gpio_set_pin_high(RESPONSE_A);
		gpio_clear_pin_interrupt_flag(TEST_A);
	}
	
	if (gpio_get_pin_interrupt_flag(TEST_B)) {
		busy_delay_us(100);
		gpio_set_pin_low(RESPONSE_B);
		busy_delay_us(5); // Hold signal low for a while, so the BRTT can see it
		gpio_set_pin_high(RESPONSE_B);
		gpio_clear_pin_interrupt_flag(TEST_B);
	}
	
	if (gpio_get_pin_interrupt_flag(TEST_C)) {
		gpio_set_pin_low(RESPONSE_C);
		busy_delay_us(5); // Hold signal low for a while, so the BRTT can see it
		gpio_set_pin_high(RESPONSE_C);
		gpio_clear_pin_interrupt_flag(TEST_C);
	}
	*/
}

//#define TASK_A
//#define TASK_B

int main (void){
    init();
	
	// Configure pins for butterfly test
	gpio_configure_pin (TEST_A, GPIO_DIR_INPUT);
	gpio_configure_pin (RESPONSE_A, GPIO_DIR_OUTPUT | GPIO_INIT_HIGH);
	
	gpio_configure_pin (TEST_B, GPIO_DIR_INPUT);
	gpio_configure_pin (RESPONSE_B, GPIO_DIR_OUTPUT | GPIO_INIT_HIGH);
	
	gpio_configure_pin (TEST_C, GPIO_DIR_INPUT);
	gpio_configure_pin (RESPONSE_C, GPIO_DIR_OUTPUT | GPIO_INIT_HIGH);
	
	// Set up interrupts
	gpio_enable_pin_interrupt(TEST_A, GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(TEST_B, GPIO_FALLING_EDGE);
	gpio_enable_pin_interrupt(TEST_C, GPIO_FALLING_EDGE);
	
	gpio_set_pin_high(TEST_A);
	gpio_set_pin_high(TEST_B);
	gpio_set_pin_high(TEST_C);
	
	gpio_set_pin_high(RESPONSE_A);
	gpio_set_pin_high(RESPONSE_B);
	gpio_set_pin_high(RESPONSE_C);
    	
	int testA = 1;
	int testB = 1;
	int testC = 1;

    while(1){
		if (g_testA_intr_flag == 1) { // Received interrupt
			gpio_set_pin_low(RESPONSE_A);
			busy_delay_us(5); // Hold signal low for a while, so the BRTT can see it
			gpio_set_pin_high(RESPONSE_A);
			g_testA_intr_flag = 0;
		}
				
		if (g_testB_intr_flag == 1) { // Received interrupt
			//busy_delay_us(100);
			gpio_set_pin_low(RESPONSE_B);
			busy_delay_us(5); // Hold signal low for a while, so the BRTT can see it
			gpio_set_pin_high(RESPONSE_B);
			g_testB_intr_flag = 0;
		}
				
		if (g_testC_intr_flag == 1) { // Received interrupt
			gpio_set_pin_low(RESPONSE_C);
			busy_delay_us(5); // Hold signal low for a while, so the BRTT can see it
			gpio_set_pin_high(RESPONSE_C);
			g_testC_intr_flag = 0;
		}
		
		//busy_delay_us(10);
		
		/*
		testA = gpio_get_pin_value(TEST_A);
		if (testA == 0) { // Received test signal from BRTT
			gpio_set_pin_low(RESPONSE_A);
			busy_delay_us(5); // Hold signal low for a while, so the BRTT can see it
			gpio_set_pin_high(RESPONSE_A);
		}
		
		testB = gpio_get_pin_value(TEST_B);
		if (testB == 0) { // Received test signal from BRTT
			gpio_set_pin_low(RESPONSE_B);
			busy_delay_us(5); // Hold signal low for a while, so the BRTT can see it
			gpio_set_pin_high(RESPONSE_B);
		}
		
		testC = gpio_get_pin_value(TEST_C);
		if (testC == 0) { // Received test signal from BRTT
			gpio_set_pin_low(RESPONSE_C);
			busy_delay_us(5); // Hold signal low for a while, so the BRTT can see it
			gpio_set_pin_high(RESPONSE_C);
		}
		*/
    }
}
