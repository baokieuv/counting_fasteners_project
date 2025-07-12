#include <driver/gpio.h>
#include <stdlib.h>
#include "esp_timer.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_camera.h"
// #include "esp_heap_caps.h"

#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "esp_lvgl_port.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/spi_master.h"

// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <sys/types.h>

// #include "driver/i2c.h"
// #include "image_data.h"
#include "img_converters.h"

#define LCD_H_RES       240
#define LCD_W_RES       320

#define ENT_BTN         GPIO_NUM_12
#define MOV_BTN         GPIO_NUM_4

#define LCD_HOST        HSPI_HOST
#define PIN_NUM_MOSI    GPIO_NUM_13
#define PIN_NUM_CLK     GPIO_NUM_14
#define PIN_NUM_CS      GPIO_NUM_15
#define PIN_NUM_DC      GPIO_NUM_2
#define PIN_NUM_RST     -1

#define WIFI_SSID       "TP-Link_E276"
#define WIFI_PASS       "56445466"
#define MAXIMUM_RETRY   5

#define NUM_OBJECT      5

#define CAM_PIN_PWDN     32
#define CAM_PIN_RESET    -1 //software reset will be performed
#define CAM_PIN_XCLK     0
#define CAM_PIN_SIOD     26
#define CAM_PIN_SIOC     27

#define CAM_PIN_D7       35
#define CAM_PIN_D6       34
#define CAM_PIN_D5       39
#define CAM_PIN_D4       36
#define CAM_PIN_D3       21
#define CAM_PIN_D2       19
#define CAM_PIN_D1       18
#define CAM_PIN_D0       5
#define CAM_PIN_VSYNC    25
#define CAM_PIN_HREF     23
#define CAM_PIN_PCLK     22

// typedef struct sockaddr     SOCKADDR;
// typedef struct sockaddr_in  SOCKADDR_IN;
// typedef struct in_addr      IN_ADDR;

static const char* TAG = "test_dis";
static esp_lcd_panel_handle_t panel_handle = NULL;
static esp_lcd_panel_io_handle_t io_handle = NULL;
static lv_obj_t* src1 = NULL, *src2 = NULL;
static lv_obj_t* seletor = NULL, *img = NULL;
static camera_fb_t *pic = NULL;
static lv_img_dsc_t frame_src = {
    .header.w = LCD_W_RES,
    .header.h = LCD_H_RES,
    .header.cf = LV_COLOR_FORMAT_RGB565,
    .data_size = LCD_W_RES * LCD_H_RES * 2,
    .data = NULL,
};

static int s_retry_num = 0;

TaskHandle_t cam_task = NULL;
static QueueHandle_t btn_queue;

esp_err_t init_cam(void){
    camera_config_t camera_cfg = {
        .pin_pwdn = CAM_PIN_PWDN,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sccb_sda = CAM_PIN_SIOD,
        .pin_sccb_scl = CAM_PIN_SIOC,

        .pin_d7 = CAM_PIN_D7,
        .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5,
        .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3,
        .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1,
        .pin_d0 = CAM_PIN_D0,
        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href = CAM_PIN_HREF,
        .pin_pclk = CAM_PIN_PCLK,

        .xclk_freq_hz = 20000000,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = PIXFORMAT_RGB565, //YUV422,GRAYSCALE,RGB565,JPEG
        .frame_size = FRAMESIZE_QVGA,     //QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

        .jpeg_quality = 15, //0-63, for OV series camera sensors, lower number means higher quality
        .fb_count = 1,      //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
        .fb_location = CAMERA_FB_IN_PSRAM,
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
    };

    esp_err_t err = esp_camera_init(&camera_cfg);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "Camera init failed");
    }else{
        ESP_LOGI(TAG, "Init camera done");
    }
    return err;
}

esp_err_t init_st7789(void){
    esp_err_t err;
    spi_bus_config_t bus_cfg = {
        .sclk_io_num = PIN_NUM_CLK,
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = -1,
        .quadhd_io_num = -1,
        .quadwp_io_num = -1,
        .max_transfer_sz = 20 * LCD_H_RES * sizeof(uint16_t),
    };
    err = spi_bus_initialize(LCD_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "Error %s", esp_err_to_name(err));
        return err;
    }

    esp_lcd_panel_io_spi_config_t io_cfg = {
        .cs_gpio_num = PIN_NUM_CS,
        .dc_gpio_num = PIN_NUM_DC,
        .spi_mode = 0,
        .pclk_hz = 20000000,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    err = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_cfg, &io_handle);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "Error %s", esp_err_to_name(err));
        return err;
    }
    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = PIN_NUM_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
        .data_endian = LCD_RGB_DATA_ENDIAN_BIG,
    };
    err = esp_lcd_new_panel_st7789(io_handle, &panel_cfg, &panel_handle);
    if(err != ESP_OK){
        ESP_LOGE(TAG, "Error %s", esp_err_to_name(err));
        return err;
    }
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    if(esp_lcd_panel_init(panel_handle) != ESP_OK){
        ESP_LOGE(TAG, "Error %s", esp_err_to_name(err));
        return err;
    };
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    const uint8_t cmd_invoff = 0x21;
    ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(io_handle, cmd_invoff, NULL, 0));
    return ESP_OK;
}

void init_lvgl(void){
    lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    lvgl_port_display_cfg_t dis_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .control_handle = panel_handle,
        .hres = LCD_W_RES,
        .vres = LCD_H_RES,
        .buffer_size = LCD_H_RES * 20 * sizeof(uint16_t),
        .color_format = LV_COLOR_FORMAT_RGB565,
        .rotation.swap_xy = 1,
        .rotation.mirror_x = 1,
    };

    lv_disp_t *disp = lvgl_port_add_disp(&dis_cfg);
    if(disp == NULL) {
        ESP_LOGE(TAG, "Error in creating lv_disp"); 
        return;
    }

    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));

    lvgl_port_lock(portMAX_DELAY);

    src1 = lv_obj_create(NULL);
    src2 = lv_obj_create(NULL);

    lv_scr_load(src1);

    lvgl_port_unlock();
}

void screen_create_ui(){
    lvgl_port_lock(portMAX_DELAY);

    seletor = lv_dropdown_create(src1);

    lv_dropdown_set_options(seletor, "Washer\n"
                                    "Bolt\n"
                                    "Nut\n"
                                    "Screw\n"
                                    "Ring");
    
    lv_obj_align(seletor, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t* lb = lv_label_create(src1);
    lv_label_set_text(lb, "DEMO COUNTING OBJETS");
    lv_obj_align_to(lb, seletor, LV_ALIGN_OUT_TOP_MID, 0, -30);

    lvgl_port_unlock();

    lvgl_port_lock(portMAX_DELAY);
    img = lv_img_create(src2);
    lv_img_set_src(img, &frame_src);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    lvgl_port_unlock();
}

void draw_line(uint16_t x, uint16_t y, uint16_t length, uint8_t route){
    uint16_t width = (route == 0) ? length : 2;
    uint16_t height = (route == 0) ? 2 : length;

    if(width * height > LCD_W_RES * 2) return;

    uint16_t *line_buf = malloc(width * height * sizeof(uint16_t));
    if (!line_buf) {
        ESP_LOGE(TAG, "Failed to allocate line_buf");
        return;
    }
    for (int i = 0; i < width * height; i++) {
        line_buf[i] = 0xf81f;
    }

    esp_lcd_panel_draw_bitmap(panel_handle, x, y, x + width, y + height, line_buf);
    free(line_buf);
}

void draw_rect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1){
    draw_line(x0, y0, x1 - x0, 0);      // top
    draw_line(x0, y0, y1 - y0, 1);      // left
    draw_line(x0, y1, x1 - x0, 0);      // bottom
    draw_line(x1, y0, y1 - y0, 1);      // right
}

void task_draw_box(void *param){
    char* data = (char*)param;
    char *line = strtok(data, "\n");
    while(line != NULL){
        uint16_t x_min, y_min, x_max, y_max;
        if(sscanf(line, "%hd %hd %hd %hd", &x_min, &y_min, &x_max, &y_max) == 4){
            draw_rect(x_min, y_min, x_max, y_max);
        }
        line = strtok(NULL, "\n");
    }
    free(param);
    vTaskDelete(NULL);
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            ESP_LOGI(TAG,"connect to the AP fail");   
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
    }
}

static void wifi_init_sta(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");
}

esp_err_t http_event_handler(esp_http_client_event_t *evt){
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_ON_DATA:
        char *data_copy = malloc(evt->data_len + 1);
        if (!data_copy) {
            ESP_LOGE(TAG, "Failed to allocate memory for data copy");
            break;
        }
        memcpy(data_copy, evt->data, evt->data_len);
        data_copy[evt->data_len] = 0;
        xTaskCreate(task_draw_box, "draw_box", 2048, (void*)data_copy, 5, NULL);
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    default:
        break;
    }
    return ESP_OK;
}

void http_send_task(void *param){
    const char *boundary = "----myboundary";
    char type[32] = { 0 };
    lv_dropdown_get_selected_str(seletor, type, sizeof(type));
    for(int i = 0; i < strlen(type); i++) type[i] = tolower(type[i]);
    printf("%s\n", type);

    uint8_t *jpg_buf = NULL;
    size_t jpg_len = 0;
    ESP_LOGI("HEAP", "Largest free block (8-bit): %d bytes",
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    
    if(pic == NULL){
        ESP_LOGE(TAG, "There's no picture");
        vTaskDelete(NULL);
        return;
    }
    if (!fmt2jpg(pic->buf, pic->len, LCD_W_RES, LCD_H_RES, PIXFORMAT_RGB565, 80, &jpg_buf, &jpg_len) || jpg_len == 0) {
        ESP_LOGE(TAG, "JPEG conversion failed or empty JPEG buffer");
        free(jpg_buf);
        esp_camera_fb_return(pic);
        pic = NULL;
        vTaskDelete(NULL);
        return;
    }

    char header[256];
    int header_len = snprintf(header, sizeof(header),
        "--%s\r\n"
        "Content-Disposition: form-data; name=\"type\"\r\n\r\n"
        "%s\r\n"
        "--%s\r\n"
        "Content-Disposition: form-data; name=\"image\"; filename=\"image.jpg\"\r\n"
        "Content-Type: image/jpeg\r\n\r\n",
        boundary, type, boundary);
    
    
    char footer[64];
    int footer_len = snprintf(footer, sizeof(footer), "\r\n--%s--\r\n", boundary);

    int total_len = header_len + jpg_len + footer_len;

    esp_http_client_config_t config = {
        .url = "http://kvbhust.site/api/esp32",
        .event_handler = http_event_handler,
        .method = HTTP_METHOD_POST,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    char content_type[64];
    snprintf(content_type, sizeof(content_type), "multipart/form-data; boundary=%s", boundary);
    esp_http_client_set_header(client, "Content-Type", content_type);

    if (esp_http_client_open(client, total_len) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection");
        free(jpg_buf);
        esp_http_client_cleanup(client);
        vTaskDelete(NULL);
        return;
    }

    esp_http_client_write(client, header, header_len);
    esp_http_client_write(client, (char *)jpg_buf, jpg_len);
    esp_http_client_write(client, footer, footer_len);

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP POST Status = %d", esp_http_client_get_status_code(client));
    } else {
        ESP_LOGE(TAG, "HTTP POST failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    free(jpg_buf);
    esp_camera_fb_return(pic);
    pic = NULL;
    vTaskDelete(NULL);
}

void camera_task(void *param){
    while(1){
        if(pic != NULL){
            esp_camera_fb_return(pic);
            pic = NULL;
        }
        pic = esp_camera_fb_get();

        if (!pic) {
            ESP_LOGW(TAG, "No frame from camera");
            vTaskDelay(50 / portTICK_PERIOD_MS);
            continue;
        }

        lvgl_port_lock(portMAX_DELAY);

        frame_src.data = pic->buf;
        // frame_src.data = (const uint8_t*)image_data;
        lv_img_set_src(img, &frame_src);
        lv_obj_invalidate(img);

        lvgl_port_unlock();
        // for(int y = 0; y < LCD_H_RES; y += 40){
        //     esp_lcd_panel_draw_bitmap(panel_handle, 0, y, LCD_W_RES, y + 40, image_data + y * LCD_W_RES);
        // }
        vTaskDelay(200/portTICK_PERIOD_MS);
    }
}

void control_task(void *param){
    char data = 0;
    uint8_t option = 0, mode = 0;
    while(1){
        while(xQueueReceive(btn_queue, &data, portMAX_DELAY) == pdTRUE){
            if(data == 'm'){
                lvgl_port_lock(portMAX_DELAY);
                option = (option + 1) % NUM_OBJECT;
                lv_dropdown_set_selected(seletor, option);
                lvgl_port_unlock();
            }else if(data == 'e'){
                if(mode == 0){  //STA
                    lvgl_port_lock(portMAX_DELAY);
                    lv_scr_load(src2);
                    lvgl_port_unlock();
                    if(cam_task != NULL) vTaskDelete(cam_task);
                    xTaskCreate(camera_task, "cam task", 4096, NULL, 5, &cam_task);
                }else if(mode == 1){    //TAK
                    vTaskDelete(cam_task);
                    cam_task = NULL;
                    xTaskCreate(http_send_task, "http task", 3072, NULL, 5, NULL);
                }else if(mode == 2){    //OKE
                    lvgl_port_lock(portMAX_DELAY);
                    lv_scr_load(src1);
                    lvgl_port_unlock();
                }
                mode = (mode + 1) % 3;
            }
        }
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}

void IRAM_ATTR btn_handler(void *param){
    char c = *(char*)param;
    static uint64_t last_time = 0;
    
    if(esp_timer_get_time() - last_time > 200000){
        last_time = esp_timer_get_time();
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(btn_queue, &c, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    vTaskDelay(100 / portTICK_PERIOD_MS);

    wifi_init_sta();
    init_cam();

    gpio_config_t io_cfg = {
        .pin_bit_mask = (1ULL << MOV_BTN) | (1ULL << ENT_BTN),
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = 1,
        .pull_up_en = 0,
        .intr_type = GPIO_INTR_POSEDGE,
    };
    gpio_config(&io_cfg);
    // gpio_install_isr_service(0);
    char m = 'm', e = 'e';
    gpio_isr_handler_add(MOV_BTN, btn_handler, &m);
    gpio_isr_handler_add(ENT_BTN, btn_handler, &e);
    btn_queue = xQueueCreate(1, sizeof(char));

    if(init_st7789() == ESP_OK){
        init_lvgl();
        screen_create_ui();
        ESP_LOGI(TAG, "Successed to initialize st7789");
    }else{
        ESP_LOGE(TAG, "Failed to initialize st7789");
    }
    xTaskCreate(control_task, "ctl task", 2048, NULL, 5, NULL);

    // if(cam_task != NULL) vTaskDelete(cam_task);
    // xTaskCreate(camera_task, "cam task", 4096, NULL, 5, &cam_task);

    ESP_LOGI("HEAP", "Largest free block (8-bit): %d bytes",
            heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    while(1){
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}
