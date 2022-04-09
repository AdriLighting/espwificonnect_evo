#include "staconnect.h"
#include "wificonnectevo.h"

void WCEVO_STAconnect::init(WCEVO_server * server){
	_server = server; 
}

uint8_t WCEVO_STAconnect::get_reconnectAttempt() 									{return _reconnectAttempt;}
void 		WCEVO_STAconnect::get_reconnectAttempt(uint8_t & v1)			{v1 = _reconnectAttempt;}
void 		WCEVO_STAconnect::get_lastReconnectAttempt(uint32_t & v1)	{v1 = _lastReconnectAttempt;}
boolean WCEVO_STAconnect::get_active()  													{return _active;}
boolean WCEVO_STAconnect::get_serverInitialized()                 {return _serverInitialized;}
boolean WCEVO_STAconnect::get_wasConnected()                      {return _wasConnected;}


void WCEVO_STAconnect::set_wasConnected(boolean v1)       {_wasConnected = v1;}
void WCEVO_STAconnect::set_active(boolean v1)             {_active = v1;}
void WCEVO_STAconnect::set_serverInitialized(boolean v1)  {_serverInitialized = v1;}
void WCEVO_STAconnect::set_lastReconnectAttempt()	        {_lastReconnectAttempt = millis();}
void WCEVO_STAconnect::set_reconnectAttempt(boolean v1){
	if (v1) _reconnectAttempt++;
	else _reconnectAttempt = 0;
}

void WCEVO_STAconnect::reset() {
  // stop and en reset wifi netwotk scanner
  WCEVO_managerPtrGet()->set_scanNetwork_running(false);
  WCEVO_managerPtrGet()->set_scanNetwork_requiered(false);
  WCEVO_managerPtrGet()->networkScan()->scan_reset();
	
  // reset timer
  _lastReconnectAttempt = 0;
	_reconnectAttempt 		= 0;	


  _serverInitialized    = false;
  
  _active = false;
}


boolean WCEVO_STAconnect::setup() {
  // persistent mod not configured yet so if we dont hav any credential return void()
  // a modifier avec un retour d'érreur + configuration du mod pesistant
  //  
  if (!WCEVO_managerPtrGet()->credential()) {
    ALT_TRACEC("main", "[FATAL] no credential found\n");
    return false;
  }
  //

  // stack
  yield();

  // debug
  ALT_TRACEC("main", "Start of connection to the WiFi Router\n\t credListPos: %d - ", WCEVO_managerPtrGet()->_credentialPos);
  if (WCEVO_managerPtrGet()->credential()) WCEVO_managerPtrGet()->credential()->print();
  const char * ssid = "persistent";
  const char * pass = "persistent";
  // const char * hostName = "wcevo";
  if (WCEVO_managerPtrGet()->credential()) {
    WCEVO_managerPtrGet()->credential()->get_ssid(ssid);
    WCEVO_managerPtrGet()->credential()->get_psk(pass);    
  }
  ALT_TRACEC("main", "Attemp to connect to [ssid: %s] [psk: %s]\n", ssid, pass);
  //

  // reset timer
  _lastReconnectAttempt = millis();

  // if (WCEVO_managerPtrGet()->get_cm()==WCEVO_CM_STA && WCEVO_managerPtrGet()->get_AP()->get_active()) {
  //   ALT_TRACEC("main", "Access point disabled.\n");
  //   ALT_TRACEC("main", "&c:1&s:\tSet _AP.active to FALSE\n");
  //   WiFi.softAPdisconnect(true);
  //   WCEVO_managerPtrGet()->get_AP()->set_active(false);
  // }

  // attempt to connect using saved settings, on fail fallback to AP config portal
  if(!WiFi.enableSTA(true)){
    // handle failure mode Brownout detector etc.
    ALT_TRACEC("main","[FATAL] Unable to enable wifi!\n");
    return false;
  } else {ALT_TRACEC("main", "Set enableSTA(true) succes\n");}
  
  ALT_TRACEC("main", "Set autoReconnect to TRUE\n");
  WiFi.setAutoReconnect(true);

  #ifdef ESP8266
    const char * hostName = "wcevo";
    _server->get_hostName(hostName);  
    ALT_TRACEC("main", "Set hostname: %s\n", hostName);  
    WiFi.hostname(hostName);
  #endif   

  // WiFi.disconnect(true);
  WCEVO_managerPtrGet()->get_WM()->WiFi_Disconnect();

  if      (WCEVO_managerPtrGet()->get_cm()==WCEVO_CM_STAAP)  WiFi.mode(WIFI_AP_STA);
  else if (WCEVO_managerPtrGet()->get_cm()==WCEVO_CM_STA)    WiFi.mode(WIFI_STA);
  else WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, pass);

  if (!_active) {
    ALT_TRACEC("main", "Set _active to TRUE\n");
    _active = true;
  }
  
  return true;

    /*  
  if (ssid != (const char*)"") {
    #if defined(ESP8266)
      ETS_UART_INTR_DISABLE();
      wifi_station_disconnect();
      ETS_UART_INTR_ENABLE();
    #else
      WiFi.disconnect(false);
    #endif
    WiFi.begin(ssid, pass);
  } else {
    if (WiFi.SSID().length() > 0) {
      ALT_TRACEC("main", "Using last saved values, should be faster\n");
      #if defined(ESP8266)
        ETS_UART_INTR_DISABLE();
        wifi_station_disconnect();
        ETS_UART_INTR_ENABLE();
      #else
        WiFi.disconnect(false);
      #endif
      WiFi.begin();
    } else {
      ALT_TRACEC("main", "Try to connect with saved credentials\n");
      WiFi.begin();
    }
  }*/
  // #ifdef ARDUINO_ARCH_ESP32
  //   WiFi.setSleep(false);
  //   WiFi.setHostname(hostName);
  // #else
  //   wifi_set_sleep_type(MODEM_SLEEP_T);
  // #endif
}

/**
 * set sta config if set
 * @since $dev
 * @return bool success
 */
/*
bool WCEVO_STAconnect::setSTAConfig(){
  #ifdef WM_DEBUG_LEVEL
  ALT_TRACEC("main", "STA static IP:",_sta_static_ip);  
  #endif
  bool ret = true;
  if (_sta_static_ip) {
      #ifdef WM_DEBUG_LEVEL
      ALT_TRACEC("main", "Custom static IP/GW/Subnet/DNS");
      #endif
    if(_sta_static_dns) {
      #ifdef WM_DEBUG_LEVEL
      ALT_TRACEC("main", "Custom static DNS");
      #endif
      ret = WiFi.config(_sta_static_ip, _sta_static_gw, _sta_static_sn, _sta_static_dns);
    }
    else {
      #ifdef WM_DEBUG_LEVEL
      ALT_TRACEC("main", "Custom STA IP/GW/Subnet");
      #endif
      ret = WiFi.config(_sta_static_ip, _sta_static_gw, _sta_static_sn);
    }

      #ifdef WM_DEBUG_LEVEL
      if(!ret) ALT_TRACEC("main", "[ERROR] wifi config failed");
      else ALT_TRACEC("main", "STA IP set:",WiFi.localIP());
      #endif
  } 
  else {
      #ifdef WM_DEBUG_LEVEL
      ALT_TRACEC("main", "setSTAConfig static ip not set, skipping");
      #endif
  }
  return ret;
}
*/

