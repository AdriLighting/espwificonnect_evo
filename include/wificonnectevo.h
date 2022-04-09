/*
	espwificonnect_evo is a library for the framework-arduinoespressif8266/32 
		configuration and reconfiguration of WiFi credentials using a Captive Portal


	library used for my personalproject
		fetures requiered 
			- asyncrone mod
			- multiple credential with best ssid rssi
			- STA + AP || STA || AP
			- captive portal 
				- station mod
				- credential || multiple  
				- info
					- ip
					- hostname

	inspired by:
		library:
			https://github.com/Aircoookie/WLED/blob/main/wled00/wled.cpp#L684-L777
			https://github.com/tzapu/WiFiManager
		 	https://github.com/alanswx/ESPAsyncWiFiManager
		 	https://gist.github.com/tablatronix/497f3b299e0f212fc171ac5662fa7b42
		 	framework-arduinoespressif8266 - https://github.com/esp8266/Arduino
		 		ESP8266WiFi - ESP8266WiFiMulti.h
			https://github.com/disk91/esp8266-sigfox-trackr
			https://github.com/overtone1000/uDAQC/blob/db2439e05dfa67c49c9bfb5320c6c1842942f3d9/Components/Device/src/Network/Network.cpp
		other:
			https://github.com/esp8266/Arduino/blame/c55f49bd6103dab81c6a389470f6d5bbbee399d0/boards.txt#L420-L471
			https://stackoverflow.com/questions/46289283/esp8266-captive-portal-with-pop-up
*/

#ifndef _WIFICONNECTEVO_H
#define _WIFICONNECTEVO_H

	#include "def.h"

	#include "wcevo_scan.h"
	#include "credential.h"
	#include "staconnect.h"
	#include "apconnect.h"
	#include "wcevo_wifimanager.h"

	const char * const WIFI_STA_STATUS[] PROGMEM
	{
	  "WL_IDLE_STATUS",     // 0 STATION_IDLE
	  "WL_NO_SSID_AVAIL",   // 1 STATION_NO_AP_FOUND
	  "WL_SCAN_COMPLETED",  // 2
	  "WL_CONNECTED",       // 3 STATION_GOT_IP
	  "WL_CONNECT_FAILED",  // 4 STATION_CONNECT_FAIL, STATION_WRONG_PASSWORD(NI)
	  "WL_CONNECTION_LOST", // 5
	  "WL_DISCONNECTED",    // 6
	  "WL_STATION_WRONG_PASSWORD" // 7 KLUDGE
	};
	const char S_NA[]                 PROGMEM = "Unknown";

	const char WCEVO_PTJSON_001[] PROGMEM = "credentials";
	const char WCEVO_PTJSON_002[] PROGMEM = "credential";


	/**
	 * mod de connection a l'initialisation du programm 
	 * STA
	 * 	STA -> si sta nn dispo ou si connection pas etblie -> reset
	 * STAAP 
	 * 	STA -> si sta nn dispo ou si connection pas etblie -> AP
	 * AP
	 * 	ap
	 */
  typedef enum : uint8_t {
    WCEVO_CM_STA = 0,
    WCEVO_CM_AP,
    WCEVO_CM_STAAP,
    WCEVO_CM_NONE
  } wcevo_connectmod_t;
	const char* const wcevo_connectmod_s[] PROGMEM = { "CM_STA", "CM_AP", "CM_STAAP", "CM_NONE" };
  extern wcevo_connectmod_t wcevo_connectmodArray_t[];

  /**
   * ???
   */
  typedef enum : uint8_t {
    WCEVO_CF_RESET = 0,
    WCEVO_CF_NEXTAP,
    WCEVO_CF_AP,
    WCEVO_CF_NONE
  } wcevo_connectfail_t;
	const char* const wcevo_connectfail_s[] PROGMEM = { "CF_RESET", "CF_NEXTAP", "CF_AP", "CF_NONE" };
  extern wcevo_connectfail_t wcevo_connectfaildArray_t[];


	class WCEVO_manager
	{
		const char * config_filepath = "/wcevo_config.json";

    AsyncWebServer  * _webserver ;
    DNSServer       * _dnsServer ;
    
		WCEVO_server								* _server 			= nullptr;
		LList<WCEVO_credential *>  	_credentials;
		WCEVO_credential 						* _credential 	= nullptr;
		boolean 										_credentialUse 	= false;

		WCEVO_scanNetwork						_scanNtwork;

		WCEVO_APconnect 						_APCO;
		WCEVO_STAconnect 						_STACO;
		WiFiManagerCPY 							_WM;
//
    wcevo_connectmod_t 	_CONNECTMOD		= wcevo_connectmod_t::WCEVO_CM_AP;	
    wcevo_connectfail_t _CONNECTFAIL 	= wcevo_connectfail_t::WCEVO_CF_NEXTAP;	
		
		boolean _scanNetwork_gotSSID = false;

		boolean _otaSetup = false;

		boolean _scanNetwork_running = true;
		boolean _scanNetwork_requiered = false;



	public:
		uint8_t _credentialPos 	= 0;

		// WCEVO_manager(WCEVO_server * cr, DNSServer*dnsServer,AsyncWebServer*webserver);
		WCEVO_manager(const char * const & v1 = NULL, const char * const & v2 = NULL, const char * const & v3 = NULL, DNSServer* v4 = nullptr,AsyncWebServer* v5 = nullptr);
		WCEVO_manager(const char * const & v1 = NULL, const char * const & v2 = NULL, DNSServer* v4 = nullptr,AsyncWebServer* v5 = nullptr);
		WCEVO_manager(const char * const & v1 = NULL, DNSServer* v4 = nullptr,AsyncWebServer* v5 = nullptr);
		WCEVO_manager(DNSServer* v4 = nullptr,AsyncWebServer* v5 = nullptr);
		~WCEVO_manager();
		void init(const char * const &, const char * const &, const char * const &);
		
		WCEVO_server 								* server();
		LList<WCEVO_credential *> 	* credentials();
		WCEVO_credential 						* credential();
		WCEVO_scanNetwork						* networkScan();
		WCEVO_STAconnect 						* get_STA();
		WCEVO_APconnect							* get_AP();
		WiFiManagerCPY 							* get_WM();

	private:

		void credentials_delete();

		void set_credential(WCEVO_credential*&, JsonArray & arr_1);
		void set_credential(JsonArray & arr_1);
		void set_credential(WCEVO_credential*&);

		WCEVO_credential * credential_getBestSSID();


		void get_credential_json(WCEVO_credential*, JsonArray &);
		void get_credentials_json(JsonArray &);
		void get_credentials_json(WCEVO_credential*, DynamicJsonDocument &);
		void get_credentials_json(DynamicJsonDocument &);


		boolean 	isConnected();		
		IPAddress localIP();
		void 			sta_loop();
		void 			sta_reconnect();
		void 			sta_reconnect_end(boolean apSetup = true);
		uint8_t 	sta_getMaxAettemp();
	public:
		void set_credentialUse(boolean);
		boolean get_credentialUse();

		wcevo_connectmod_t get_cm();
		wcevo_connectfail_t get_cmFail();
		void set_cm(wcevo_connectmod_t);
		void set_cmFail(wcevo_connectfail_t);

		boolean get_scanNetwork_running();
		boolean get_scanNetwork_requiered();
		void set_scanNetwork_running(boolean);
		void set_scanNetwork_requiered(boolean);

		void set_credential(uint8_t);
		void set_credential(const String &, const String &);

		void credential_print();

		uint8_t credentials_add(const char * const &, const char * const & );
		void credentials_print();

		void mdns_setup();
		void ota_setup();

		void handleConnection();

		void start();
		void print();

		#ifdef FILESYSTEM
			void credentials_to_fs(
				boolean cu,
				wcevo_connectmod_t cm = WCEVO_CM_NONE, 
				wcevo_connectfail_t cmf = WCEVO_CF_NONE);	
			void credentials_to_fs(
				wcevo_connectmod_t cm = WCEVO_CM_NONE, 
				wcevo_connectfail_t cmf = WCEVO_CF_NONE);	
			
			void credentials_from_fs();	
		#endif		
//

		/*
		void initSTA();
		void initAP();
		
		;
		

		String getWLStatusString(uint8_t status);
		String getWLStatusString();
		*/

/*
		bool wifiConnectNew(const char * ssid, const char * pass,bool connect);
		bool WiFi_Mode(WiFiMode_t m,bool persistent);
		bool WiFi_Mode(WiFiMode_t m);
		bool WiFi_enableSTA(bool enable,bool persistent);
		bool WiFi_enableSTA(bool enable);
*/

	};
	WCEVO_manager * WCEVO_managerPtrGet();


#endif // MAIN_H