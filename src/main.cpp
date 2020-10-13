
// ----Juergens Breakout Board mit M8N
/*
 #define RXD2 16
 #define TXD2 17
 #define S2BAUD 9600
 #define LED 2
//*/

// -----DLG Board mit F9P
#define RXD2 13
#define TXD2 5
#define S2BAUD 115200
// Achtung hier ist TXD auf der internen LED  GPIO-05 !!!
#define LED 2 // --> macht nichts (da LED auf GPIO-05)

#include <WiFi.h>
#include <WiFiAP.h>

// Static IP für connect to Router
IPAddress local_IP(192, 168, 2, 26);
// Set your Gateway IP address
IPAddress gateway(192, 168, 2, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 2, 1);   //optional

// ESP als eigener AccessPoint
// Set these to your desired credentials.
IPAddress AP_local_IP(192,168,4,1);
IPAddress AP_local_gateway(192,168,4,1);
IPAddress AP_local_subnet(255,255,255,0);


uint8_t buffer_ser2tcp[1436];
uint8_t buffer_tcp2ser[1436];
uint8_t i;


WiFiServer server(80);

/* Verbindung zum Router Messnetz oder SMC4 herstellen */
/* --------------------------------------------------- */
uint TryToConnect() 
{
   uint8_t i_timeout = 0;
   while ( (WiFi.status() != WL_CONNECTED) and (WiFi.status() != WL_CONNECT_FAILED) and (i_timeout < 20) )
    {
      i_timeout++;
      Serial.println(".");
      //digitalWrite(LED, HIGH);
      delay(200);
      //digitalWrite(LED, LOW);
      delay(200);
      Serial.print("WiFi.status=");
      Serial.println((uint) WiFi.status());
    }
  return (uint) WiFi.status();
}

void StartAccessPointMode()
{
  //Serial.print("Setting soft-AP configuration ... ");
  //Serial.print(WiFi.softAPConfig()
  //Serial.println(WiFi.softAPConfig(AP_local_IP, AP_local_gateway, AP_local_subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP("GNSS") ? "Ready" : "Failed!");

  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
}

void StartRouterConnect()
{
   // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS))
  {
    Serial.println("STA Failed to configure");
  }
    
  char ssid1[] = "MessNetz";   // your network SSID (name)
  char pass1[] = "An2HFVUj";   // your network password

  char ssid2[] = "SMC4";      // your network SSID (name)
  char pass2[] = "";   // your network password

  Serial.println("");
  delay(1000);
  bool bConnected = false;

  // Versuche Verbindung zu DLG-Netz
  Serial.print("...try to Connecting to: ");
  Serial.println(ssid1);
  WiFi.mode(WIFI_STA);
  delay(300);
  WiFi.begin(ssid1, pass1);
  delay(300);
  if (TryToConnect() == WL_CONNECTED)
  {
      bConnected = true;
  }

    // Versuche Verbindung zu Heimnetz  
  if (!bConnected)
  {
    WiFi.disconnect();
    delay(500);
    WiFi.mode(WIFI_OFF);
    delay(100);
    WiFi.mode(WIFI_STA);
    
    Serial.print("...try to Connecting to: ");
    Serial.println(ssid2);
    WiFi.begin(ssid2, pass2);
    if (TryToConnect() == WL_CONNECTED)
    {
       bConnected = true; 
    }
  }
  
  if (!bConnected)
  {
    Serial.println("no WIFI-CONNECTION --> RESET !!!");
    delay(500);
    ESP.restart();
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  WiFi.setSleep(false);   
}

void setup() 
{
 // Set pin mode
  //pinMode(LED,OUTPUT);
  //digitalWrite(LED,LOW);
  Serial.begin(115200);
  delay(1000);

  // ---- Router MESSNETZ oder SMC4 (192.168.2.26) ---
  // StartRouterConnect();
  
  // --- Eigener AP                 (192.168.4.1) ---
  StartAccessPointMode();

  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  WiFi.setSleep(false);   

  // Print local IP address and start web server
  Serial.println("");
  Serial.println("--------- WiFi connected -------------");
  
  delay(200);
  server.begin();
  delay(200);
  //digitalWrite(LED,HIGH);
  Serial2.setRxBufferSize(400);
  Serial2.begin(S2BAUD, SERIAL_8N1, RXD2, TXD2);
}

void loop() 
{
  // listen for incoming clients
  if (server.hasClient()) 
  {
    
    WiFiClient client = server.available();
    //client.setNoDelay(true); // by JG
    Serial.print("* client connected...");
    
    while(Serial2.available())
    {
      char c = Serial2.read();
      if (c== 0x0A)
      {
        break;
      }
      yield();
    }
    
    //digitalWrite(LED,HIGH);

    while (client.connected()) 
    {
      if(Serial2.available())
      {
        delay(1);
        int cnt_ser2tcp = Serial2.readBytesUntil(0x0A,buffer_ser2tcp, sizeof(buffer_ser2tcp));
       
        buffer_ser2tcp[cnt_ser2tcp] = 0x0A;
        client.write(buffer_ser2tcp, cnt_ser2tcp+1);
        

        // nur zum debuggen (evt. wieder auskommentieren)
        //Serial.write(buffer_ser2tcp,cnt_ser2tcp+1);
      }
      
      if(client.available())
      {
          Serial2.write(client.read());
         
          //char c = client.read();
          //Serial2.write(c);
          //Serial.print(c);
      }
      yield();
    } // ... client.Connected()

    delay(300);
    //close the connection:
    //client.stop();
    //Serial2.flush();
    //digitalWrite(LED,LOW);
    Serial.println("client disonnected");

  } //... server.hasClient()

}
