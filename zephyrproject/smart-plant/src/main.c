#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include "measurements.h"

/* 1000 msec = 1 sec */
#define SENSOR_INTERVAL K_SECONDS(2)

#define STACKSIZE 1024
#define LED_THREAD_PRIORITY 7

// K_TIMER_DEFINE(led_timer, NULL, NULL);

/* The devicetree node identifier for the "led0" alias. */
// #define LED0_NODE DT_ALIAS(led0)

// static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

// void led_thread(void)
// {
//     int ret;
//     bool led_state = true;

//     while (1) {
//         k_timer_status_sync(&led_timer);

//         ret = gpio_pin_toggle_dt(&led);
//         if (ret < 0) {
//             return; 
//         }

//         led_state = !led_state;
//         printf("LED state: %s\n", led_state ? "ON" : "OFF");
//     }
// }

// K_THREAD_DEFINE(led_thread_id, STACKSIZE, led_thread, NULL, NULL, NULL, LED_THREAD_PRIORITY, 0, 0);

int main(void)
{
    int env_status;
    int soil_status;
    int ret;

    plant_measurement_t measurements = {0};

    printk("\n");
    printk("========================\n");
    printk(" Smart Plant - Milestone 2\n");
    printk("========================\n");

    env_status = environment_sensor_init();
    if(env_status < 0)
    {
        printk("BME sensor initialisation failed: %d\n", env_status);
    }else{
        printk("BME280 initialized successfully\n");
    }

    /* Initialise the soil moisture sensor. */
    soil_status = soil_sensor_init();

    if (soil_status < 0) {
        printk("Soil ADC initialization failed: %d\n",
               soil_status);
    } else {
        printk("Soil ADC initialized successfully\n");
    }

    /* Do not enter the acquisition loop if setup failed. */
    if (env_status < 0 || soil_status < 0) {
        printk("Sensor setup failed. Check hardware.\n");
        return 0;
    }

    while(1)
    {
        printk("\n--- Sensor Measurement ---\n");

        env_status = environment_sensor_read(&measurements.environment);

        if(env_status == 0 && measurements.environment.valid)
        {
            int32_t temp_centi = (uint32_t)(measurements.environment.temperature_c *100);
            int32_t humidity_centi = (int32_t)(measurements.environment.humidity_per * 100.0);
            int32_t pressure_centi = (int32_t)(measurements.environment.pressure_kpa * 100.0);

            printk("Temperature x100: %d C\n", temp_centi);
            printk("Humidity x100:    %d %%RH\n", humidity_centi);
            printk("Pressure x100:    %d kPa\n",pressure_centi);
        } else {
            printk("Environmental sensor error: %d\n",env_status);
        }

        /* Obtain soil moisture measurements. */
        soil_status = soil_sensor_read(&measurements.soil);

        if (soil_status == 0 &&measurements.soil.valid) {

            printk("Soil ADC raw:      %d\n", measurements.soil.raw_adc);
            printk("Soil ADC filtered: %d\n",measurements.soil.filtered_adc);
            printk("Soil moisture:     %d %%\n", measurements.soil.moisture_percentage);

        } else {
            printk("Soil sensor error: %d\n",soil_status);
        }

        printk("Uptime: %lld ms\n", (long long)k_uptime_get());

        k_sleep(SENSOR_INTERVAL);

        
    }
    return 0;
}