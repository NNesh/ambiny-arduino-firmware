#include <FastLED.h>

#define OFF_TIME_SECS 2
#define SGN_PIN 6
#define LED_NUMS 48
#define SERIAL_BAUD_RATE 115200

#define CONTROL_LED
#define COLOR_ORDER RGB
#define LED_PIN LED_BUILTIN

#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a > b) ? b : a)
#define LEPR(from, to, value) ((from < value) ? MIN(from + value, to) : MAX(from - value, to))

typedef enum {
  STATUS_OK = 0,
  STATUS_DISCONNECTED,
  STATUS_NO_CONNECTION,
  STATUS_ERROR
} Status;

//bool bIsColorsDifferent = false;
//uint8_t targetColors[LED_NUMS * 3];
bool isLedActive = false;
volatile uint32_t lastTime = 0;
CRGB currentColors[LED_NUMS];

Status check_connection() {
  if (isLedActive) {
    if (millis() - lastTime > (OFF_TIME_SECS * 1000)) {
#ifdef CONTROL_LED
      digitalWrite(LED_PIN, LOW);
#endif
      isLedActive = false;
      FastLED.clear();
      FastLED.show();

      return STATUS_DISCONNECTED;
    }

    return STATUS_OK;
  }

  return STATUS_NO_CONNECTION;
}

Status read_channels(CRGB* color) {
  for (uint8_t idx = 0; idx < 3; idx++) {
    while (!Serial.available()) {
      Status connectionStatus = check_connection();
      switch (connectionStatus) {
        case STATUS_DISCONNECTED:
        case STATUS_NO_CONNECTION:
          return connectionStatus;
        default:;
      }
    }

    (*color)[idx] = Serial.read();
  }

  return STATUS_OK;
}

void wait_sync_msg() {
  uint8_t currentValue = 115;
  while (true) {
    while (!Serial.available()) {
      switch (check_connection()) {
        case STATUS_DISCONNECTED:
        case STATUS_NO_CONNECTION:
          currentValue = 115;
        default:;
      }
    }

    if (!isLedActive) {
#ifdef CONTROL_LED
      digitalWrite(LED_PIN, HIGH);
#endif
      isLedActive = true;
    }

    lastTime = millis();
    uint8_t value = Serial.read();

    switch(value) {
      case 115:
        currentValue = 105;
        break;
      case 105:
        currentValue = 110;
        break;
      case 110:
        currentValue = 99;
        break;
      case 99:
        currentValue = 32;
        break;
      case 32:
        return;
      default:
        currentValue = 115;
    }
  }
}

void setup() {
#ifdef CONTROL_LED
  pinMode(LED_PIN , OUTPUT);
#endif
  FastLED.addLeds<WS2812, SGN_PIN, COLOR_ORDER>(currentColors, LED_NUMS);
  Serial.begin(SERIAL_BAUD_RATE);
}

void loop() {
  wait_sync_msg();

  for (uint16_t ledIdx = 0; ledIdx < LED_NUMS; ledIdx++) {
    while (!Serial.available()) {
      switch (check_connection()) {
        case STATUS_DISCONNECTED:
        case STATUS_NO_CONNECTION:
          return;
        default:;
      }
    }

    lastTime = millis();

    if (!isLedActive) {
#ifdef CONTROL_LED
      digitalWrite(LED_PIN, HIGH);
#endif
      isLedActive = true;
    }

    Status readStatus = read_channels(&currentColors[ledIdx]);
    if (readStatus != STATUS_OK) {
#ifdef CONTROL_LED
      digitalWrite(LED_PIN, LOW);
#endif
      return;
    }
  }

  FastLED.show();
}
