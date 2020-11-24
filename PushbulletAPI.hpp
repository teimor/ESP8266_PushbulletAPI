#ifndef _PUSHBULLETAPI_H_
#define _PUSHBULLETAPI_H_

#include <Arduino.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>


// #define DEBUG_PUSHBULLETAPI true

class PushbulletAPI{	
	public:
	  PushbulletAPI(const String t_accessToken);
	  PushbulletAPI(const String t_accessToken, const String t_fingerprint, bool t_verifiedConnectionOnly = false);
	  bool connectionStatus();
	  void pushNotifcation(String t_title, String t_body);
	  void pushNotifcationUrl(String t_title, String t_body, String t_url);
    void pushNotifcationEmail(String t_title, String t_body, String t_email);

	  const String PUSHBULLET_HOST = "api.pushbullet.com";
	  const String PUSHBULLET_PUSH_URL = "/v2/pushes";
	  const int PUSHBULLET_PORT = 443;
		
	private:
	  String m_accessToken;
	  String m_fingerprint;

		WiFiClientSecure * m_secure_client;
		unsigned int m_connectionLastState; // 0 - Init/Unable to connect, 1 - Connected, 2 - Insecure Connection.
	  bool m_verifiedConnectionOnly;
	  bool m_isFingerprintVerified;

	  bool getConnection();
	  bool openConnection();
	  bool openInsecureConnection();
	  void closeConnection();
	  String createHttpPostRequest(String t_host, String t_url, String t_accessToken ,String t_messagebody);
	  void sendPushbulletHttpRequest(String t_messagebody);

	  void serialPrint(const String t_message, const int t_msgType = 0);
	  void debugResponse();

};
#endif