#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#define PIN 2
#define MAX 2000

int pulseLength(bool level) {
    int count = 0;
    while (gpio_get_level(PIN)==level) {
        if (count++ >= MAX) {
            return 0;
        };
    };
    return count;
};

void app_main() {
    while (true) {
        gpio_set_direction(PIN, GPIO_MODE_OUTPUT);
        gpio_set_level(PIN, 1);
        vTaskDelay(250 / portTICK_PERIOD_MS);

        gpio_set_level(PIN, 0);
        vTaskDelay(20 / portTICK_PERIOD_MS);
        
        // end start sequence ready to read data
        gpio_set_level(PIN, 1);
        gpio_set_direction(PIN, GPIO_MODE_INPUT);

        // get past start of message - left high, drops low, 
        // spikes high, drops low, and only then the message begins
        int j = pulseLength(1);
        j = pulseLength(0);
        j = pulseLength(1);
        j = pulseLength(0);
        j = 0;
        // Array to store times signal is high and low for - later we
        // use the fact short is shorter than low time, and long is 
        // longer to work out which bits are ones and which are zeroes
        int c[80];
        for (j=0;j<80;j+=2) {
            c[j]=pulseLength(1);
            c[j+1]=pulseLength(0);
        }
        int res[40]; 
        for (int i=0;i<40;i++) {
            int highLength = c[2*i];
            int lowLength = c[2*i+1];
            if (highLength != 0 && lowLength != 0) {
                res[i] = highLength>lowLength;
            }
        }
        int hex[] = {0, 0, 0, 0, 0};
        for (int i=0;i<40;i++) {
            hex[i/8] <<= 1;
            if (res[i]) {
                hex[i/8] |= 1;
            }
        }
        if (hex[4] != ((hex[0]+hex[1]+hex[2]+hex[3])&0xFF)) {
            printf("Checksum error - we didn't get good data\n\n");
        } else {
            printf("The temperature is %d ",hex[2]);
            printf("and the humidity is %d\n\n",hex[0]);
        }
        fflush(stdout);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
};