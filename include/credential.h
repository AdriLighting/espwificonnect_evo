#ifndef _CREDENTIAL_H
#define _CREDENTIAL_H
  #include "def.h"

  class WCEVO_server
  {
      char * _ota_psk   = nullptr;
      char * _ap_ssid   = nullptr;
      char * _ap_psk    = nullptr;
      char * _hostname  = nullptr;
      char * ApSSIDInit(const char * const & str);
  public:
      ~WCEVO_server();
      WCEVO_server( const char * const &, const char * const & , const char * const & );  
      WCEVO_server( WCEVO_server* ptr );  
      WCEVO_server();
      String get_hostName();
      void get_hostName(const char *& result);
      void get_apSSID(const char *& result);
      void get_apPsk(const char *& result); 
      void get_otaPsk(const char *& result);  
      void print();
  };
  class WCEVO_credential
  {
    char    * _sta_ssid     = nullptr;
    char    * _sta_ssidPsk  = nullptr;
    uint8_t _order          = 0;
    boolean _tested         = false;
  public:
    ~WCEVO_credential();
    WCEVO_credential( const char * const &, const char * const &  );  
    void get_ssid(const char *& result);
    String get_ssid();
    String get_psk();
    boolean get_tested();
    void get_psk(const char *& result); 
    void set_tested(boolean);
    void print();
  };


#endif // CREDENTIAL_H