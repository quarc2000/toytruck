#include <Arduino.h>
#include <setget.h>
#include <motor.h>


#define PWM

// Ultrasound number and pins
// Motor
#define M1E_PIN 2
#define M1F_PIN 4
#define M1R_PIN 5

// Steering (without servo)
#define SENABLE 33
#define SRIGHT 27
#define SLEFT 23

//  Laser Lidar ToF __________________________
/*#define VL53L0X_REG_IDENTIFICATION_MODEL_ID 0xc0
#define VL53L0X_REG_IDENTIFICATION_REVISION_ID 0xc2
#define VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD 0x50
#define VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD 0x70
#define VL53L0X_REG_SYSRANGE_START 0x00

#define VL53L0X_ADDRESS 0x29 */

Motor drive ;

void setup()
{
  // Initialize serial communication for debugging
  Serial.begin(57600);


  //globalVar_init();


  vTaskDelay(pdMS_TO_TICKS(100));
  //----------------------------------------------------------------------------------------------------------------
}

void loop()
{
   vTaskDelay(pdMS_TO_TICKS(1000));
   drive.driving(80);
   vTaskDelay(pdMS_TO_TICKS(1000));
   drive.driving(-90);
};
