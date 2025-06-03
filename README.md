# Waveshare ESP32-C6-LCD-1.47 Internet Clock Project with SquareLine Studio UI

## Description:
This code implements an internet clock with Wi-Fi connectivity on the Waveshare ESP32-C6 1.47" LCD display in landscape mode. The project uses Squareline Studio to create the UI interface and includes time/date display, Wi-Fi status monitoring, and visual effects. The clock synchronizes with NTP servers and maintains time using the ESP32's RTC capabilities.

## How it works:
- Initialization: Sets up display, LVGL, and WiFi connection
- Time Synchronization: Gets current time from NTP server at startup and every 2 hours
- Display Updates: Continuously updates clock face with smooth transitions
- Wi-Fi Monitoring: Regularly checks and displays connection quality
- Animation: Maintains rotating corner graphic and LED effects

## Notes:
- Time zone offset is configurable (default GMT+2)
- Edit the Wi-Fi SSID & password variables to your network
- Backlight brightness is set to 75% by default
- Had to comment out line 5 of LVGL_Driver.h

## SLS Project Files:
- This repository includes the SquareLine Studio project files in: **'.\Waveshare-ESP32-C6-LCD-1.47-PlasmaBall-Internet_Clock\sls_files'**
- In the **'sls_files'** folder, there are 2 subfolders: **'export'** & **'project'**
- Open SquareLine_Project in the **'project'** folder with Squareline Studio to make changes to the UI.
- You may need to update the SLS project settings **Project Export Root** & **UI Files Export Path** locations to reflect where you have saved the Arduino project **before exporting**.
- Export project files to the **'export'** folder & copy all, then replace all files in the **root** of the Arduino project folder.
- **Do not export into the root Arduino project folder as SLS will erase the folder contents before exporting!**
- **NB!!** Every time a change is made in SLS, & the UI files have been replaced in the Arduino project folder - you **must** edit the ui.h file on line 30: **FROM** *#include "screens/ui_MainScreen.h"* **TO** *#include "ui_MainScreen.h"* (remove *'screens/'* -- I'm not sure why SLS is exporting this way as the flat export option is selected.)

## Credits:
This project is inspired by [Volos Projects - waveshareBoards](https://github.com/VolosR/waveshareBoards) modified for the Waveshare ESP32-C6-LCD-1.47
