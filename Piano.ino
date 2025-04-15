#include "esp_timer.h"

// === 4-bit Sine Wave Table (0 to 7) ===
const uint8_t SineWave[16] = { 6, 9, 11, 13, 15, 14, 13, 11, 9, 7, 5, 3, 1, 0, 2, 4 };

volatile uint8_t Index = 0;
volatile bool waveformActive = false;

// === DAC output pins (simulate 3-bit DAC on GPIO 0, 1, 2) ===
const int DAC_PINS[4] = {0, 1, 2, 3};

// === Switch pin (active-high when pressed) ===
const int KEY_PINS[4] = {D5, D8, D9, D10}; // For notes C, D, E, G
const uint16_t timerPeriods[4] = {120, 106, 95, 80}; // Approximate values


// === For edge detection ===
bool lastSwitchState = LOW;

// === Timer handle ===
esp_timer_handle_t periodic_timer;

// === Initialize DAC Pins ===
void DAC_Init() {
  for (int i = 0; i < 4; i++) {
    pinMode(DAC_PINS[i], OUTPUT);
    digitalWrite(DAC_PINS[i], LOW);
  }
}

// === Output 3-bit Value to DAC ===
void DAC_Out(uint8_t value) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(DAC_PINS[i], (value >> i) & 0x01);
  }
}

// === Timer callback function ===
void onTimer(void* arg) {
  if (waveformActive) {
    Index = (Index + 1) & 0x0F;
    DAC_Out(SineWave[Index]);
  }
}

//Write frequency to timer
void timerAlarmWrite(){
  for (int i = 0; i < 4; i++) {
    if (digitalRead(KEY_PINS[i]) == LOW) {
      esp_timer_restart(periodic_timer, timerPeriods[i]);
    }
  }
}

// === Setup Function ===
void setup() {
  Serial.begin(115200);
  DAC_Init();

  for (int i = 0; i < 4; i++) {
    pinMode(KEY_PINS[i], INPUT);
  }
  
  // Create the esp_timer
  const esp_timer_create_args_t timer_args = {
    .callback = &onTimer,
    .arg = NULL,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "sine_timer"
  };

  esp_timer_create(&timer_args, &periodic_timer);
  esp_timer_start_periodic(periodic_timer, 100);
}

// === Main Loop ===
void loop() {
  for (int i = 0; i < 4; i++) {
    if (digitalRead(KEY_PINS[i]) == LOW) { //I was having trouble with my circuit, so reversed the logic
      waveformActive = true;
      timerAlarmWrite();//timer, timerPeriods[i], true);
      Serial.println(i);
      delay(100); //I added a delay to stabalize the pin reads
      return;
    }
  }
  waveformActive = false;
  DAC_Out(0); // Silence when no key is pressed
}