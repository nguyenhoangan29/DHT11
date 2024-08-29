#include <ESP8266WiFi.h>   // Thư viện để kết nối với WiFi trên ESP8266
#include <PubSubClient.h>  // Thư viện để làm việc với MQTT
#include <DHT.h>           // Thư viện để làm việc với cảm biến DHT

#define DHTPIN D4  // Định nghĩa chân GPIO trên ESP8266 mà cảm biến DHT được kết nối
#define DHTTYPE DHT11  // Định nghĩa loại cảm biến DHT đang sử dụng

DHT dht(DHTPIN, DHTTYPE);  // Khởi tạo đối tượng DHT với chân GPIO và loại cảm biến

// Thông tin mạng Wi-Fi
const char* ssid = "Fablab 2.4G";  // Tên mạng WiFi để kết nối
const char* password = "Fira@2024";  // Mật khẩu WiFi

// Thông tin MQTT broker
const char* mqtt_server = "broker.emqx.io";  // Địa chỉ của MQTT broker
const int mqtt_port = 1883;  // Cổng của MQTT broker

WiFiClient espClient;  // Khách hàng WiFi để kết nối với MQTT
PubSubClient client(espClient);  // Khách hàng MQTT với kết nối WiFi

void setup() {
  Serial.begin(115200);  // Khởi tạo giao tiếp Serial với tốc độ 115200 baud
  dht.begin();  // Khởi động cảm biến DHT
  setupWiFi();  // Gọi hàm kết nối WiFi
  client.setServer(mqtt_server, mqtt_port);  // Thiết lập địa chỉ và cổng của MQTT server
}

void setupWiFi() {
  delay(10);  // Đợi 10ms trước khi bắt đầu kết nối WiFi
  Serial.println();  // In dòng trống để làm rõ các thông báo
  Serial.print("Đang kết nối tới ");  // In thông báo kết nối WiFi
  Serial.println(ssid);  // In tên WiFi

  WiFi.begin(ssid, password);  // Bắt đầu kết nối với WiFi

  while (WiFi.status() != WL_CONNECTED) {  // Kiểm tra trạng thái kết nối WiFi
    delay(500);  // Đợi 500ms trước khi kiểm tra lại
    Serial.print(".");  // In dấu chấm để báo hiệu đang kết nối
  }

  Serial.println();  // In dòng trống khi kết nối thành công
  Serial.println("Đã kết nối WiFi");  // Thông báo kết nối WiFi thành công
  Serial.print("Địa chỉ IP: ");  // In địa chỉ IP của ESP8266 trên mạng WiFi
  Serial.println(WiFi.localIP());  // In địa chỉ IP
}

void reconnect() {
  while (!client.connected()) {  // Kiểm tra nếu MQTT client chưa kết nối
    Serial.print("Đang kết nối MQTT...");  // Thông báo đang cố gắng kết nối với MQTT
    if (client.connect("But")) {  // Cố gắng kết nối với broker với ID client là "But"
      Serial.println("Đã kết nối");  // Thông báo kết nối MQTT thành công
      // Đăng ký nhận dữ liệu từ topic "DHT11"
      client.subscribe("DHT11");  // Đăng ký nhận dữ liệu từ topic "DHT11"
    } else {
      Serial.print("Kết nối thất bại");  // Thông báo kết nối thất bại
      Serial.print(client.state());  // In mã lỗi kết nối MQTT
      Serial.println(" Thử lại sau 5 giây");  // Thông báo thử lại kết nối sau 5 giây
      delay(5000);  // Đợi 5 giây trước khi thử kết nối lại
    }
  }
}

void loop() {
  if (!client.connected()) {  // Kiểm tra nếu MQTT client không kết nối
    reconnect();  // Cố gắng kết nối lại với MQTT
  }
  client.loop();  // Cập nhật trạng thái MQTT client

  static unsigned long lastMsg = 0;  // Biến lưu thời gian gửi tin nhắn trước đó
  unsigned long now = millis();  // Lấy thời gian hiện tại (số mili giây kể từ khi bắt đầu)

  if (now - lastMsg > 3000) {  // Nếu đã trôi qua 3 giây từ lần gửi tin nhắn trước
    lastMsg = now;  // Cập nhật thời gian gửi tin nhắn lần này
    float temperature = dht.readTemperature();  // Đọc giá trị nhiệt độ từ cảm biến DHT
    float humidity = dht.readHumidity();  // Đọc giá trị độ ẩm từ cảm biến DHT
    // Kiểm tra nếu không đọc được giá trị cảm biến
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Không đọc được giá trị từ cảm biến DHT!");  // Thông báo lỗi khi đọc cảm biến
    } else {
      Serial.print("Nhiệt độ: ");  // In giá trị nhiệt độ
      Serial.print(temperature);  // In giá trị nhiệt độ
      Serial.println(" °C");  // In đơn vị nhiệt độ
      Serial.print("Độ ẩm: ");  // In giá trị độ ẩm
      Serial.print(humidity);  // In giá trị độ ẩm
      Serial.println(" %");  // In đơn vị độ ẩm
      // Tạo chuỗi kết hợp nhiệt độ và độ ẩm
      String sensorData = "Temperature: " + String(temperature) + "C, Humidity: " + String(humidity) + "%";
      // Gửi dữ liệu lên một topic duy nhất
      client.publish("DHT11", sensorData.c_str());  // Gửi chuỗi dữ liệu lên topic "DHT11"
    }
  }
  delay(2000);  // Đợi 2 giây trước khi lặp lại
}
