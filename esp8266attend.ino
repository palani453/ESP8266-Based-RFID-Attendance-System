#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

// RFID Pins for ESP8266
#define SS_PIN  15   // D8
#define RST_PIN 0    // D3

// WiFi credentials
const char* ssid = "";
const char* password = "";

// Google Apps Script Web App URL
String Web_App_URL = "";

// RFID objects
MFRC522 mfrc522(SS_PIN, RST_PIN);
int readsuccess;
char str[32] = "";
String UID_Result = "--------";

// 🔁 Change mode here before uploading: "reg" or "atc"
String modes = "atc";

// Info holders
String reg_Info = "";
String atc_Info = "";
String atc_Name = "";
String atc_Date = "";
String atc_Time_In = "";
String atc_Time_Out = "";

void http_Req(String str_modes, String str_uid) {
  if (WiFi.status() == WL_CONNECTED) {
    String http_req_url = "";

    if (str_modes == "reg") {
      http_req_url = Web_App_URL + "?sts=reg&uid=" + str_uid;
    } else if (str_modes == "atc") {
      http_req_url = Web_App_URL + "?sts=atc&uid=" + str_uid;
    }

    Serial.println();
    Serial.println("-------------");
    Serial.println("Sending request to Google Sheets...");
    Serial.print("URL : ");
    Serial.println(http_req_url);

    WiFiClientSecure client;
    client.setInsecure(); // For HTTPS (testing only)

    HTTPClient http;
    http.begin(client, http_req_url);
    int httpCode = http.GET();
    Serial.print("HTTP Status Code : ");
    Serial.println(httpCode);

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Payload : " + payload);

      String sts_Res = getValue(payload, ',', 0);

      if (str_modes == "reg") {
        reg_Info = getValue(payload, ',', 1);

        if (sts_Res == "OK") {
          if (reg_Info == "R_Successful") {
            Serial.println("✅ Registration successful.");
          } else if (reg_Info == "regErr01") {
            Serial.println("⚠️ UID already registered.");
          } else {
            Serial.println("⚠️ Unknown registration response.");
          }
        }
      }

      else if (str_modes == "atc") {
        atc_Info = getValue(payload, ',', 1);

        if (atc_Info == "TI_Successful") {
          atc_Name = getValue(payload, ',', 2);
          atc_Date = getValue(payload, ',', 3);
          atc_Time_In = getValue(payload, ',', 4);

          Serial.println("✅ Time In marked successfully.");
          Serial.println("Name: " + atc_Name);
          Serial.println("Date: " + atc_Date);
          Serial.println("Time In: " + atc_Time_In);
        }

        else if (atc_Info == "TO_Successful") {
          atc_Name = getValue(payload, ',', 2);
          atc_Date = getValue(payload, ',', 3);
          atc_Time_In = getValue(payload, ',', 4);
          atc_Time_Out = getValue(payload, ',', 5);

          Serial.println("✅ Time Out marked successfully.");
          Serial.println("Name: " + atc_Name);
          Serial.println("Date: " + atc_Date);
          Serial.println("Time In: " + atc_Time_In);
          Serial.println("Time Out: " + atc_Time_Out);
        }

        else if (atc_Info == "atcInf01") {
          Serial.println("ℹ️ Attendance already completed for today.");
        }

        else if (atc_Info == "atcErr01") {
          Serial.println("❌ UID not registered. Please register first.");
        }

        else {
          Serial.println("⚠️ Unknown attendance response.");
        }

        // Clear vars
        atc_Info = "";
        atc_Name = "";
        atc_Date = "";
        atc_Time_In = "";
        atc_Time_Out = "";
      }

    } else {
      Serial.println("❌ Failed to connect to script.");
    }

    http.end();
    Serial.println("-------------");

  } else {
    Serial.println("❌ WiFi not connected.");
  }
}

String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

int getUID() {
  if (!mfrc522.PICC_IsNewCardPresent()) return 0;
  if (!mfrc522.PICC_ReadCardSerial()) return 0;

  byteArray_to_string(mfrc522.uid.uidByte, mfrc522.uid.size, str);
  UID_Result = str;

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  return 1;
}

void byteArray_to_string(byte array[], unsigned int len, char buffer[]) {
  for (unsigned int i = 0; i < len; i++) {
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;
    buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
    buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
  }
  buffer[len*2] = '\0';
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  SPI.begin();
  mfrc522.PCD_Init();
  delay(500);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  int retries = 20;
  while (WiFi.status() != WL_CONNECTED && retries > 0) {
    delay(500);
    Serial.print(".");
    retries--;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected.");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ WiFi connection failed. Restarting...");
    delay(2000);
    ESP.restart();
  }

  Serial.print("System ready. Mode: ");
  Serial.println(modes == "reg" ? "Registration" : "Attendance");
  Serial.println("Waiting for RFID scan...");
}

void loop() {
  readsuccess = getUID();

  if (readsuccess) {
    Serial.println("\n---------------------------");
    Serial.println("Card detected!");
    Serial.print("UID: ");
    Serial.println(UID_Result);
    if (modes == "reg")
      Serial.println("Sending for Registration...");
    else
      Serial.println("Sending for Attendance...");
    
    http_Req(modes, UID_Result);
    Serial.println("---------------------------");
    delay(3000); // debounce delay
  }

  delay(50);
}
