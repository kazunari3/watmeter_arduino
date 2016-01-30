#include <Wire.h>
#include <RTC8564.h>

/*

 Udp NTP Client
 
 Get the time from a Network Time Protocol (NTP) time server
 Demonstrates use of UDP sendPacket and ReceivePacket 
 For more on NTP time servers and the messages needed to communicate with them, 
 see http://en.wikipedia.org/wiki/Network_Time_Protocol
 
 created 4 Sep 2010 
 by Michael Margolis
 modified 9 Apr 2012
 by Tom Igoe
 
 This code is in the public domain.
 
 */
#include <Time.h> 
#include <SPI.h>         
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <Dns.h>

#define CT ((330.0 * 0.99 / 3000.0) / (5.0 / 1024.0))

//#define DEBUG

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {  
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE };

unsigned int localPort = 8888;      // local port to listen for UDP packets
unsigned int customPort = 8192;

//IPAddress timeServer(192, 43, 244, 18); // time.nist.gov NTP server
IPAddress timeServer(133,243,238,163);
const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets 

// A UDP instance to let us send and receive packets over UDP
EthernetUDP UdpNtp;
EthernetUDP UdpCustom;

time_t prevDisplay = 0; // when the digital clock was displayed
time_t prevSecond = 0;
const  long timeZoneOffset = (32400); // set this to the offset in seconds to your local time;

EthernetServer server(80);

#ifdef DEBUG
IPAddress gae(10,0,0,1); // linux
#else
char gae[] = "http://inukura01.appspot.com";
#endif

EthernetClient webclient;

void setup() 
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.println("Boot start!");
  
  bool flg = Rtc.begin();
  Serial.print(flg);
  
  
  // start Ethernet and UDP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for(;;)
      ;
  }
  // print your local IP address:
  Serial.print("My IP address: ");
  Serial.println(ip_to_str(Ethernet.localIP()));
  Serial.print("Gateway IP address is ");
  Serial.println(ip_to_str( Ethernet.gatewayIP()));

  Serial.print("DNS IP address is ");
  Serial.println(ip_to_str( Ethernet.dnsServerIP()));

  UdpNtp.begin(localPort);
  UdpCustom.begin(customPort);
#ifdef DEBUG
#else
  unsigned long t = getNtpTime();
  Rtc.set(t);
  Serial.println(t);
  setSyncProvider(Rtc.get);
  while(timeStatus()== timeNotSet)
    ; // wait until the time is set by the sync provider  
#endif
 wdt_enable (WDTO_4S); // これは効いていないみたい
}

void loop()
{
  if( second()/10 != prevSecond) {
    if( minute() != prevDisplay) //update the display only if the time has changed
    {
      prevDisplay = minute();
      digitalClockDisplay();
    
      //POST
      static char buf[100];
      static char df[100];
      static char header[60];
  
            float A1, A2, SUM;
  
            A1 = (float)analogRead(1) / CT;
            A2 = (float)analogRead(2) / CT;
            SUM = A1+ A2;
      dtostrf((double)SUM,-1,1,df);
      sprintf(buf, "json={\"date\":%ld, \"value\":%s}\0", now(), df);
      Serial.println(buf);
      sprintf(header, "Content-Length: %d", strlen(buf)); //POST隴弱ｅ竊楢��ｽ�ｽ
      while (!webclient.connected()) {
  #ifdef DEBUG
        if (webclient.connect(gae, 8080)) {
  #else
        //if (webclient.connect(gae, 80)) {
        webclient.connect(gae, 80);
  #endif
        }
          Serial.println("gae connected");
          // Make a HTTP request:
          webclient.println("POST /sign HTTP/1.1");
  
  #if 1
          webclient.println("Host: inukura01.appspot.com");
          webclient.println("Connection: keep-alive");
          webclient.println(header);
          webclient.println("Cache-Control: max-age=0");
          webclient.println("Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
          webclient.println("Accept-Language: en-US,en;q=0.8");
  #endif        
          webclient.println();
          webclient.println(buf);
          webclient.println();
          char c;
          while(webclient.available())
          {
            c = webclient.read();
            Serial.print(c);
          }
      webclient.stop(); //邵ｺ阮呻ｼ�ｸｺ�ｧstop邵ｺ蜉ｱ窶ｻ郢ｧ蛹ｻ�樒ｸｺ蜿･�ｽ髢�ｿｽ�ｽ闖ｴ蜷晄�邵ｺ繧�ｽ�    } else {
      // 郢晢ｿｽﾎ醍ｹ晢ｽｼ邵ｺ�ｮGETT陷�ｽｦ騾�ｿｽ      prevSecond = second()/10;
      
      while (!webclient.connected()) {
  #ifdef DEBUG
        if (webclient.connect(gae, 8080)) {
  #else
        //if (webclient.connect(gae, 80)) {
        webclient.connect(gae, 80);
  #endif
        }
          Serial.println("gae get connected");
          // Make a HTTP request:
          webclient.println("GET / HTTP/1.0");
  #if 1
          webclient.println("Host: inukura01.appspot.com");
          webclient.println("Connection: keep-alive");
          //webclient.println(header);
          webclient.println("Cache-Control: max-age=0");
          //webclient.println("Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
          webclient.println("Accept-Language: en-US,en;q=0.8");
  #endif            
          webclient.println();
          char c;
          while(webclient.available())
          {
            c = webclient.read();
            Serial.print(c);
          }

      webclient.stop(); //邵ｺ阮呻ｼ�ｸｺ�ｧstop邵ｺ蜉ｱ窶ｻ郢ｧ蛹ｻ�樒ｸｺ蜿･�ｽ髢�ｿｽ�ｽ闖ｴ蜷晄�邵ｺ繧�ｽ�     
    }
  }
  if ( UdpCustom.parsePacket() ) {
    Serial.println(UdpCustom.remoteIP());
    Serial.println(UdpCustom.remotePort());
    sendMyUdpPacket(UdpCustom.remoteIP());

  }
  // wait ten seconds before asking for the time again
  //delay(10000); 
  wdt_reset ();
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

char* getTimeString(){
  static char buf[16];
  sprintf(buf, "%02d:%02d:%02d\0", hour(), minute(), second());
  return buf;  
}

unsigned long getNtpTime()
{
  IPAddress ntpAddr;
  DNSClient dns;
  dns.begin(Ethernet.dnsServerIP());
  //char ntp[] = "http://inukura01.appspot.com";
  int  ret = dns.getHostByName("ntp.nict.jp", ntpAddr);

  if (ret == 1) {
    
  } else {
    Serial.println("No NTP Server");
  }
  //sendNTPpacket(timeServer); // send an NTP packet to a time server
  sendNTPpacket(ntpAddr);
  delay(1000);
  if ( UdpNtp.parsePacket() ) {
    if(UdpNtp.remotePort() == 123) {
      Serial.println("Sync!!");
      Serial.print("NTP Remoto port = ");    
      Serial.println(UdpNtp.remotePort());  
      Serial.print("NTP Remot IP = ");  
      Serial.println(UdpNtp.remoteIP());
      // We've received a packet, read the data from it
      UdpNtp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer

      //the timestamp starts at byte 40 of the received packet and is four bytes,
      // or two words, long. First, esxtract the two words:

      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
      // combine the four bytes (two words) into a long integer
      // this is NTP time (seconds since Jan 1 1900):
      unsigned long secsSince1900 = highWord << 16 | lowWord;  
      Serial.print("Seconds since Jan 1 1900 = " );
      Serial.println(secsSince1900);               

      // now convert NTP time into everyday time:
      Serial.print("Unix time = ");
      // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
      const unsigned long seventyYears = 2208988800UL;     
      // subtract seventy years:
      unsigned long epoch = secsSince1900 - seventyYears   + timeZoneOffset;  
      // print Unix time:
      Serial.println(epoch);                               

      return epoch;
    }
  }
  return 0;
}

void sendMyUdpPacket(IPAddress address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, UDP_TX_PACKET_MAX_SIZE); 
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = Ethernet.localIP()[0];   // LI, Version, Mode
  packetBuffer[1] = Ethernet.localIP()[1];     // Stratum, or type of clock
  packetBuffer[2] = Ethernet.localIP()[2];     // Polling Interval
  packetBuffer[3] = Ethernet.localIP()[3];  // Peer Clock Precision

  UdpCustom.beginPacket(address, UdpCustom.remotePort());
  UdpCustom.write(packetBuffer,UDP_TX_PACKET_MAX_SIZE);
  UdpCustom.endPacket();   
}



// send an NTP request to the time server at the given address 
unsigned long sendNTPpacket(IPAddress& address)
{
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
  UdpNtp.beginPacket(address, 123); //NTP requests are to port 123
  UdpNtp.write(packetBuffer,NTP_PACKET_SIZE);
  UdpNtp.endPacket(); 
}

// Just a utility function to nicely format an IP address.
const char* ip_to_str(IPAddress ipAddr)
{
  static char buf[16];
  sprintf(buf, "%d.%d.%d.%d\0", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
  return buf;
}

//////////////////////////////////////////////////////////
// 8564NB













