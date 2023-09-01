#include "adc.h"
#include "driver/rtc_io.h"

#define FCOUNT 33
#define AIN 32

#define DEVIDER 700
#define COUNTER 3700

#define BUFFER_SIZE 256

TaskHandle_t Task1;

hw_timer_t *My_timer = NULL;

uint16_t *buffer = (uint16_t *)malloc(sizeof(uint16_t) * BUFFER_SIZE);
uint8_t *outBuffer = (uint8_t *)malloc(sizeof(uint8_t) * BUFFER_SIZE * 2);

int counter = 0;
int frame = 0;

void IRAM_ATTR onTimer() {
  if (frame > 31) {
    gpio_set_level(GPIO_NUM_27, 1);
    delayMicroseconds(15);
    gpio_set_level(GPIO_NUM_27, 0);
    delayMicroseconds(15);
    frame = 0;
  }
  frame++;
  adc1_get_raw(ADC1_CHANNEL_4);
  adc1_get_raw(ADC1_CHANNEL_4);
  for (int i = 0; i != 256; i++) {
    gpio_set_level(GPIO_NUM_33, 1);
    //delayMicroseconds(1);
    buffer[i] = adc1_get_raw(ADC1_CHANNEL_4);
    gpio_set_level(GPIO_NUM_33, 0);
    delayMicroseconds(5);
  }
  counter = 256;
}

void setup() {
  //pin mode
  gpio_set_direction(GPIO_NUM_33, GPIO_MODE_OUTPUT);
  gpio_set_pull_mode(GPIO_NUM_33, GPIO_PULLDOWN_ONLY);
  //gpio_set_drive_capability(GPIO_NUM_33, GPIO_DRIVE_CAP_MAX);
  gpio_set_direction(GPIO_NUM_27, GPIO_MODE_OUTPUT);
  gpio_set_pull_mode(GPIO_NUM_27, GPIO_PULLDOWN_ONLY);
  //gpio_set_drive_capability(GPIO_NUM_25, GPIO_DRIVE_CAP_3);
  //pinMode(FCOUNT, OUTPUT);
  pinMode(AIN, INPUT);

  gpio_set_level(GPIO_NUM_27, 1);
  delayMicroseconds(5);
  gpio_set_level(GPIO_NUM_27, 0);

  adc_power_acquire();
  adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_11db);
  adc1_config_width(ADC_WIDTH_12Bit);
  adc_set_clk_div(0);
  // timer setup
  Serial.begin(921600);
  My_timer = timerBegin(0, DEVIDER, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, COUNTER, true);
  timerAlarmEnable(My_timer);  //Just Enable

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    Task1code, /* Task function. */
    "Task1",   /* name of task. */
    100000,    /* Stack size of task */
    NULL,      /* parameter of the task */
    0,         /* priority of the task */
    &Task1,    /* Task handle to keep track of created task */
    0);        /* pin task to core 0 */
}

void Task1code(void *pvParameters) {
  for (;;) {
    if (counter == 256) {
      //for (uint16_t n = 0; n < 256; n++) {
      //outBuffer[n * 2] = (uint8_t)((buffer[n] & 0x0FC0) >> 6);
      //outBuffer[n * 2 + 1] = (uint8_t)(buffer[n] & 0x003F);
      //}
      counter = 0;
      uint16_t n;
      for (n = 0; n != 256; n++) {
        //Serial.write(outBuffer[n]);
        Serial.write((uint8_t)((buffer[n] & 0x0FC0) >> 6));
        __asm("nop");
        Serial.write((uint8_t)(buffer[n] & 0x003F));
      }
    } else if (counter > 256) {
      Serial.printf("\n Skipped frame %d\n", counter);
    }
  }
}
void loop() {
}