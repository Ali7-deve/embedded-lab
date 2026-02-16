//different approaches using regiser level programming(not used in the project)
#include <Arduino.h>
#include <stdio.h>
#include <soc/gpio_reg.h>
#include <soc/gpio_struct.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


void setup() {
   // Initialization
   
    GPIO_ENABLE_W1TS_REG = (1 << 17) | (1 << 18);
   
}

void loop() {

    // Read sensor
    uint32_t gpio_state = GPIO_IN_REG;
        
        
    // Control LEDs based on sensor
    if (gpio_state & (1 << 15)) {
        GPIO_OUT_W1TS_REG = (1 << 17);
        GPIO_OUT_W1TC_REG = (1 << 18);
    } else {
        GPIO_OUT_W1TC_REG = (1 << 17);
        GPIO_OUT_W1TS_REG = (1 << 18);
    }

    delay(500);
    


}
