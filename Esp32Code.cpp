//Esp32 Code
#include "esp_camera.h"
#include <WiFi.h>
#include <bitset>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "esp_http_server.h"
#include "raw.h"

//Identifer and Password for either .softAP or .begin
const char* ssid = "Car";
const char* password = "bingo123";

//predetermined board defined. From ESP32 Examples
#define CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

//hardware Serial ports. 
#define RXD2 14
#define TXD2 15

//From ESP32 Cam Examples
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;


// Index handler. Responsible for sending raw html and creating the webpage. 
static esp_err_t index_handler(httpd_req_t *http_request){
  httpd_resp_set_type(http_request, "text/html");
  return httpd_resp_send(http_request, (const char *)INDEX_HTML, strlen(INDEX_HTML));// send over the html from raw.cpp
}

//video stream handler. From ESP32 Examples
static esp_err_t stream_handler(httpd_req_t *http_request){
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(http_request, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK){
    return res;
  }

  while(true){
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    } else {
      if(fb->width > 400){
        if(fb->format != PIXFORMAT_JPEG){
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if(!jpeg_converted){
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if(res == ESP_OK){
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(http_request, (const char *)part_buf, hlen);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(http_request, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(http_request, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if(fb){
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if(_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if(res != ESP_OK){
      break;
    }
    //Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));
  }
  return res;
}

//comand handler for the webserver. This takes in the http rquest and parses it for data
static esp_err_t cmd_handler(httpd_req_t *http_request){
  char*  command_buffer;    //command buffer(url)
  size_t command_buffer_length; //buffer lenght
  char variable[8] = {0,}; //html output buffer, size of 8.
  
  command_buffer_length = httpd_req_get_url_query_len(http_request); //returns 0 if query is not found in the request url
  if (command_buffer_length > 0) { // if its greater than 0, we got valid data.
    command_buffer = (char*)malloc(command_buffer_length); //create command buffer to store the url query in
    if (httpd_req_get_url_query_str(http_request, command_buffer, command_buffer_length+1) == ESP_OK) { // fill the buffer with the url query
      httpd_query_key_value(command_buffer, "go", variable, sizeof(variable)) == ESP_OK)  // ESP_OK : Key is found in the URL query string and copied to buffer
    }
    free(command_buffer); // free buffer. Buffer can be giant so we need to manually alocate memory on the heap.
  } 

  //Serial.println(variable); // print the array for testing purposes.

  
  if (variable[0] >= '0' && variable[0] <= 'Z')){ //if the first char which is the command falls with in this range, there is another char in the array that has the value.
   
    Serial2.write(variable[0]); // command char
    Serial2.write(variable[1]); // value char
    
  } else { // else this is a single byte command. Just send var[0] which is one char.
    Serial2.write(variable[0]); // comand char
  }


  httpd_resp_set_hdr(http_request, "Access-Control-Allow-Origin", "*"); // allows request to be shared
  return httpd_resp_send(http_request, NULL, 0);                        //send response back.                 
}

void startCameraServer(){
  //function to hook all handler functions to respective actions.
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80; // default webserver port
  httpd_uri_t index_uri = { // create uri, (webserver & socket)
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = index_handler, 
    .user_ctx  = NULL
  };
  httpd_uri_t cmd_uri = {
    .uri       = "/command",
    .method    = HTTP_GET,
    .handler   = cmd_handler,
    .user_ctx  = NULL
  };
  httpd_uri_t stream_uri = {
    .uri       = "/video",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };
  if (httpd_start(&camera_httpd, &config) == ESP_OK) { //Creates an instance of HTTP server, allocate memory/resources , has UDP, TCP
    httpd_register_uri_handler(camera_httpd, &index_uri); // The server has both a listening socket (TCP) for HTTP traffic, and a control socket (UDP) for control signals.
    httpd_register_uri_handler(camera_httpd, &cmd_uri);   // Same as before, however for the command side.
  }
  config.server_port += 1;
  config.ctrl_port += 1; // increment by one to start stream port, send over images. The HTML RAW is expecting the images to be sent over the 81 port.  
  if (httpd_start(&stream_httpd, &config) == ESP_OK) { //attempts to hook stream
    httpd_register_uri_handler(stream_httpd, &stream_uri); //connect stream up
  }
}

void setup() {  
  Serial.begin(115200); // usb serial used for debugging

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); // serial for comm with arduino
  
  camera_config_t config; // config from example
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000; // 20Meg
  config.pixel_format = PIXFORMAT_JPEG; 
  
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  // Camera init config 
  esp_err_t err = esp_camera_init(&config);

  // Wi-Fi connection
  WiFi.softAP(ssid, password); //This creates a access point devices can connect to. (Create Wifi)
  //WiFi.begin(ssid, password); //This connects to a access point, where it uses the access point to route traffic, not iself. (Connect to wifi)
  
  IPAddress IP = WiFi.softAPIP(); //ip for soft ap.
  //IPAddress IP = WiFi.localIP(); //ip for port fowarding
  Serial.print("AP IP address: ");
  Serial.println(IP);
  Serial.println("");
  
  // Start streaming web server
  startCameraServer();
}

void loop() {
  
}
