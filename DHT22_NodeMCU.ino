#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Thông tin WiFi
const char* ssid = "BDU-Phonghoc";
const char* password = "HocHoiHieuHanh";

// Thông tin MQTT Broker
const char* mqtt_server = "broker.hivemq.com"; // Địa chỉ IP của Mosquitto broker
const int mqtt_port = 1883;

// Cấu hình cảm biến DHT11
#define DHTPIN 4         // Chân dữ liệu của cảm biến DHT11
#define DHTTYPE DHT11    // Loại cảm biến DHT

DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe("weather");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  dht.begin();  // Khởi động cảm biến DHT11
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Đọc dữ liệu từ cảm biến DHT11
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();  // Đọc nhiệt độ ở độ C

  // Kiểm tra xem dữ liệu có hợp lệ không
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Tạo chuỗi thông điệp với dữ liệu cảm biến
  String message = "Temperature: " + String(temperature) + "C, Humidity: " + String(humidity) + "%";
  
  // Gửi thông điệp đến broker
  client.publish("sensor/data", message.c_str());

  delay(2000);  // Gửi dữ liệu mỗi 2 giây
}
