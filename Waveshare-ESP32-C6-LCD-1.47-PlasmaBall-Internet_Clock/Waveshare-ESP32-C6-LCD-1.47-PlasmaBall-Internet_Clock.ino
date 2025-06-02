/*********************************************************************************************************
* Waveshare ESP32-C6-LCD-1.47 Internet Clock Project
*
* Description:
*  This code implements an internet clock with Wi-Fi connectivity on the Waveshare ESP32-C6 1.47" LCD
*  display. The project uses Squareline Studio to create the UI interface and includes time/date display,
*  WiFi status monitoring, and visual effects. The clock synchronizes with NTP servers and maintains time
*  using the ESP32's RTC capabilities.
*
* Key Features:
*  - Accurate timekeeping with NTP synchronization (every 2 hours)
*  - LVGL-based graphical interface with smooth animations
*  - Display of time, date, day of week, week number, and year
*  - WiFi quality indicator with colour-coded signal strength
*  - FPS counter
*  - Rotating plasma ball animation for visual appeal
*  - NeoPixel LED status indicator
*
* How It Works:
*  1. Initialization: Sets up display, LVGL, and WiFi connection
*  2. Time Synchronization: Gets current time from NTP server at startup and every 2 hours
*  3. Display Updates: Continuously updates clock face with smooth transitions
*  4. WiFi Monitoring: Regularly checks and displays connection quality
*  5. Animation: Maintains rotating corner graphic and LED effects
*
* Notes:
*  - Time zone offset is configurable (default GMT+2)
*  - Edit the Wi-Fi SSID & password variables to your network
*  - Backlight brightness is set to 75% by default
*  - Had to comment out line 5 of LVGL_Driver.h
*  - SLS export path will need to get updated in the project settings
*  - When exporting in SLS & replacing project files, you NEED to edit the ui.h file on line 30:
*      #include "screens/ui_MainScreen.h"
*      >TO<
*      #include "ui_MainScreen.h"
*      (Remove 'screens/' - not sure why SLS is exporting this way as flat export option is selected.)
* 
**********************************************************************************************************/

/*************************************************************
******************* INCLUDES & DEFINITIONS *******************
**************************************************************/

#include <ESP32Time.h> // https://github.com/fbiego/ESP32Time
#include <WiFi.h>

#include "Display_ST7789.h"
#include "LVGL_Driver.h"
#include "NeoPixel.h"
#include "ui.h"

// Time configuration
ESP32Time rtc(0);
const char* ntpServer = "pool.ntp.org";

/******************************
*** >> EDIT THIS SECTION << ***
*******************************/
int tzOffset = 2;                  // edit with your time zone offset (mine is GMT +2)
String ssid = "YOUR_SSID";         // replace with your Wi-Fi SSID name
String password = "YOUR_PASSWORD"; // replace with your Wi-Fi password

// Global Variables
String months[] = {
  "January", "February", "March", "April", "May", "June",
  "July", "August", "September", "October", "November", "December"
};

int pos[7] = {69, -69, -46, -23, 0, 23, 46}; // position array

String ipAddress = "";

int last_second = 0;
String dateString = "";

int angle = 0;
int n = 7200;

unsigned long lastWiFiUpdate = 0;
const int wifiUpdateInterval = 5000; // 5 seconds

unsigned long lastFPSTime = 0;
unsigned int frameCount = 0;
int currentFPS = 0;


/*************************************************************
********************** HELPER FUNCTIONS **********************
**************************************************************/

// Function to connect to Wi-Fi
void connectWifi() { 
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  ipAddress = WiFi.localIP().toString(); // store the IP when connected
}

// Function to set time
void setTime() {
  configTime(3600 * tzOffset, 0, ntpServer);
  struct tm timeinfo;

  if (getLocalTime(&timeinfo)) {
    rtc.setTimeStruct(timeinfo); 
  }
}

// Function to get calendar week
int getWeekNumber() {
  int dayOfYear = rtc.getDayofYear();
  int firstDayOfWeek = (rtc.getDayofWeek() - (dayOfYear % 7) + 7) % 7;
  int weekNumber = (dayOfYear + firstDayOfWeek - 1) / 7 + 1;
  return weekNumber;
}

// Function to update WiFi quality indicator
void updateWiFiQuality() {
  // Use LVGL's selector syntax
  lv_style_selector_t selector = LV_STATE_DEFAULT;
  
  int rssi = WiFi.RSSI();

  if (WiFi.status() != WL_CONNECTED) {
    // Not connected - grey
    lv_obj_set_style_bg_color(ui_PanelWiFiQuality, lv_color_hex(0x808080), selector);
    _ui_label_set_property(ui_RSSI, _UI_LABEL_PROPERTY_TEXT, "N/C"); // Not/Connected
    return;
  }

  // Update the RSSI strength value with dBm unit
  _ui_label_set_property(ui_RSSI, _UI_LABEL_PROPERTY_TEXT, (String(rssi) + "dBm").c_str());

  // Update the RSSI indicator circle colours
  if (rssi >= -50) {
    lv_obj_set_style_bg_color(ui_PanelWiFiQuality, lv_color_hex(0x41C241), selector); // excellent - green
  }
  else if (rssi >= -60) {
    lv_obj_set_style_bg_color(ui_PanelWiFiQuality, lv_color_hex(0xACC241), selector); // good - light green
  }
  else if (rssi >= -70) {
    lv_obj_set_style_bg_color(ui_PanelWiFiQuality, lv_color_hex(0xBC9534), selector); // fair - yellow
  }
  else {
    lv_obj_set_style_bg_color(ui_PanelWiFiQuality, lv_color_hex(0xCE2C2C), selector); // poor - red
  }
}


/*************************************************************
*********************** MAIN FUNCTIONS ***********************
**************************************************************/

// SETUP
void setup() {
  LCD_Init();
  Lvgl_Init();

  Set_Backlight(75);
  Set_Color(200, 200, 2);

  ui_init();

  connectWifi();
  setTime();

  // Update IP Address
  _ui_label_set_property(ui_ipLBL, _UI_LABEL_PROPERTY_TEXT, ipAddress.c_str());
}

// MAIN LOOP
void loop() {
  Timer_Loop(); // call lv_timer_handler & 5ms delay via LVGL Driver

  // FPS calculation (runs every second)
  frameCount++;
  if (millis() - lastFPSTime >= 1000) {
    currentFPS = frameCount / ((millis() - lastFPSTime) / 1000);
    frameCount = 0;
    lastFPSTime = millis();
    
    // Update all time/date related UI variables
    if(rtc.getSecond() != last_second) {
      last_second = rtc.getSecond();
      
      // Time
      _ui_label_set_property(ui_timeLBL, _UI_LABEL_PROPERTY_TEXT, rtc.getTime().substring(0,5).c_str());
      _ui_label_set_property(ui_secLBL, _UI_LABEL_PROPERTY_TEXT, rtc.getTime().substring(6,8).c_str());
      
      // Date
      dateString = months[rtc.getMonth()] + " " + String(rtc.getDay());
      _ui_label_set_property(ui_dateLBL, _UI_LABEL_PROPERTY_TEXT, dateString.c_str());
      lv_obj_set_pos(ui_dayPNL, 121, pos[rtc.getDayofWeek()]);

      // Year
      _ui_label_set_property(ui_yearLBL, _UI_LABEL_PROPERTY_TEXT, String(rtc.getYear()).c_str());

      // NTP Sync
      _ui_label_set_property(ui_ntpSync, _UI_LABEL_PROPERTY_TEXT, (String(n) + "s").c_str());

      // Week number
      _ui_label_set_property(ui_weekLBL, _UI_LABEL_PROPERTY_TEXT, (String(getWeekNumber())).c_str());

      // FPS value
      _ui_label_set_property(ui_FPS, _UI_LABEL_PROPERTY_TEXT, String(currentFPS).c_str());

      // NTP sync countdown timer
      n--;
      if(n == 0) {
        n = 7200;
        setTime();
      }
    }
  }

  // Plasma ball rotation (runs continuously)
  angle = angle + 5; // increase to speed up rotation
  if (angle >= 3600) angle = 0;
  lv_img_set_angle(ui_cornerImage, angle);

  // WiFi and IP updates (runs every 5 seconds)
  if (millis() - lastWiFiUpdate >= wifiUpdateInterval) {
    updateWiFiQuality();
    
    // Only update IP if it's changed (Wi-Fi reconnected)
    String newIP = WiFi.localIP().toString();
    if (newIP != ipAddress) {
      ipAddress = newIP;
      _ui_label_set_property(ui_ipLBL, _UI_LABEL_PROPERTY_TEXT, ipAddress.c_str());
    }
    
    lastWiFiUpdate = millis();
  }

  // Handle the NeoPixel LED
  NeoPixel_Loop(3);
}
