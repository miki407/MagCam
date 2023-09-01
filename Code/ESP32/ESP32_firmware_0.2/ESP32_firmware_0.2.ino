//  ESP32 Firmware for MagCam v0.2
//
//  8/25/2023 writen by Milos MIlievic
//
//  This firmware reads the date from an array of 256 hall effect sensors. This data is then transmited using Serial at 921600 BaudRate to a computer running the desktop application

#include "adc.h"        // include libreries

#define FCOUNT GPIO_NUM_33  // Pin number assigned to the binary counter for setting the read adress
#define AIN GPIO_NUM_32     // Pin number assigned to the output of the analog multiplexers 
#define FRESET GPIO_NUM_27  // Pin number assigned to the binary counter reset pin

#define DEVIDER 800   // These values determine the the frame rate of the module. This frame rate is calculated using the following formula:
#define COUNTER 4100  // Frame Rate = 80000000 / (DEVIDER * COUNTER)             24.0038406 fraes/ second

#define FRAMES_BEFORE_RESET 64 // The number of frames after witch the binary counter is reset

#define BUFFER_SIZE 512 // Value determening the number of elements in the array outBuffer

uint8_t *outBuffer = (uint8_t *)malloc(sizeof(uint8_t) * BUFFER_SIZE); // Creating an uint8_t array with BUFFER_SIZE elements

TaskHandle_t Task1; // Handle for setting up a task on a diffrent core for multithreading

hw_timer_t *My_timer = NULL; // Timer that determanes the framerate

bool state = false; // Global variables for keeping track of the state of the frame and current frame count
uint8_t frame = 0;

void IRAM_ATTR onTimer() { // Function ran everytime the timer triggers
  if (frame == 64) {       // Check if the current frame count equals 64 and reset the counter if that is the case. This is done as a preventative measure in case any bits are skipped
    gpio_set_level(FRESET, 0);
    delayMicroseconds(5);
    gpio_set_level(FRESET, 1);
    frame = 0;              // Reset the frame counter to 0
  }
  for (uint8_t x = 0; x < 16; x++) {  // For loop to go trough all the column
    adc1_get_raw(ADC1_CHANNEL_4); // Disriguard 2 readings on the start of each column. This is done to increase the accuracy of the SAR ADCs
    adc1_get_raw(ADC1_CHANNEL_4);
    for (uint8_t y = 0; y < 16; y++) { // For loop to go trought all the column
      gpio_set_level(FCOUNT, 1);   // Change state of the output pin FCOUNT to hight
      uint16_t buf = adc1_get_raw(ADC1_CHANNEL_4);   // Store the current ADC reading in memory
      gpio_set_level(FCOUNT, 0);  // Change state of output pin FCOUNT to low
      //                             ADC data      ADC Reading             empty    ADC MSB         empty   ADC LSB
      outBuffer[(x * 16 + y) * 2] = (uint8_t)((buf & 0x0FC0) >> 6); // Convert the read data from |_ _ _ _ | _ _ _ _ _ _ _ _ _ _ |   to  |_ _ | _ _ _ _ _ _ | and |_ _ |_ _ _ _ _ _|
      outBuffer[(x * 16 + y) * 2 + 1] = (uint8_t)(buf & 0x003F); //                                  4bits          12bits               2bits     6bits          2bits    6bits
      //delayMicroseconds(1); // delay to give digital pins time to change state
    }
  }
  frame++; // Incrament frame counter
  state = true; // Change state to indicate frame is done
}

void setup() {

  gpio_set_direction(FCOUNT, GPIO_MODE_OUTPUT);  // Set gpio pins as outputs or inputs
  gpio_set_direction(FRESET, GPIO_MODE_OUTPUT);
  pinMode(AIN, INPUT);

  adc_power_acquire();                           // Initilize the ADC
  adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_11db);
  adc1_config_width( ADC_WIDTH_12Bit);
  adc_set_clk_div(1);

  Serial.begin(921600);   // Serial initilization


  My_timer = timerBegin(0, DEVIDER, true);      // Setting up the timer
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, COUNTER, true);
  timerAlarmEnable(My_timer);

  // create a task that will be executed in the Task1code() function, with priority 0 and executed on core 0
  xTaskCreatePinnedToCore(
    Task1code,   /* Task function. */
    "Task1",     /* name of task. */
    100000,       /* Stack size of task */
    NULL,        /* parameter of the task */
    0,           /* priority of the task */
    &Task1,      /* Task handle to keep track of created task */
    0);          /* pin task to core 0 */


  gpio_set_level(FRESET, 0); // Reset the binary counter on startup
  delayMicroseconds(5);
  gpio_set_level(FRESET, 1);

}

void Task1code( void * pvParameters ) { //Code running on Core 0
  for (;;) {  //Loop
    if (state) {  //if the frame is finished reading
      state = false;  //change the state of the frame
      for (uint16_t n = 0; n < BUFFER_SIZE; n++) {  //For loop to go trough all the values
        Serial.write(outBuffer[n]);     //output the frame to serial
      }
    }
    else if (!state) {
      __asm("nop");
    }
  }
}
void loop() {
  //UNUSED
}
