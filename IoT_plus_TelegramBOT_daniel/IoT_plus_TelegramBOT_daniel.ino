#define l1 A5
#define l2 3
#define s1 2
#define f1 A4

byte loads[5] = { l1, l2, s1, f1};
bool load_states[5] = { 1, 1, 0, 0};

String Buffer = "", data = "", mem = "";

unsigned long lastSendTime = 0;

struct IOT {
  void init() {
    Serial.begin(9600);
    for (int i = 0; i < 5; i++) {
      pinMode(loads[i], 1);
    }
    Buffer.reserve(64);
    data.reserve(64);
    mem.reserve(64);
  }

  bool isListData(String *data) {
    if (data->startsWith("[") && data->endsWith("]")) {
      return true;
    } else {
      return false;
    }
  }

  String readStrList(String *memory, String strList, byte position) {
    byte index = 0;
    *memory = "";
    for (int i = 0; i < strList.length(); i++) {
      if (strList[i] == ',') {
        index++;
      }
      if (index == position - 1) {
        memory->concat(strList[i]);
      }
    }
    if (memory->startsWith(",")) {
      *memory = memory->substring(memory->indexOf(',') + 1);
    }
    return *memory;
  }

  void checkSerial() {
    if (Serial.available()) {
      while (Serial.available() > 0) {
        delay(3);
        char c = Serial.read();
        data += c;
      }
    }
    if (data.length() > 0) {
      data.trim();
      if (isListData(&data)) {
        data = data.substring(data.indexOf('[') + 1, data.indexOf(']'));
        String command = readStrList(&mem, data, 1);
        if (command.startsWith("L")) {
          int val = command.substring(1).toInt();
          val = constrain(val, 1, 2);
          if (load_states[val - 1]) {
            load_states[val - 1] = 0;
          } else {
            load_states[val - 1] = 1;
          }
        } else if (command == "SCK") {
          if (load_states[2]) {
            load_states[2] = 0;
          } else {
            load_states[2] = 1;
          }
        } else if (command == "FAN") {
          if (load_states[3]) {
            load_states[3] = 0;
          } else {
            load_states[3] = 1;
          }
        } else if (command == "AON") {
          for (int i = 0; i < 4; i++) {
            load_states[i] = 1;
          }
        } else if (command == "AOFF") {
          for (int i = 0; i < 4; i++) {
            load_states[i] = 0;
          }
        } else {
          ;
        }
      }
    }
    data = "";
  }

  void sendStates() {
    if (millis() - lastSendTime >= 5000) {
      load_buffer();
      Serial.println(Buffer);
      lastSendTime = millis();
    }
  }

  void writeStates() {
    for (int i = 0; i < 5; i++) {
      digitalWrite(loads[i], load_states[i]);
      delay(100);
    }
  }

  void load_buffer(void) {
    Buffer = "";
    Buffer.concat("{ l1: ");
    Buffer.concat(load_states[0]);
    Buffer.concat(", l2: ");
    Buffer.concat(load_states[1]);
    Buffer.concat(", sck: ");
    Buffer.concat(load_states[2]);
    Buffer.concat(", fan: ");
    Buffer.concat(load_states[3]);
    Buffer.concat(" }");
  }

  void run() {
    checkSerial();
    sendStates();
    writeStates();
  }
} iot;

void setup() {
  iot.init();
}

void loop() {
  iot.run();
}
