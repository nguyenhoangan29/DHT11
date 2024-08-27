#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN D4  // Pin kết nối DHT
#define DHTTYPE DHT11  // Loại cảm biến DHT

DHT dht(DHTPIN, DHTTYPE);

// Thông tin mạng Wi-Fi
const char* ssid = "Thu Vien DHBD";
const char* password = "HocHoiHieuHanh";

// Thông tin MQTT broker
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;  // Cổng MQTT

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  dht.begin();  // Khởi động cảm biến DHT
  setupWiFi();
  client.setServer(mqtt_server, mqtt_port);
}

void setupWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Đang kết nối tới ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Đã kết nối WiFi");
  Serial.print("Địa chỉ IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Đang kết nối MQTT...");
    if (client.connect("But")) {
      Serial.println("Đã kết nối");
      // Đăng ký nhận dữ liệu từ topic
      client.subscribe("DHT11");
    } else {
      Serial.print("Kết nối thất bại");
      Serial.print(client.state());
      Serial.println(" Thử lại sau 5 giây");
      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  static unsigned long lastMsg = 0;
  unsigned long now = millis();

  if (now - lastMsg > 3000) {
    lastMsg = now;
    float temperature = dht.readTemperature();  // Đọc nhiệt độ theo độ C
    float humidity = dht.readHumidity();  // Đọc độ ẩm
    // Kiểm tra nếu không đọc được giá trị cảm biến
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Không đọc được giá trị từ cảm biến DHT!");
    } else {
      Serial.print("Nhiệt độ: ");
      Serial.print(temperature);
      Serial.println(" °C");
      Serial.print("Độ ẩm: ");
      Serial.print(humidity);
      Serial.println(" %");
      // Tạo chuỗi kết hợp nhiệt độ và độ ẩm
      String sensorData = "Temperature: " + String(temperature) + "C, Humidity: " + String(humidity) + "%";
      // Gửi dữ liệu lên một topic duy nhất
      client.publish("DHT11", sensorData.c_str());
    }
  }
  delay(2000);
}