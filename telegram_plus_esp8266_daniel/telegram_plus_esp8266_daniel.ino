#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define botRequestDelay 1000


const char *ssid = "GPIoT";
const char *pswd = "GPIOT-2025";

unsigned long lastTimeBotRan = 0;

String data_buffer = "", ser_buf = "";


const char *BOT_TOKEN = "7818414685:AAFCQlYWVc-pmW2kHd4agk3z2UicgXTrAPY";

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

String devId = "819434606";
String ownerId = "7093505512";  // Daniel

std::vector<String> authorizedUsers = { ownerId, devId };  // List of authorized users

bool isAuthorized(String userId) {
  for (String id : authorizedUsers) {
    if (id == userId) return true;
  }
  return false;
}

void handleCommands(int numNewMessages) {
  // Serial.print("New message(s) available...");
  for (int i = 0; i < numNewMessages; i++) {
    String chatId = bot.messages[i].chat_id;
    String userId = String(bot.messages[i].from_id);
    String command = bot.messages[i].text;
    // Serial.print("User Id: ");
    // Serial.println(userId);
    if (!isAuthorized(userId)) {
      bot.sendMessage(chatId, "You are not authorized to use this bot.", "");
      continue;
    }

    if (command == "/start") {
      String msg = " Hello! Type or Tap a valid command in the menu to continue.\n";
      bot.sendMessage(chatId, msg, "");
    } else if (command.startsWith("/toggle")) {
      Serial.println(command.substring(8));
      bot.sendMessage(chatId, "Success!", "");
    } else if (command == "/all_on") {
      Serial.println("[AON]");
      bot.sendMessage(chatId, "Success!", "");
    } else if (command == "/all_off") {
      Serial.println("[AOFF]");
      bot.sendMessage(chatId, "Success!", "");
    } else if (command == "/status") {
      if (data_buffer == "") {
        bot.sendMessage(chatId, "{...}", "");
      } else {
        bot.sendMessage(chatId, data_buffer, "");
      }
    } else if (command.startsWith("/add_user ")) {
      if (userId == ownerId) {
        String newUserId = command.substring(10);
        // Serial.print("Adding ");
        // Serial.println(newUserId);
        authorizedUsers.push_back(newUserId);
        bot.sendMessage(chatId, "User added successfully.", "");
      } else {
        bot.sendMessage(chatId, "Only the owner can add users.", "");
      }
    } else if (command.startsWith("/remove_user ")) {
      if (userId == ownerId) {
        String removeUserId = command.substring(13);
        authorizedUsers.erase(std::remove(authorizedUsers.begin(), authorizedUsers.end(), removeUserId), authorizedUsers.end());
        bot.sendMessage(chatId, "User removed successfully.", "");
      } else {
        bot.sendMessage(chatId, "Only the owner can remove users.", "");
      }
    } else {
      bot.sendMessage(chatId, "Unknown command.", "");
    }
  }
}


void readStates() {
  if (Serial.available()) {
    while (Serial.available() > 0) {
      delay(3);
      char c = Serial.read();
      ser_buf += c;
    }
  }
  if (ser_buf.length() > 0) {
    ser_buf.trim();
    data_buffer = ser_buf;
    ser_buf = "";
  }
}

void handleNewMessages() {
  if (millis() - lastTimeBotRan >= botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      activity();
      // Serial.println("Bot.Incoming...");
      handleCommands(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}

void checkConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    // Serial.print("Connecting.");
    while (WiFi.status() != WL_CONNECTED) {
      // Serial.print(".");
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
    }
    // Serial.println("Connected.");
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void activity() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
}


void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pswd);
  data_buffer.reserve(64);
  ser_buf.reserve(32);
  // client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  client.setInsecure();  // Disable certificate verification for simplicity
}

void loop() {
  handleNewMessages();
  readStates();
  checkConnection();
}
