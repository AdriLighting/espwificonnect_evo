#include "staconnect.h"
#include "wificonnectevo.h"

void WCEVO_STAconnect::init(WCEVO_server * server){
  _server = server; 
}

uint8_t WCEVO_STAconnect::get_reconnectAttempt()                  {return _reconnectAttempt;}
void    WCEVO_STAconnect::get_reconnectAttempt(uint8_t & v1)      {v1 = _reconnectAttempt;}
void    WCEVO_STAconnect::get_lastReconnectAttempt(uint32_t & v1) {v1 = _lastReconnectAttempt;}
boolean WCEVO_STAconnect::get_active()                            {return _active;}
boolean WCEVO_STAconnect::get_serverInitialized()                 {return _serverInitialized;}
boolean WCEVO_STAconnect::get_wasConnected()                      {return _wasConnected;}


void WCEVO_STAconnect::set_wasConnected(boolean v1)       {_wasConnected = v1;}
void WCEVO_STAconnect::set_active(boolean v1)             {_active = v1;}
void WCEVO_STAconnect::set_serverInitialized(boolean v1)  {_serverInitialized = v1;}
void WCEVO_STAconnect::set_lastReconnectAttempt()         {_lastReconnectAttempt = millis();}
uint8_t WCEVO_STAconnect::set_reconnectAttempt(boolean v1){
  uint8_t ret=0;
  if (v1) _reconnectAttempt++;
  else {ret=_reconnectAttempt;_reconnectAttempt = 0;}
  return ret;
}

void WCEVO_STAconnect::reset() {
  // stop and en reset wifi netwotk scanner
  WCEVO_managerPtrGet()->set_scanNetwork_running(false);
  WCEVO_managerPtrGet()->set_scanNetwork_requiered(false);
  WCEVO_managerPtrGet()->networkScan()->scan_reset();
  
  // reset timer
  _lastReconnectAttempt = 0;
  _reconnectAttempt     = 0;  


  _serverInitialized    = false;
  
  _active = false;
}


boolean WCEVO_STAconnect::setup() {
  // persistent mod not configured yet so if we dont hav any credential return void()
  // a modifier avec un retour d'érreur + configuration du mod pesistant
  //  
  if (!WCEVO_managerPtrGet()->credential()) {
    ALT_TRACEC(WCEVO_DEBUGREGION_STA, "[FATAL] no credential found\n");
    return false;
  }

  //

  // stack
  yield();

  // debug
  ALT_TRACEC(WCEVO_DEBUGREGION_STA, "Start of connection to the WiFi Router\n\t credListPos: %d - ", WCEVO_managerPtrGet()->_credentialPos);
  WCEVO_managerPtrGet()->credential()->print();
  const char * ssid = "";
  const char * pass = "";
  // const char * hostName = "wcevo";
  WCEVO_managerPtrGet()->credential()->get_ssid(ssid);
  WCEVO_managerPtrGet()->credential()->get_psk(pass); 
  if ((strcmp(ssid, "") == 0) && (strcmp(pass, "") == 0)) {
    ALT_TRACEC(WCEVO_DEBUGREGION_STA, "[FATAL] credential found but ssid and psk is blank\n");
    return false;
  }
  uint8_t listSize = 0;
  int8_t listPos = 0;
  WCEVO_managerPtrGet()->networkScan()->get_SSID(al_tools::ch_toString(ssid), listSize, listPos);   
  ALT_TRACEC(WCEVO_DEBUGREGION_STA, "networkScan search result\n\t[ssid: %s] [listSize: %d] [listPos: %d]\n", ssid, listSize, listPos);
  if (listSize > 0 && listPos < 0) {
    ALT_TRACEC(WCEVO_DEBUGREGION_STA, "[FATAL] no ssid find in scan network list\n");
    return false;
  }
  ALT_TRACEC(WCEVO_DEBUGREGION_STA, "attempt to connect to the router [ssid: %s] [psk: %s]\n", ssid, pass);
  //

  // reset timer
  _lastReconnectAttempt = millis();

  // if (WCEVO_managerPtrGet()->get_cm()==WCEVO_CM_STA && WCEVO_managerPtrGet()->get_AP()->get_active()) {
  //   ALT_TRACEC(WCEVO_DEBUGREGION_STA, "Access point disabled.\n");
  //   ALT_TRACEC(WCEVO_DEBUGREGION_STA, "&c:1&s:\tSet _AP.active to FALSE\n");
  //   WiFi.softAPdisconnect(true);
  //   WCEVO_managerPtrGet()->get_AP()->set_active(false);
  // }

  // ALT_TRACEC(WCEVO_DEBUGREGION_STA, "WiFi disconnect false\n");  
  // WiFi.disconnect(false);

  ALT_TRACEC(WCEVO_DEBUGREGION_STA, "wifi_station_disconnect\n");  

    #if defined(ESP8266)
      ETS_UART_INTR_DISABLE ();
      wifi_station_disconnect ();
      ETS_UART_INTR_ENABLE ();  
    #elif defined(ESP32)
      WCEVO_managerPtrGet()->get_WM()->WiFi_Disconnect();
    #endif
  // WCEVO_managerPtrGet()->get_WM()->WiFi_Disconnect();

  if(!WiFi.enableSTA(true)){
    ALT_TRACEC(WCEVO_DEBUGREGION_STA,"[FATAL] Unable to enable wifi!\n");
    return false;
  } else {ALT_TRACEC(WCEVO_DEBUGREGION_STA, "WiFi enableSTA true -> succes\n");}
  
  ALT_TRACEC(WCEVO_DEBUGREGION_STA, "WiFi autoReconnect true\n");
  WiFi.setAutoReconnect(true);

  // ALT_TRACEC(WCEVO_DEBUGREGION_STA, "WiFi persistent: true\n");
  // WiFi.persistent(true);


  // if      (WCEVO_managerPtrGet()->get_cm()==WCEVO_CM_STAAP)  WiFi.mode(WIFI_AP_STA);
  // else if (WCEVO_managerPtrGet()->get_cm()==WCEVO_CM_STA)    WiFi.mode(WIFI_STA);
  // else WiFi.mode(WIFI_STA);
  // 
  ALT_TRACEC(WCEVO_DEBUGREGION_STA, "WiFi mode: WIFI_STA\n");  
  WiFi.mode(WIFI_STA);

  // #ifdef ESP8266
    const char * hostName = "wcevo";
    _server->get_hostName(hostName);  
    ALT_TRACEC(WCEVO_DEBUGREGION_STA, "WiFi hostname: %s\n", hostName);  
    WiFi.hostname(hostName);
  // #endif   


  WiFi.begin(ssid, pass);

  if (!_active) {
    ALT_TRACEC(WCEVO_DEBUGREGION_STA, "Set _active to TRUE\n");
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
      ALT_TRACEC(WCEVO_DEBUGREGION_STA, "Using last saved values, should be faster\n");
      #if defined(ESP8266)
        ETS_UART_INTR_DISABLE();
        wifi_station_disconnect();
        ETS_UART_INTR_ENABLE();
      #else
        WiFi.disconnect(false);
      #endif
      WiFi.begin();
    } else {
      ALT_TRACEC(WCEVO_DEBUGREGION_STA, "Try to connect with saved credentials\n");
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
  ALT_TRACEC(WCEVO_DEBUGREGION_STA, "STA static IP:",_sta_static_ip);  
  #endif
  bool ret = true;
  if (_sta_static_ip) {
      #ifdef WM_DEBUG_LEVEL
      ALT_TRACEC(WCEVO_DEBUGREGION_STA, "Custom static IP/GW/Subnet/DNS");
      #endif
    if(_sta_static_dns) {
      #ifdef WM_DEBUG_LEVEL
      ALT_TRACEC(WCEVO_DEBUGREGION_STA, "Custom static DNS");
      #endif
      ret = WiFi.config(_sta_static_ip, _sta_static_gw, _sta_static_sn, _sta_static_dns);
    }
    else {
      #ifdef WM_DEBUG_LEVEL
      ALT_TRACEC(WCEVO_DEBUGREGION_STA, "Custom STA IP/GW/Subnet");
      #endif
      ret = WiFi.config(_sta_static_ip, _sta_static_gw, _sta_static_sn);
    }

      #ifdef WM_DEBUG_LEVEL
      if(!ret) ALT_TRACEC(WCEVO_DEBUGREGION_STA, "[ERROR] wifi config failed");
      else ALT_TRACEC(WCEVO_DEBUGREGION_STA, "STA IP set:",WiFi.localIP());
      #endif
  } 
  else {
      #ifdef WM_DEBUG_LEVEL
      ALT_TRACEC(WCEVO_DEBUGREGION_STA, "setSTAConfig static ip not set, skipping");
      #endif
  }
  return ret;
}
*/

