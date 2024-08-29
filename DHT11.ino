#include <ESP8266WiFi.h>    // Thư viện để kết nối ESP8266 với mạng WiFi
#include <PubSubClient.h>   // Thư viện để sử dụng giao thức MQTT
#include <DHT.h>            // Thư viện để sử dụng cảm biến DHT

#define DHTPIN D4           // Định nghĩa chân kết nối của cảm biến DHT11
#define DHTTYPE DHT11       // Loại cảm biến DHT đang sử dụng

DHT dht(DHTPIN, DHTTYPE);   // Khởi tạo đối tượng DHT với chân và loại cảm biến

const char* ssid = "Fablab 2.4G";  // Tên mạng WiFi
const char* password = "Fira@2024";  // Mật khẩu WiFi

const char* mqtt_server = "broker.emqx.io";  // Địa chỉ máy chủ MQTT
const int mqtt_port = 1883;   // Cổng MQTT

WiFiClient espClient;         // Khởi tạo đối tượng WiFiClient
PubSubClient client(espClient);  // Khởi tạo đối tượng PubSubClient với client WiFi

void setup() {
  Serial.begin(115200);      // Khởi tạo Serial với tốc độ truyền 115200 bps
  dht.begin();              // Khởi tạo cảm biến DHT
  setupWiFi();              // Kết nối với mạng WiFi
  client.setServer(mqtt_server, mqtt_port);  // Cấu hình máy chủ MQTT và cổng
}

void setupWiFi() {
  delay(10);                 // Delay nhỏ để ổn định
  Serial.println();          // Xuất một dòng trống
  Serial.print("Đang kết nối tới ");  
  Serial.println(ssid);      // In tên mạng WiFi đang kết nối

  WiFi.begin(ssid, password);  // Kết nối đến mạng WiFi

  while (WiFi.status() != WL_CONNECTED) {  // Kiểm tra kết nối WiFi
    delay(500);             // Delay 500ms
    Serial.print(".");     // In dấu chấm để hiển thị tiến trình kết nối
  }

  Serial.println();        // In một dòng trống
  Serial.println("Đã kết nối WiFi");  
  Serial.print("Địa chỉ IP: ");  
  Serial.println(WiFi.localIP());  // In địa chỉ IP của ESP8266
}

void reconnect() {
  while (!client.connected()) {  // Kiểm tra nếu client chưa kết nối
    Serial.print("Đang kết nối MQTT...");  
    if (client.connect("But")) {  // Cố gắng kết nối với tên client là "But"
      Serial.println("Đã kết nối");  
      client.subscribe("DHT11");  // Đăng ký nhận dữ liệu từ topic "DHT11"
    } else {
      Serial.print("Kết nối thất bại");  
      Serial.print(client.state());  
      Serial.println(" Thử lại sau 5 giây");  
      delay(5000);  // Delay 5 giây trước khi thử lại
    }
  }
}

void loop() {
  if (!client.connected()) {  // Nếu client không kết nối
    reconnect();  // Thử kết nối lại
  }
  client.loop();  // Đảm bảo client xử lý các tin nhắn MQTT

  static unsigned long lastMsg = 0;  // Biến để lưu thời gian gửi tin nhắn cuối cùng
  unsigned long now = millis();       // Lấy thời gian hiện tại

  if (now - lastMsg > 3000) {  // Nếu đã 3 giây kể từ lần gửi tin nhắn cuối
    lastMsg = now;  // Cập nhật thời gian gửi tin nhắn
    float temperature = dht.readTemperature();  // Đọc nhiệt độ từ cảm biến
    float humidity = dht.readHumidity();  // Đọc độ ẩm từ cảm biến

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Không đọc được giá trị từ cảm biến DHT!");  // Nếu không đọc được dữ liệu
    } else {
      Serial.print("Nhiệt độ: ");  
      Serial.print(temperature);  
      Serial.println(" °C");  // In nhiệt độ

      Serial.print("Độ ẩm: ");  
      Serial.print(humidity);  
      Serial.println(" %");  // In độ ẩm

      // Gửi nhiệt độ lên topic "DHT11/Temperature"
      String temperatureData = String(temperature);
      client.publish("DHT11/Temperature", temperatureData.c_str());

      // Gửi độ ẩm lên topic "DHT11/Humidity"
      String humidityData = String(humidity);
      client.publish("DHT11/Humidity", humidityData.c_str());
    }
  }
  delay(2000);  // Delay 2 giây trước khi vòng lặp tiếp theo bắt đầu
}
