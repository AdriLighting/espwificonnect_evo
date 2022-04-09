/*
  espwificonnect_evo is a library for the framework-arduinoespressif8266/32 
    configuration and reconfiguration of WiFi credentials using a Captive Portal


  library used for my personalproject
    fetures requiered 
      - asyncrone mod
      - multiple credential with sorted ssid-rssi
      - STA + AP || STA || AP
      - captive portal 
        - station mod
        - credential || multiple credential 
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


#include "wificonnectevo.h" 
#include <altoolslib.h>
  
WCEVO_credential * _temp_WCEVO_credential = nullptr;

#define WIFISCAN_SSIDFILTERLEN  5

    wcevo_connectmod_t wcevo_connectmodArray_t[] = {
    WCEVO_CM_STA,
    WCEVO_CM_AP,
    WCEVO_CM_STAAP,
    WCEVO_CM_NONE
    };
    wcevo_connectfail_t wcevo_connectfaildArray_t[] = {
    WCEVO_CF_RESET,
    WCEVO_CF_NEXTAP,
    WCEVO_CF_AP,
    WCEVO_CF_NONE
    };

WCEVO_manager * WCEVO_managerPtr = nullptr;
WCEVO_manager * WCEVO_managerPtrGet(){return WCEVO_managerPtr;}
void WCEVO_manager::init(const char * const & Host, const char * const & APpass, const char * const & OTApass){
  _temp_WCEVO_credential = new WCEVO_credential("", "");
  _server = new WCEVO_server(Host,APpass,OTApass);  
  _APCO.init(_server, _dnsServer, _webserver);
  _STACO.init(_server);   
} 
WCEVO_manager::WCEVO_manager( const char * const & Host, const char * const & APpass, const char * const & OTApass, 
  DNSServer*dnsServer,AsyncWebServer*webserver) : _webserver(webserver), _dnsServer(dnsServer) {
  WCEVO_managerPtr = this;
  init(Host,APpass,OTApass);  
}
WCEVO_manager::WCEVO_manager( const char * const & Host, const char * const & APpass, 
  DNSServer*dnsServer,AsyncWebServer*webserver) : _webserver(webserver), _dnsServer(dnsServer) {
  WCEVO_managerPtr = this;
  init(Host,APpass,"wcevo_1234");  
}
WCEVO_manager::WCEVO_manager( const char * const & Host, DNSServer*dnsServer, AsyncWebServer*webserver) :
 _webserver(webserver), _dnsServer(dnsServer) {
  WCEVO_managerPtr = this;
  init(Host,"","wcevo_1234");  
}
WCEVO_manager::WCEVO_manager( DNSServer*dnsServer, AsyncWebServer*webserver) : 
  _webserver(webserver), _dnsServer(dnsServer) {
  WCEVO_managerPtr = this;
  init("wcevo","","wcevo_1234");  
}

// WCEVO_manager::WCEVO_manager(WCEVO_server * ptr, DNSServer*dnsServer,AsyncWebServer*webserver){
//  WCEVO_managerPtr = this;
//  _server = new WCEVO_server(ptr);
//   _temp_WCEVO_credential = new WCEVO_credential("", "");
//   _APCO.init(_server, dnsServer, webserver);
//   _STACO.init(_server);
// }
WCEVO_manager::~WCEVO_manager(){}

WCEVO_server * WCEVO_manager::server(){
  return _server;
}
WCEVO_scanNetwork * WCEVO_manager::networkScan(){
  return &_scanNtwork;
}
WCEVO_credential * WCEVO_manager::credential(){

  return (!_credential)?_temp_WCEVO_credential:_credential;}

void WCEVO_manager::start(){
  uint8_t credsSize = _credentials.size();

  /*ALT_TRACEC("main", "cred size: %d\n", credsSize);*/
  /*ALT_TRACEC("main", "_scanNetwork_running: %d\n", _scanNetwork_running);*/

  if ((credsSize <= 0) && !_temp_WCEVO_credential) {
    _CONNECTMOD = wcevo_connectmod_t::WCEVO_CM_AP;
  }
  if (!_temp_WCEVO_credential && _credentialUse) _credentialUse = false;
  if (credsSize < 2) {
    if (credsSize > 0) set_credential(0);
    if ((credsSize <= 0)  && !_credentialUse && _temp_WCEVO_credential) _credentialUse =true;
    _CONNECTFAIL = wcevo_connectfail_t::WCEVO_CF_AP;
    _scanNetwork_running = false;
  }
  else {
    _CONNECTFAIL = wcevo_connectfail_t::WCEVO_CF_NEXTAP;
    if (_temp_WCEVO_credential && _credentialUse) _scanNetwork_running = false;
    else _scanNetwork_running = true;
  }
  // if (_CONNECTFAIL == wcevo_connectfail_t::WCEVO_CF_NEXTAP && credsSize < 2) {
  //   _CONNECTFAIL = wcevo_connectfail_t::WCEVO_CF_AP;
  // }  
}
void WCEVO_manager::print(){
  uint8_t p = (uint8_t)_CONNECTMOD;
  String cm = FPSTR(wcevo_connectmod_s[p]);
  p = (uint8_t)_CONNECTFAIL;
  String cmf = FPSTR(wcevo_connectfail_s[p]);
  ALT_TRACEC("main", "\n\tcm: %s\n\tcmf: %s\n\tcu: %d\n\tscan_runnig: %d\n\tscan_requiered: %d\n",
    cm.c_str(),
    cmf.c_str(),
    _credentialUse,
    _scanNetwork_running,
    _scanNetwork_requiered);
  _server->print();
  credential_print();
  credentials_print();  

}
void WCEVO_manager::set_credential(WCEVO_credential*&ptr){
  _credential=ptr;
  if (_temp_WCEVO_credential) delete _temp_WCEVO_credential;
  _temp_WCEVO_credential = new WCEVO_credential(_credential->get_ssid().c_str(), _credential->get_psk().c_str());
}
void WCEVO_manager::set_credential(uint8_t pos){
  _credential = _credentials.get(pos);
  if (_temp_WCEVO_credential) delete _temp_WCEVO_credential;
  _temp_WCEVO_credential = new WCEVO_credential(_credential->get_ssid().c_str(), _credential->get_psk().c_str());  
}
void WCEVO_manager::set_credential(WCEVO_credential *& ptr, JsonArray & arr_1){
  String ssid = arr_1[0].as<String>();
  String psk  = arr_1[1].as<String>();
  if (_temp_WCEVO_credential) delete _temp_WCEVO_credential;
  _temp_WCEVO_credential = new WCEVO_credential(ssid.c_str(), psk.c_str());
  ptr = _temp_WCEVO_credential;
}
void WCEVO_manager::set_credential(JsonArray & arr_1){
  String ssid = arr_1[0].as<String>();
  String psk  = arr_1[1].as<String>();
  if (_temp_WCEVO_credential) delete _temp_WCEVO_credential;
  _temp_WCEVO_credential = new WCEVO_credential(ssid.c_str(), psk.c_str());
  _credential = _temp_WCEVO_credential;
}
void WCEVO_manager::set_credential(const String & ssid,const String & psk){
  if (_temp_WCEVO_credential) delete _temp_WCEVO_credential;
  _temp_WCEVO_credential = new WCEVO_credential(ssid.c_str(), psk.c_str());
  _credential = _temp_WCEVO_credential;
}


uint8_t WCEVO_manager::credentials_add(const char * const & v1, const char * const & v2){
  _credentials.add( new WCEVO_credential(v1, v2) ); 
  return _credentials.size()-1;
}
void WCEVO_manager::credentials_delete(){
  if (_credentials.size() <= 0) return;
  while (_credentials.size()) {
    WCEVO_credential *ptr = _credentials.shift();
    delete ptr;
  }
  _credentials.clear();
}

void WCEVO_manager::credential_print(){
  if (!_credential) _credential = _temp_WCEVO_credential;
  if (!_credential) {
    ALT_TRACEC("main", "credential not instanced\n");
    return;
  }
  ALT_TRACEC("main", "-\n");
  _credential->print();
}

WCEVO_credential * WCEVO_manager::credential_getBestSSID(){
  if (_credentials.size() <= 0) {_scanNetwork_gotSSID = true; return nullptr;}
  int pos = networkScan()->get_bestSSID();
  if (pos == -1) {_scanNetwork_gotSSID = true; return nullptr;}
  Serial.printf(" >>> IS TESTED: %d/%d = %d\n", pos, _credentials.size(), _credentials.get(pos)->get_tested());
  _credentialPos = pos; 
  return _credentials.get(pos);
}


// FPSTR(WCEVO_PTJSON_002)
void WCEVO_manager::get_credential_json(WCEVO_credential * ptr, JsonArray & arr_1){
  if (!ptr) return;
  const char * ssid;
  const char * psk;
  ptr->get_ssid(ssid);
  ptr->get_psk(psk);
  JsonArray arr_2 = arr_1.createNestedArray();
  arr_2.add(ssid);    
  arr_2.add(psk);      
}
// FPSTR(WCEVO_PTJSON_001)
void WCEVO_manager::get_credentials_json(JsonArray & arr_1){
  for(int i = 0; i < _credentials.size(); ++i) {
    WCEVO_credential * item = _credentials.get(i);
    get_credential_json(item, arr_1);
  }
}
void WCEVO_manager::get_credentials_json(WCEVO_credential * ptr, DynamicJsonDocument & doc){
  JsonArray arr_1 = doc.createNestedArray(FPSTR(WCEVO_PTJSON_001));  
  get_credentials_json(arr_1);
  arr_1 = doc.createNestedArray(FPSTR(WCEVO_PTJSON_002));  
  get_credential_json(ptr, arr_1);
} 
void WCEVO_manager::get_credentials_json(DynamicJsonDocument & doc){
  get_credentials_json((_credential)?_credential:_temp_WCEVO_credential, doc);
} 

#ifdef FILESYSTEM
/*
  API
    credentials   array [["",""],["",""],...]
    credential    array [["",""]]
    cm            object
    cmf           object

  sauvegardes de tous les parametres wifi en une fois
    a modifer pour ne sauvegarder que les parametres voullu
      - plusieur fichies json || system d'ecriture via numero de ligne dans un fichier
  
  items:  [
            ["ssid", "psk"],["ssid", "psk"],...
          ]

  current : [["ssid", "psk"]]
    array = array[0]
    ssid  = array[0]
    psk   = array[1]
*/
void WCEVO_manager::credentials_to_fs(wcevo_connectmod_t cm, wcevo_connectfail_t cmf){
  credentials_to_fs(_credentialUse, cm, cmf);
}
void WCEVO_manager::credentials_to_fs(boolean cu, wcevo_connectmod_t cm, wcevo_connectfail_t cmf){
  File f=FILESYSTEM.open(config_filepath,"w");
  ALT_TRACEC("main", "-\n");
  if (!f) {
    ALT_TRACEC("main", "[Error open /w]\n\t%s\n", config_filepath);  
    return;
  } 
  DynamicJsonDocument doc(2048);
  get_credentials_json(_temp_WCEVO_credential, doc);
  doc[F("cm")]  = (cm==WCEVO_CM_NONE  )?_CONNECTMOD:cm;
  doc[F("cmf")] = (cmf==WCEVO_CF_NONE )?_CONNECTFAIL:cmf;
  doc[F("cu")]  = cu;
  serializeJson(doc, f);   
  serializeJsonPretty(doc, Serial);
  Serial.println();   
}
void WCEVO_manager::credentials_from_fs(){
  ALT_TRACEC("main", "read filepath: %s\n", config_filepath);
  File f=FILESYSTEM.open(config_filepath,"r");
  if (!f) {
    ALT_TRACEC("main", "[Error open /r]\n\t%s\n", config_filepath);  
    return;
  } 
  DynamicJsonDocument doc(2048);
  deserializeJson(doc, f);
  JsonArray arr_1;
  JsonArray arr_2;
  if (doc[F("credentials")]) {
    arr_1 = doc["credentials"];
    for(size_t i = 0; i < arr_1.size(); ++i) {
      arr_2 = arr_1[i];
      String iS = arr_2[0].as<String>();
      String iP = arr_2[1].as<String>();
      credentials_add(iS.c_str(), iP.c_str());
    }
  }  
  if (doc[F("credential")]) {
    arr_1 = doc["credential"];
    arr_1 = arr_1[0];
    ALT_TRACEC("main", "&c:1&s:->load creadential\n\tssid: %s\n\tpsk: %s\n", arr_1[0].as<String>().c_str(), arr_1[1].as<String>().c_str());
    set_credential(arr_1);
  }
  if (doc.containsKey(F("cm"))) {
    uint8_t p = doc[F("cm")].as<uint8_t>();
    ALT_TRACEC("main", "&c:1&s:->load cm\n\t%s\n", wcevo_connectmod_s[p]);
    _CONNECTMOD = wcevo_connectmodArray_t[p];
  }  
  if (doc.containsKey(F("cmf"))) {
    uint8_t p = doc[F("cmf")].as<uint8_t>();
    ALT_TRACEC("main", "&c:1&s:->load cmf\n\t%s\n", wcevo_connectfail_s[p]);
    _CONNECTFAIL = wcevo_connectfaildArray_t[p];
  }  
  if (doc.containsKey(F("cu"))) {
    bool cu = doc["cu"];
    ALT_TRACEC("main", "&c:1&s:->load cu\n\t%d\n", cu);
    _credentialUse = cu;
  }  

} 

#endif

LList<WCEVO_credential *> * WCEVO_manager::credentials() {
  return &_credentials;
}

void WCEVO_manager::credentials_print(){
  ALT_TRACEC("main", "size: %d\n", _credentials.size());
  for(int i = 0; i < _credentials.size(); ++i) {
    WCEVO_credential * item = _credentials.get(i);
    const char * ssid;
    const char * psk;
    item->get_ssid(ssid);
    item->get_psk(psk);
    Serial.printf_P(PSTR("[%d] ssid: %-20s psk: %-20s\n"), i, ssid, psk);
  }
}

void WCEVO_manager::set_cm(wcevo_connectmod_t v1)         {_CONNECTMOD=v1;};
void WCEVO_manager::set_cmFail(wcevo_connectfail_t v1)    {_CONNECTFAIL=v1;};

void WCEVO_manager::set_credentialUse(boolean v1)         {_credentialUse=v1;};
boolean WCEVO_manager::get_credentialUse()            {return _credentialUse;};

void WCEVO_manager::set_scanNetwork_running(boolean v1)   {
  _scanNetwork_running=v1;
  if (!_scanNetwork_running) networkScan()->scan_stop();
};
void WCEVO_manager::set_scanNetwork_requiered(boolean v1) {
  _scanNetwork_requiered=v1;
  if (!_scanNetwork_requiered) networkScan()->scan_stop();
};

boolean WCEVO_manager::get_scanNetwork_running()    {return _scanNetwork_running;};
boolean WCEVO_manager::get_scanNetwork_requiered()  {return _scanNetwork_requiered;};

wcevo_connectmod_t  WCEVO_manager::get_cm()       {return _CONNECTMOD;};
wcevo_connectfail_t WCEVO_manager::get_cmFail()   {return _CONNECTFAIL;};

WiFiManagerCPY      * WCEVO_manager::get_WM()     {return &_WM;}
WCEVO_STAconnect    * WCEVO_manager::get_STA()    {return &_STACO;}
WCEVO_APconnect     * WCEVO_manager::get_AP()     {return &_APCO;}

void WCEVO_manager::mdns_setup(){
  MDNS.end(); 
  #if defined(ESP8266)
    MDNS.begin(_server->get_hostName());  
  #elif defined(ESP32)
    MDNS.begin(_server->get_hostName().c_str());  
  #endif
  MDNS.addService("http", "tcp", 80);
  ALT_TRACEC("main", "mDNS started\n");
}

boolean WCEVO_manager::isConnected(){
  return (WiFi.localIP()[0] != 0 && WiFi.status() == WL_CONNECTED);
}
IPAddress WCEVO_manager::localIP() {
  IPAddress localIP;
  localIP = WiFi.localIP();
  if (localIP[0] != 0) {
    return localIP;
  }

  return INADDR_NONE;
}


uint8_t WCEVO_manager::sta_getMaxAettemp(){ 
  switch (_CONNECTFAIL) {
    case WCEVO_CF_NEXTAP:
       return 1;
    break;
    case WCEVO_CF_AP:
      return 1;
    break;
    default: return 1; break;
   } 
}
void WCEVO_manager::sta_reconnect(){ 
  _STACO.set_reconnectAttempt(true);

  ALT_TRACEC("main", "ReconnectAttempt: %d\n", _STACO.get_reconnectAttempt() );

  if (!_credential && _CONNECTFAIL == wcevo_connectfail_t::WCEVO_CF_NEXTAP) {
    ALT_TRACEC("main", "&c:1&s:\t networkscan\n" );
    _STACO.set_lastReconnectAttempt();
    _credential           = nullptr;
    _scanNetwork_running  = true;  
    networkScan()->scan_reset();    
  } else { 
    ALT_TRACEC("main", "&c:1&s:\t initSTA\n" );
    _STACO.setup();
  }  
}
void WCEVO_manager::sta_reconnect_end(boolean apSetup){
  ALT_TRACEC("main", "-\n")
  _STACO.set_reconnectAttempt(false);
  if (_CONNECTFAIL != wcevo_connectfail_t::WCEVO_CF_NEXTAP) _STACO.set_lastReconnectAttempt();
  _credential           = nullptr;
  _scanNetwork_running  = false;
  networkScan()->scan_reset();
  if (apSetup && !_APCO.get_active()) _APCO.setup();
}

uint32_t lastReconnectAttempt = 0;
void WCEVO_manager::sta_loop(){

  /*
    network scanner enabled by WCEVO_CM_STA || WCEVO_CM_STAAP if reconnecting attemp is needed

    le debut du loop commence via le scan wifi si multiple credential

  */

  if (_scanNetwork_running) {
    ALT_TRACEC("main", "SCAN network\n");
    if (networkScan()->scan(3)){
      ALT_TRACEC("main", "NETWORKSCAN DONE -> Begin sta connection\n");
      networkScan()->list_sortByBestRSSI();
      networkScan()->list_print();  
      _credential = credential_getBestSSID();
      _scanNetwork_running = false;
      if (!_STACO.setup()) {
         sta_reconnect_end();
      }
    }
  } else {
    _STACO.get_lastReconnectAttempt(lastReconnectAttempt);
    if (lastReconnectAttempt == 0) {
      ALT_TRACEC("main", "NETWORKSCAN FALSE -> Begin sta connection\n");
      _STACO.set_reconnectAttempt(true);
      if (!_STACO.setup()) {
         sta_reconnect_end();
      }
    }
  }

  /*
    A FAIRE
    si deco alor reset du mod ap et sta
  */
  if (_STACO.get_wasConnected() && !isConnected() && _STACO.get_serverInitialized() ) { 
    Serial.println("DISCONNECTED !!!");
  }

  if ( !isConnected()  ) {
    
    if (_CONNECTMOD == wcevo_connectmod_t::WCEVO_CM_STA || _CONNECTMOD == wcevo_connectmod_t::WCEVO_CM_STAAP ) {

      _STACO.get_lastReconnectAttempt(lastReconnectAttempt);
      if (_STACO.get_reconnectAttempt() <= sta_getMaxAettemp() && !_APCO.get_active() && !_scanNetwork_running && (millis() - lastReconnectAttempt > 20000) ){
        sta_reconnect();
      } 

      _STACO.get_lastReconnectAttempt(lastReconnectAttempt);
      if (!_APCO.get_active() && !_scanNetwork_running && (millis() - lastReconnectAttempt > 20000) ){
        if (_CONNECTFAIL == wcevo_connectfail_t::WCEVO_CF_NEXTAP) {
          if (_STACO.get_reconnectAttempt() > sta_getMaxAettemp()-1 ) {
            if (_scanNetwork_gotSSID) {
              ALT_TRACEC("main", "WCEVO_CF_NEXTAP -> reconnect ? end\n")
              sta_reconnect_end();
            } else {
              ALT_TRACEC("main", "WCEVO_CF_NEXTAP -> reconnect ? next\n")
              sta_reconnect_end(false);
            }
          }   
        } else if (_CONNECTFAIL == wcevo_connectfail_t::WCEVO_CF_AP) {
          if (_STACO.get_reconnectAttempt() > sta_getMaxAettemp()-1 ) {
            ALT_TRACEC("main", "WCEVO_CF_AP -> reconnect\n")
            sta_reconnect_end();
          } 
        }  
      }    
    }
  } else if (!_STACO.get_serverInitialized()){

    yield();

    sta_reconnect_end(false);

    String duration;
    _STACO.get_lastReconnectAttempt(lastReconnectAttempt);
    al_tools::on_time_h((millis()-lastReconnectAttempt), duration);
    ALT_TRACEC("main", "\n\t>>> Connected in %s IP address:", duration.c_str());
    Serial.println(localIP());

    if (_CONNECTMOD == wcevo_connectmod_t::WCEVO_CM_STAAP){  
      if (!_APCO.get_active()) _APCO.setup(false, false, false);
    } 

    if (!_APCO.get_active()) mdns_setup();

    ota_setup(); 

    ALT_TRACEC("main", "Init STA interfaces\n");
    _webserver->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/plain", "Hello, world");
    }).setFilter(ON_STA_FILTER);  
    ALT_TRACEC("main", "webserverBegin\n");
    _webserver->begin();
    // if (_STAFUNC_INIT_SERVER!=nullptr) _STAFUNC_INIT_SERVER();
    
    _STACO.set_serverInitialized(true); 
    _STACO.set_wasConnected(true);

    delay(0);

    
  }
}
void WCEVO_manager::handleConnection(){
  if (_scanNetwork_requiered) {
    Serial.println("-tScan");
    if (networkScan()->scan(1)){
      networkScan()->list_sortByBestRSSI();
      networkScan()->scan_reset();  
      networkScan()->list_print();  
      _scanNetwork_requiered = false;
    }  
  }

  if (_STACO.get_active() || _APCO.get_active()){
    #if defined(ESP8266)
      MDNS.update();
    #endif  
    ArduinoOTA.handle();
  }

  if ((_CONNECTMOD == wcevo_connectmod_t::WCEVO_CM_STA ||_CONNECTMOD == wcevo_connectmod_t::WCEVO_CM_STAAP) && !_APCO.get_active()){
    sta_loop();
  }
  if (_CONNECTMOD == wcevo_connectmod_t::WCEVO_CM_AP && !_APCO.get_active() ) {
    _APCO.setup();
  }
  if (_APCO.get_active() ) {
    _dnsServer->processNextRequest();   
  }
  
}

void WCEVO_manager::ota_setup(){
  if (_otaSetup) return;

  ALT_TRACEC("main", "-\n");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf_P(PSTR("Progress: %u%%\n"), (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.setHostname(_server->get_hostName().c_str());
  ArduinoOTA.begin();

  _otaSetup = true;
}






/*
#if defined(ESP32)
void display_connected_devices()
{
  static byte stacO = 0;

    wifi_sta_list_t wifi_sta_list;
    tcpip_adapter_sta_list_t adapter_sta_list;
    esp_wifi_ap_get_sta_list(&wifi_sta_list);
    tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list);

    if (adapter_sta_list.num == stacO) return;
    stacO = adapter_sta_list.num;

    if (adapter_sta_list.num > 0)
        Serial.println("-----------");
    for (uint8_t i = 0; i < adapter_sta_list.num; i++)
    {
        tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];
        Serial.print((String)"[+] Device " + i + " | MAC : ");
        Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X", station.mac[0], station.mac[1], station.mac[2], station.mac[3], station.mac[4], station.mac[5]);
        Serial.println((String) " | IP " + ip4addr_ntoa(&(station.ip)));
    }
}
#elif defined(ESP8266)
void display_connected_devices()
{
  // https://stackoverflow.com/questions/42593385/get-mac-address-of-client-connected-with-esp8266
  static byte stacO = 0;

  unsigned char number_client = wifi_softap_get_station_num();
  
  if (number_client == stacO) return;
  stacO = number_client;

  
  
  
  Serial.print(" Total Connected Clients are = ");
  Serial.println(number_client);

    auto i = 1;
    struct station_info *station_list = wifi_softap_get_station_info();
    while (station_list != NULL) {
        auto station_ip = IPAddress((&station_list->ip)->addr).toString().c_str();
        char station_mac[18] = {0};
        sprintf(station_mac, "%02X:%02X:%02X:%02X:%02X:%02X", MAC2STR(station_list->bssid));
        Serial.printf("%d. %s %s\n", i++, station_ip, station_mac);
        station_list = STAILQ_NEXT(station_list, next);
    }
    wifi_softap_free_station_info();
}
#endif
*/