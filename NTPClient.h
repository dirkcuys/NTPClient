#ifndef NTPCLIENT_H
#define NTPCLIENT_H

#include <WiFiUdp.h>

class NTPClient {
    public:
        NTPClient(UDP & udp) : _udp(udp){};
        void setup();
        void loop();
        unsigned long getEpoch();
        bool timeSynched(){ return _epoch_at_check > 0; };

    private:
        void sendNTPpacket(IPAddress& address);

        UDP & _udp;
        unsigned long _tick_at_request = 0;
        unsigned long _epoch_at_check = 0;
        unsigned long _tick_at_check = 0;
        bool _outstanding_packet = false;

};
#endif
