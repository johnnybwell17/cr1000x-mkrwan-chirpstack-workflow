#include <MKRWAN.h>

LoRaModem modem;

// --- Replace with your credentials ---
String appEui = "0000000000000000";
String appKey = "REPLACE_WITH_REAL_APP_KEY";

String message;
bool collecting = false;

enum SensorIndex {
  IDX_VWC5 = 0,
  IDX_VWC10,
  IDX_VWC20,
  IDX_VWC50,
  IDX_VWC100,
  IDX_EC5,
  IDX_EC10,
  IDX_EC20,
  IDX_EC50,
  IDX_EC100,
  IDX_T5,
  IDX_T10,
  IDX_T20,
  IDX_T50,
  IDX_T100,
  IDX_PRECIP,
  SENSOR_COUNT
};

const char* SENSOR_NAMES[SENSOR_COUNT] = {
  "VWC5", "VWC10", "VWC20", "VWC50", "VWC100",
  "EC5", "EC10", "EC20", "EC50", "EC100",
  "T5", "T10", "T20", "T50", "T100",
  "PRECIP"
};

void packInt16(uint8_t *buffer, int index, int16_t value) {
  buffer[index]     = (value >> 8) & 0xFF;
  buffer[index + 1] = value & 0xFF;
}

int16_t scaleValue(float value) {
  return (int16_t)(value * 100.0);
}

int sensorNameToIndex(const String& name) {
  for (int i = 0; i < SENSOR_COUNT; i++) {
    if (name == SENSOR_NAMES[i]) {
      return i;
    }
  }
  return -1;
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  Serial.println(F("\nCR1000X -> MKR WAN 1310 -> LoRaWAN"));
  Serial.println(F("Initializing LoRa modem..."));

  if (!modem.begin(US915)) {
    Serial.println(F("ERROR: LoRa modem failed to start."));
    while (1);
  }

  Serial.print(F("DevEUI: "));
  Serial.println(modem.deviceEUI());

  modem.setADR(true);

  Serial.println(F("Joining LoRaWAN via OTAA..."));
  while (!modem.joinOTAA(appEui, appKey)) {
    Serial.println(F("Join failed. Retrying..."));
    delay(10000);
  }
  Serial.println(F("Joined."));

  Serial1.begin(9600);
}

void loop() {
  while (Serial1.available()) {
    char ch = (char)Serial1.read();

    if (ch == 'S') {
      collecting = true;
      message = "S";
      continue;
    }

    if (!collecting) continue;

    if (ch != '\r' && ch != '\n') {
      message += ch;
    }

    if (message.endsWith(",F")) {
      collecting = false;
      processMessage();
      message = "";
      return;
    }
  }
}

void processMessage() {
  String s = message;

  if (!s.startsWith("S,") || !s.endsWith(",F")) {
    Serial.print(F("Bad frame: "));
    Serial.println(s);
    return;
  }

  s.remove(0, 2);
  s.remove(s.length() - 2, 2);

  float values[SENSOR_COUNT];
  bool present[SENSOR_COUNT];

  for (int i = 0; i < SENSOR_COUNT; i++) {
    values[i] = 0.0;
    present[i] = false;
  }

  while (s.length() > 0) {
    int commaIndex = s.indexOf(',');
    String field;

    if (commaIndex == -1) {
      field = s;
      s = "";
    } else {
      field = s.substring(0, commaIndex);
      s = s.substring(commaIndex + 1);
    }

    field.trim();
    if (field.length() == 0) continue;

    int eqIndex = field.indexOf('=');
    if (eqIndex == -1) continue;

    String name = field.substring(0, eqIndex);
    String valueText = field.substring(eqIndex + 1);

    name.trim();
    valueText.trim();

    int idx = sensorNameToIndex(name);
    if (idx >= 0) {
      values[idx] = valueText.toFloat();
      present[idx] = true;
    }
  }

  uint16_t bitmap = 0;
  for (int i = 0; i < SENSOR_COUNT; i++) {
    if (present[i]) {
      bitmap |= (1 << i);
    }
  }

  if (bitmap == 0) {
    Serial.println(F("No valid named values found."));
    return;
  }

  uint8_t payload[34];
  int payloadIndex = 0;

  payload[payloadIndex++] = (bitmap >> 8) & 0xFF;
  payload[payloadIndex++] = bitmap & 0xFF;

  for (int i = 0; i < SENSOR_COUNT; i++) {
    if (present[i]) {
      int16_t packedValue = scaleValue(values[i]);
      packInt16(payload, payloadIndex, packedValue);
      payloadIndex += 2;
    }
  }

  modem.setPort(2);
  modem.dataRate(1);

  int maxRetries = 3;
  int retryCount = 0;
  bool success = false;

  while (retryCount < maxRetries && !success) {
    modem.beginPacket();
    modem.write(payload, payloadIndex);

    int err = modem.endPacket(true);

    if (err > 0) {
      success = true;
    } else {
      retryCount++;
      if (retryCount < maxRetries) {
        delay(10000);
      }
    }
  }
}
