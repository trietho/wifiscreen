# wifiscreen
Using  iPad, tablet, iPhone, android devices or other computers as an extended monitor for Microsoft Windows

# Introduction

On Windows OS (7 and later), you are able to extend the second monitor even with only one physical monitor. 

How to turn on the second desktop: **[Windows Key] + P**, then select **Extend**

WifiScreen is an free and open source application that help Windows users can use their mobile devices as their second physical monitor.

This application is built on top of TightVNC with minimum configurations for extending screen purposes only. Thus, every devices and computers that can run a VNC Viewer application then can be used as an extended monitor.
# Download the installations
- WifiScreen Setup for Win32 [WifiScreenSetup_win32.zip](https://github.com/trietho/wifiscreen/files/1538528/WifiScreenSetup_win32.zip)
- WifiScreen Setup for Win64 [WifiScreenSetup_win64.zip](https://github.com/trietho/wifiscreen/files/1538550/WifiScreenSetup_win64.zip)

# Usage

- Run WifiScreen application
- Run VNC Viewer application on devices/computer then connect to the server:port as shown in the WifiScreen Connector.

## Settings
- Select a screen or all screens to share to other devices
- Multiple connections: Allow multiple devices connect to your shared screen (e.g: for seminar purpose)
- Require password: Authentication is required when a device want to connect.
- Remote inputs: you can use mouse/keyboard on the connected devices to input. 

![WifiScreen Connector](screenshoots/WifiScreenConnector.png)

The settings when click on Configurations button (you can only change the settings in the highlighted boxes)

![WifiScreen Server Settings](screenshoots/WifiScreenServerConfiguration.png)

![WifiScreen Server Settings](screenshoots/WifiScreenConfiguration2.png)


Thanks to TightVNC team for the source code (https://www.tightvnc.com/download.php).
- Note: On this download page, you may want to try DFMirage Driver to improve WifiScreen Server performance (as they mentioned).

WifiScreen is an R&D project of http://www.3stone.vn/
