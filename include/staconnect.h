#ifndef _STACONNECT_H
  #define _STACONNECT_H
  #include "def.h"
  #include "credential.h"
  /*
  class WCEVO_esp8266STA : public ESP8266WiFiSTAClass
  {
  public:
    WCEVO_esp8266STA() {};
    ~WCEVO_esp8266STA() {};
    void sav_persistent(const char* ssid, const char *passphrase, const uint8_t* bssid){
      if(!WiFi.enableSTA(true)) {
          // enable STA failed
          Serial.println("----> enable STA failed");
          return ;
      }

      if(!ssid || *ssid == 0x00 || strlen(ssid) > 32) {
          // fail SSID too long or missing!
          Serial.println("----> fail SSID too long or missing!");
          return ;
      }

      int passphraseLen = passphrase == nullptr ? 0 : strlen(passphrase);
      if(passphraseLen > 64) {
          // fail passphrase too long!
          Serial.println("----> fail passphrase too long!");
          return ;
      }

      struct station_config conf;
      conf.threshold.authmode = (passphraseLen == 0) ? AUTH_OPEN : (_useInsecureWEP ? AUTH_WEP : AUTH_WPA_PSK);

      if(strlen(ssid) == 32)
          memcpy(reinterpret_cast<char*>(conf.ssid), ssid, 32); //copied in without null term
      else
          strcpy(reinterpret_cast<char*>(conf.ssid), ssid);

      if(passphrase) {
        if (passphraseLen == 64) // it's not a passphrase, is the PSK, which is copied into conf.password without null term
          memcpy(reinterpret_cast<char*>(conf.password), passphrase, 64);
        else
          strcpy(reinterpret_cast<char*>(conf.password), passphrase);
      } else {
        *conf.password = 0;
      }

      conf.threshold.rssi = -127;
      #ifdef NONOSDK3V0
        conf.open_and_wep_mode_disable = !(_useInsecureWEP || *conf.password == 0);
      #endif

      if(bssid) {
        conf.bssid_set = 1;
        memcpy((void *) &conf.bssid[0], (void *) bssid, 6);
      } else {
        conf.bssid_set = 0;
      }

      ETS_UART_INTR_DISABLE();
      wifi_station_set_config(&conf);
      Serial.println("----> wifi_station_set_config");
      ETS_UART_INTR_ENABLE();
    }
    
  };  
  */
  class WCEVO_STAconnect 
  {
      
    WCEVO_credential  * _credential   = nullptr;
    WCEVO_server      * _server       = nullptr;

    uint32_t  _lastReconnectAttempt  = 0;
    uint8_t   _reconnectAttempt      = 0;
    boolean   _active                = false;
    boolean   _serverInitialized     = 0;   
    boolean   _wasConnected           = false;


    void loop();
    void get_reconnectAttempt(uint8_t &);
    void set_active(boolean);
  public:
    // WCEVO_esp8266STA _esp8266STA;

    WCEVO_STAconnect() {};
    ~WCEVO_STAconnect(){};


    void init(WCEVO_server * server);
    boolean setup();
    void reset();
    void get_lastReconnectAttempt(uint32_t &);
    uint8_t get_reconnectAttempt();
    boolean get_active();
    boolean get_serverInitialized();
    boolean get_wasConnected();
    void set_wasConnected(boolean);
    void set_serverInitialized(boolean);
    void set_reconnectAttempt(boolean);
    void set_lastReconnectAttempt();

  };

#endif // STACONNECT_H