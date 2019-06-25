#include "NTPClient.h"
#include <ESP8266WiFi.h>

#include <Arduino.h>

const char* ntpServerName = "time.nist.gov";
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
unsigned long timeout = 10000;

// TODO - these should probably be members, although it should be fine if using only on NTPClient
IPAddress timeServerIP; // time.nist.gov NTP server address
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

void NTPClient::setup(){
    const unsigned int localPort = 2390;      // local port to listen for UDP packets
    _udp.begin(localPort);
}

void NTPClient::loop(){
    unsigned long timeSinceRequest = millis() - _tick_at_request;
    if (_epoch_at_check == 0 && timeSinceRequest >= timeout){
        //get a random server from the pool
        WiFi.hostByName(ntpServerName, timeServerIP);
        sendNTPpacket(timeServerIP);
        _tick_at_request = millis();
    } else if (_epoch_at_check == 0 && timeSinceRequest < timeout){
        int cb = _udp.parsePacket();
        if (!cb) {
            Serial.println("no packet yet");
        } else {
            // We've received a packet, read the data from it
            _udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

            //the timestamp starts at byte 40 of the received packet and is four bytes,
            // or two words, long. First, esxtract the two words:
            unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
            unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
            // combine the four bytes (two words) into a long integer
            // this is NTP time (seconds since Jan 1 1900):
            unsigned long secsSince1900 = highWord << 16 | lowWord;
            // now convert NTP time into everyday time:
            // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
            const unsigned long seventyYears = 2208988800UL;
            // subtract seventy years:
            _epoch_at_check = secsSince1900 - seventyYears;
            _tick_at_check = millis();
        }
    }
}

unsigned long NTPClient::getEpoch(){
    return _epoch_at_check + (millis() - _tick_at_check)/1000;
}

// send an NTP request to the time server at the given address
void NTPClient::sendNTPpacket(IPAddress& address) {
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;

    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    _udp.beginPacket(address, 123); //NTP requests are to port 123
    _udp.write(packetBuffer, NTP_PACKET_SIZE);
    _udp.endPacket();
}

