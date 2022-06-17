#ifndef _WCEVO_SCAN_H
#define _WCEVO_SCAN_H

  #include "def.h"
  
  class WCEVO_wifiscanItem
  {
    String    _ssid     ;
    int32_t   _rssi     ;
    String    _bssidstr ;
    int32_t   _channel;
    uint8_t   _encryptionType;
    String    _encryptionTypeStr;
    boolean   _isHidden;
    uint8_t * _mac      = nullptr;

  public:

    ~WCEVO_wifiscanItem(){if (_mac) delete _mac;};

    WCEVO_wifiscanItem(uint8_t);   
    WCEVO_wifiscanItem(WCEVO_wifiscanItem *);

    void get_bssid(String &);
    void get_ssid(String &);
    void get_rssi(int32_t &);
    void get_encryptionType(uint8_t &);
    void get_chanel(int32_t &);
    static uint8_t * get_mac(WCEVO_wifiscanItem*);

    void update(WCEVO_wifiscanItem *);
        
    void print(uint8_t);
  };


  class WCEVO_scanNetwork
  {
    LList<WCEVO_wifiscanItem *>   _wifiScan;
    uint8_t                       _scanCnt = 0;
    boolean                       _scanComplete = false;
    unsigned int                  _minimumQuality = 0;
    boolean                       _scanStop = false;

    void additem(WCEVO_wifiscanItem * ptr);
    
    WCEVO_wifiscanItem * cmp_mac(WCEVO_wifiscanItem * ptr);   
    WCEVO_wifiscanItem * cmp_ssid(const char * const &);   

    int scan();

    WCEVO_wifiscanItem * get_bestSSID(String *, uint8_t size);    
    boolean get_knowSSID(uint8_t pos);


  public:
    WCEVO_scanNetwork(){};
    ~WCEVO_scanNetwork(){};

    boolean scan(uint8_t);
    void    scan_reset();
    void    scan_stop();

    void get_SSID(String search, uint8_t & listSize, int8_t & pos);  
    int get_bestSSID();  
    void list_sortByBestRSSI();
    void list_print();  

    unsigned int getRSSIasQuality(int RSSI);
    String networkListAsString(boolean);
    void networkListAsJson(uint8_t,JsonArray &);
    void list_clear();
    
  };

#endif // _WCEVO_SCAN_H