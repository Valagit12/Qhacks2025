#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include <ArduinoHttpClient.h>
#include <HTTPClient.h>
#include <string.h>
#include <base64.h>
#include <WebServer.h>
#include <Audio.h>
// #include <WiFiClientSecure.h>
//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//
//            You must select partition scheme from the board menu that has at least 3MB APP space.
//            Face Recognition is DISABLED for ESP32 and ESP32-S2, because it takes up from 15
//            seconds to process single frame. Face Detection is ENABLED if PSRAM is enabled as well

// ===================
// Select camera model
// ===================
#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
#include "camera_pins.h"

#define TOUCH_PIN D0 //D12 
#define MOTOR_PIN D7

#define I2S_BCLK D2
#define I2S_LRCLK D1
#define I2S_DOUT D3
// ===========================
// Enter your WiFi credentials
// ===========================
const char *ssid = "ballast-II";
const char *password = "Don'tPanic!";
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIICCTCCAY6gAwIBAgINAgPlwGjvYxqccpBQUjAKBggqhkjOPQQDAzBHMQswCQYD\n" \
"VQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEUMBIG\n" \
"A1UEAxMLR1RTIFJvb3QgUjQwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAwMDAw\n" \
"WjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2Vz\n" \
"IExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjQwdjAQBgcqhkjOPQIBBgUrgQQAIgNi\n" \
"AATzdHOnaItgrkO4NcWBMHtLSZ37wWHO5t5GvWvVYRg1rkDdc/eJkTBa6zzuhXyi\n" \
"QHY7qca4R9gq55KRanPpsXI5nymfopjTX15YhmUPoYRlBtHci8nHc8iMai/lxKvR\n" \
"HYqjQjBAMA4GA1UdDwEB/wQEAwIBhjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQW\n" \
"BBSATNbrdP9JNqPV2Py1PsVq8JQdjDAKBggqhkjOPQQDAwNpADBmAjEA6ED/g94D\n" \
"9J+uHXqnLrmvT/aDHQ4thQEd0dlq7A/Cr8deVl5c1RxYIigL9zC2L7F8AjEA8GE8\n" \
"p/SgguMh1YQdc4acLa/KNJvxn7kjNuK8YAOdgLOaVsjh4rsUecrNIdSUtUlD\n"  
"-----END CERTIFICATE-----\n";

WiFiClient client;

Audio audio;
String serverURL = ""; //"https://agamoto.onrender.com/";
bool idle = true;
bool ready = true;
void startCameraServer();
void setupLedFlash(int pin);

void do_send();
void handle_handshake();
void MotorTask(void *);
void LEDTask(void *);
unsigned long lastTime = 0;
unsigned long timerDelay = 100;
WebServer server(80);
bool isProcessing = false;
bool setupDone = false;

void watchCat(void *){
  unsigned long long startTime = millis();
  while(1){
    if(millis() - startTime > 10000 && setupDone == false){
      Serial.println("Setup failed.");
      Serial.println("Watch cat reset");
      ESP.restart();
    }
    vTaskDelay(500);    
  }
}
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.setDebugOutput(true);

  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  xTaskCreatePinnedToCore(
    LEDTask, //name of function
    "Led Task", //name for my own sake
    2048, //stack size
    NULL, //if multiple tasks of the same function are started, we can keep track by passing the reference to a variable that uniquely keeps track of the task
    1, //priority of the task
    NULL, //task handler, no need here
    0//here, specify which core should run this 0 or 1
  );
  xTaskCreatePinnedToCore(
    MotorTask, //name of function
    "Motor Task", //name for my own sake
    2048, //stack size
    NULL, //if multiple tasks of the same function are started, we can keep track by passing the reference to a variable that uniquely keeps track of the task
    1, //priority of the task
    NULL, //task handler, no need here
    0//here, specify which core should run this 0 or 1
  );
  xTaskCreatePinnedToCore(
    watchCat, //name of function
    "watch cat", //name for my own sake
    2048, //stack size
    NULL, //if multiple tasks of the same function are started, we can keep track by passing the reference to a variable that uniquely keeps track of the task
    1, //priority of the task
    NULL, //task handler, no need here
    0//here, specify which core should run this 0 or 1
  );
  

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_VGA;
  config.pixel_format = PIXFORMAT_JPEG;  // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }


  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(100);
    digitalWrite(MOTOR_PIN, HIGH);
    delay(100);
    digitalWrite(MOTOR_PIN, LOW);
    delay(100);
    digitalWrite(MOTOR_PIN, HIGH);
    delay(100);
    digitalWrite(MOTOR_PIN, LOW);
    ESP.restart();
  }

  sensor_t *s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);        // flip it back
    s->set_brightness(s, 1);   // up the brightness just a bit
    s->set_saturation(s, -2);  // lower the saturation
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

// Setup LED FLash if LED pin is defined in camera_pins.h
// #if defined(LED_GPIO_NUM)
//   setupLedFlash(LED_GPIO_NUM);
// #endif

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  unsigned long long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if(millis() - startTime > 5000){
      ESP.restart();
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // client.setCACert(root_ca);

  // startCameraServer();
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

  server.on("/", handle_handshake);
  
  server.begin();

  audio.setPinout(I2S_BCLK, I2S_LRCLK, I2S_DOUT);
  audio.setVolume(21);                 // 0...21(max)
  audio.setConnectionTimeout(1200,0);  // needed for some stations esp. from around the globe
  audio.setFileLoop(false);

  setupDone = true;
}


void handle_handshake() {
  Serial.println("Handshake Initiated.");
  serverURL = "http://" + server.client().remoteIP().toString() + ":3000/" ;
  idle = false;
  ready = true;

  server.send(200);
}

void do_send() {
  Serial.println("Sending");

   if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

      Serial.println(serverURL + "describe");
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverURL + "describe");
      http.setTimeout(10000);
      
      // Specify content-type header
      http.addHeader("Content-Type", "image/jpg");
      http.addHeader("Authorization", "i-have-a-massive-cock");
      // Data to send with HTTP POST
     camera_fb_t * fb = NULL;
      // Take Picture with Camera
      fb = esp_camera_fb_get();
       
      // Send HTTP POST request
      Serial.println("Sending HTTP request.");

      int httpResponseCode = http.POST(fb->buf, fb->len);

      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      
      if (httpResponseCode == 200) {
        isProcessing = false;
        audio.connecttohost( (serverURL + "audio.mp3").c_str() );
      }

      esp_camera_fb_return(fb);
        
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
      ESP.restart();
    }
    lastTime = millis();
  }
}

bool capture = false;
uint32_t threshold = 25000;
unsigned long long lastTrigger = 0;
unsigned long cooldownTime = 2000; //2s cooldown time
unsigned long timeout = 8000; //8s timeout

unsigned long lastMotorToggled = 0;
unsigned long motorCooldown = 500; //1s cooldown time
unsigned long motorDuration = 200; //100ms duration
int n = 0;
void loop() {
  server.handleClient();
  audio.loop();
  if (!idle && !audio.isRunning()) {
  //   // do_send(); 
  //   //PUT EVERYTHING IN THIS LOOP or jsut do !handshakes return 

  
    uint32_t touchVal = touchRead(TOUCH_PIN);
    if(touchVal > threshold && !isProcessing && (millis() - lastTrigger) > cooldownTime){
      lastTrigger = millis();
      Serial.println("Triggered");
      digitalWrite(MOTOR_PIN, HIGH);
      lastMotorToggled = millis();
      isProcessing = true;
      // delay(1000); //get rid of this.
      do_send();
      isProcessing = false;
  
      Serial.println("do_send done");
    }
    
  }
  yield();
}

void LEDTask(void *){
  while(true){
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay(1000);
  }
  vTaskDelete(NULL);
}

void MotorTask(void *){
  while(true){
    if(isProcessing){
      digitalWrite(MOTOR_PIN, HIGH);
      vTaskDelay(200);
      digitalWrite(MOTOR_PIN, LOW);
      vTaskDelay(400);
    }
    else{
      digitalWrite(MOTOR_PIN, LOW);
    }
    vTaskDelay(10);
  }
  vTaskDelete(NULL);
}