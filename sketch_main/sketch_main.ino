#define SGN_PIN 1
#define LED_NUMS 50
#define SERIAL_BAUD_RATE 115200

#define CONTROL_LED true
#define LED_PIN LED_BUILTIN
#define CHANNELS 3

#define SERIAL_CMD_RESET ((uint16_t)12592)
#define SERIAL_CMD_SET_COLORS ((uint16_t)12593)

#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a > b) ? b : a)

enum SerialMessageState {
  SERIAL_MSG_CMD = 0,
  SERIAL_MSG_LENGTH,
  SERIAL_MSG_PAYLOAD
};

uint8_t targetColors[LED_NUMS * CHANNELS + 100],
        currentColors[LED_NUMS * CHANNELS + 100];

void setup() {
#ifdef CONTROL_LED
  pinMode(LED_PIN , OUTPUT);
#endif
  Serial.begin(SERIAL_BAUD_RATE);
}

void loop() {
  static uint16_t msgLen = 0;
  static uint16_t messagePartPtr = 0;
  static SerialMessageState messageState = SERIAL_MSG_CMD;

  if (Serial.available() > 1) {
    uint16_t message = Serial.read() | (Serial.read() << 8);

    switch (message) {
    case SERIAL_CMD_RESET:
      messageState = SERIAL_MSG_CMD;
      return;
    case SERIAL_CMD_SET_COLORS:
      messageState = SERIAL_MSG_LENGTH;
      return;
    default:;
    }

    switch(messageState) {
    case SERIAL_MSG_LENGTH:
      if (message == 0) {
        message = SERIAL_MSG_CMD;
        return;
      }

      msgLen = message;
      messagePartPtr = 0;
      messageState = SERIAL_MSG_PAYLOAD;

      break;
    case SERIAL_MSG_PAYLOAD:
      targetColors[messagePartPtr] = message & 0xff;
      messagePartPtr += 1;

      if (messagePartPtr >= msgLen) {
        messageState = SERIAL_MSG_CMD;
        msgLen = 0;
        messagePartPtr = 0;
      }
      break;
    default:; // Ignore this
    }
  }

#ifdef CONTROL_LED
  if (targetColors[0] > 100) {
     digitalWrite(LED_PIN, HIGH);
  } else {
     digitalWrite(LED_PIN, LOW);
  }
#endif
}
