
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Thread.h>
#include <ThreadController.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <PubSubClient.h>
#include "Eeprom.cpp"

//const char* ssid = "WiFi-Arnet-2jdz";
//const char* password = "rxire3p8";


char ssid[50];
char password[50];
char ssidAP[50];
char passwordAP[50];
//const char* poolServerName = "time.nist.gov";
const char* poolServerName = "time.nist.gov";
//const char* poolServerName = "pool.ntp.org";
int ZonaHoraria = -3;
int zh = (ZonaHoraria* 3600);


char   SERVER[50]   = "54.227.205.125"; //"m15.cloudmqtt.com"
int    SERVERPORT   = 18129;
char USERNAME[50] = "jnaokrrg";
char PASSWORD[50] = "aMHg3lfP3r6i";   


/*  
char   SERVER[50]   = "mqttcontrol.ddns.net"; //"m11.cloudmqtt.com"
int    SERVERPORT   = 1883;
char USERNAME[50] = "MOfDA813";
char PASSWORD[50] = "123456789";   
*/
 

char KEPT_ALIVE[50];
char SUB_TOPIC[50];

int contconexion = 0;
unsigned long previousMillis = 0;



WiFiClient espClient;
PubSubClient client(espClient);

WiFiServer server(80);
WiFiUDP ntpUDP;
Thread ActualizacionEstado = Thread();
Thread LeerSensores = Thread();
Thread Mqtt = Thread();
NTPClient timeClient(ntpUDP, poolServerName, zh);

  
String htmlWeb;

class Termotanque
{
 private:
          float estadoTemperatura = 99.1;
          int estadoRele = 2;
          int estadoConexion;
          byte estadoSistema;
          byte tempObj;
          byte estadoSuperumbrales;
          byte estadoUmbral1;
          byte tempObjUmbral1;
          byte horaMinimaUmbral1;
          byte horaMaximaUmbral1;
          byte estadoUmbral2;
          byte tempObjUmbral2;
          byte horaMinimaUmbral2;
          byte horaMaximaUmbral2;
          byte estadoContrasena;
          bool mqttState = false;
 public:

        bool getMqttState()
        {
          return mqttState; 
        }
        void setMqttStare(bool e)
        {
          mqttState = e;
        }
        void GenerarJson()
        {

          String hora = String(hour()) + ":" + String(minute()) + ":" + String(second());
          
         StaticJsonBuffer<400> jsonBuffer;
         JsonObject& root = jsonBuffer.createObject();
         root["Dispositivo"] = 1;
         root["Nombre"] = ssidAP;
         root["localIP"] = WiFi.localIP().toString();
         root["estadoConexion"] = WiFi.status();
         root["estadoTemperatura"] = estadoTemperatura;
         root["estadoSist"] = estadoSistema;
         root["tempObj"] = tempObj;
         root["estadoSupUmbrales"] = estadoSuperumbrales;
         root["estadoUmbral1"] =  estadoUmbral1;
         root["tempObjUmbral1"] = tempObjUmbral1;
         root["HoraMinimaUmbral1"] = horaMinimaUmbral1;
         root["HoraMaximaUmbral1"] = horaMaximaUmbral1;
         root["estadoUmbral2"] =  estadoUmbral2;
         root["tempObjUmbral2"] = tempObjUmbral2;
         root["HoraMinimaUmbral2"] = horaMinimaUmbral2;
         root["HoraMaximaUmbral2"] = horaMaximaUmbral2;
         root["hora"] = hora;
         root["estCalefactor"] = estadoRele; 
         root["id"] = ESP.getChipId();
         root["ZonaHoraria"] = ZonaHoraria;
         root["estPassChang"] = estadoContrasena;
            String output;
          root.printTo(output);
         htmlWeb = output;
        }

        void actualizarVariables()
        {
         EEPROM.begin(500);
        estadoSistema = EEPROM.read(250);
        tempObj = EEPROM.read(251);          
        estadoSuperumbrales = EEPROM.read(252);
        estadoUmbral1 = EEPROM.read(253);
        tempObjUmbral1 = EEPROM.read(254);
        horaMinimaUmbral1 = EEPROM.read(255);
        horaMaximaUmbral1 = EEPROM.read(256);
        estadoUmbral2 = EEPROM.read(257);  //
        tempObjUmbral2 = EEPROM.read(258); //
        horaMinimaUmbral2 = EEPROM.read(259); //
        horaMaximaUmbral2 = EEPROM.read(260); //
        estadoContrasena = EEPROM.read(262);
        //ZonaHoraria = EEPROM.read(261);
        EEPROM.end();
        }
        
        void actualizarEstado()
        {
          
          int hora = hour();
          int estadoReleActual = 2;
          //chek super umbrales
          if(estadoSuperumbrales == 1 && WiFi.status() == WL_CONNECTED)
          {
          
              //chek umbral 1 
              if(estadoUmbral1 == 1 && hora >= horaMinimaUmbral1 && hora <= horaMaximaUmbral1 && estadoTemperatura < tempObjUmbral1)
              {
                 estadoReleActual = 1;
              }
              
              //chek umbral 2
              if(estadoUmbral2 == 1 && hora >= horaMinimaUmbral2 && hora <= horaMaximaUmbral2 && estadoTemperatura < tempObjUmbral2)
              {
                 estadoReleActual = 1;
              }
          }
          
          //chek sistema
          if(estadoSistema == 1 && estadoTemperatura < tempObj)
          {
            estadoReleActual = 1;
          }
          
          if(estadoTemperatura < 80 && estadoTemperatura > 0) // check if the temperature is in the correct rangue.
          {
          estadoRele = estadoReleActual;
          }else {estadoRele = 2;}
          estadoConexion = WiFi.status();

        }

        
        void setTemp(float temp)
        {
          estadoTemperatura = temp;
        }
        
        void SetEstado()
        {
          String estRele = String(estadoRele);
          String estConn = String(estadoConexion);
          String response = "{\"abc\":\"" + estRele + "\",\"estConnection\":\"" + estConn + "\"}";
        
          Serial.print(response);
        }
        
        float getTemp()
        {
          return(estadoTemperatura);
        }
        
        int setEstcal()
        {
          return estadoRele;
        }
};

Termotanque T;

Eeprom E;


class Web
{

  private:
         String htmlWeb;

  public:
          void ConectarWifi()
          { 
            WiFi.begin(ssid, password);
            IPAddress ip(192,168,1,200);   
            IPAddress gateway(192,168,1,1);   
            IPAddress subnet(255,255,255,0);   
            //WiFi.config(ip, gateway, subnet);
            delay(5000);
               for(int cont = 0; cont < 3 ; cont++)
               {
                  if(WiFi.status() != WL_CONNECTED)
                  {
                    Serial.println("No pudo conectar");
                    WiFi.begin(ssid, password);
                    delay(2000);
                  }
                 if(WiFi.status() == WL_CONNECTED)
                 {
                    cont = 3;
                  }
               }
              
                if(WiFi.status() == WL_CONNECTED)
                   {
                    
                    Serial.println("se pudo conectar a: ssid");
                    Serial.println(WiFi.localIP());
                    Serial.println(WiFi.status());
                    WiFi.softAPdisconnect (true); 
                          
                   }else{
                    
                      WiFi.mode(WIFI_AP);
                      WiFi.softAP(ssidAP, passwordAP);
                      Serial.print("se inicio el AP en: ");
                      Serial.println(WiFi.softAPIP());
                    }
              
                    Serial.println("Red House: ");
                    Serial.println(WiFi.localIP());
                    Serial.println(WiFi.status());
              
                 
                    Serial.println("Red AP: ");
                    Serial.println(WiFi.softAPIP());
         }


       void web(String crudo)
         {
          String msj2 = "";  
          String msj = "";
          int cont = 0;
          for(int x = 0; x < (crudo.length()); x++)
            {
              if(cont == 1){
              msj = msj + crudo[x];
              }
              if(crudo[x] == '/'){
                cont ++;
              }
              
            }
            
            for(int x = 0; x < (msj.length()-6); x++)
            {
              //msj2 = msj2 + msj[x];
              switch(msj[x]){
                case '[': msj2 = msj2 + '{'; break;
                case ']': msj2 = msj2  + '}'; break;
                case '%': msj2 = msj2  + ' '; break;
                case '\'': msj2 = msj2  + '"'; break;
                default: msj2 = msj2  + msj[x];
               }
              
            }
            Serial.println(msj2);
            ParseJson(msj2);
          }

        void ParseJson(String json)
         {
  
           StaticJsonBuffer<400> jsonBuffer;
            JsonObject& root = jsonBuffer.parseObject(json);
            String ssid = root["ssid"];
            String pass = root["pass"];
            String APssid = root["APssid"];
            String APpass = root["APpass"];
          
            int estadoSistema = root["estadoSist"];
            int tempObj = root["tempObj"];
          
            int estadoSupUmbrales = root["estadoSupUmbrales"];
            
            int estadoUmbral1 = root["estadoUmbral1"];
            int tempObjUmbral1 = root["tempObjUmbral1"];
            int HoraMinimaUmbral1 = root["HoraMinimaUmbral1"];
            int HoraMaximaUmbral1 = root["HoraMaximaUmbral1"];
          
            int estadoUmbral2 = root["estadoUmbral2"];
            char tempObjUmbral2 = root["tempObjUmbral2"];
            int HoraMinimaUmbral2 = root["HoraMinimaUmbral2"];
            int HoraMaximaUmbral2 = root["HoraMaximaUmbral2"];
            int res = root["123"];
            int ZonaHoraria = root["ZonaHoraria"]; 
          if(APpass != "")
          {
            E.grabar(150, APpass);
            Serial.println(E.leer(150));
            EEPROM.begin(500); EEPROM.write(262, 1); EEPROM.end();
          }
          if(APssid != "")
          {
            E.grabar(100, APssid);
            Serial.println(E.leer(100));
          }
          if(pass != "")
          {
            E.grabar(50, pass);
            Serial.println(E.leer(50));
          }
          if(ssid != "")
          {
           E.grabar(0, ssid);
            Serial.print(E.leer(0)); 
          }

          
          if(estadoSistema != 0){EEPROM.begin(500); EEPROM.write(250, estadoSistema); EEPROM.end();}
          if(tempObj > 20 && tempObj < 80){EEPROM.begin(500); EEPROM.write(251, tempObj); EEPROM.end();}
          if(estadoSupUmbrales != 0){EEPROM.begin(500); EEPROM.write(252, estadoSupUmbrales); EEPROM.end();}
          if(estadoUmbral1 != 0){EEPROM.begin(500); EEPROM.write(253, estadoUmbral1); EEPROM.end();}
          if(tempObjUmbral1 > 20 && tempObjUmbral1 < 80){EEPROM.begin(500); EEPROM.write(254, tempObjUmbral1); EEPROM.end();}
          if(HoraMinimaUmbral1 <= 24 && HoraMinimaUmbral1 >= 1){EEPROM.begin(500); EEPROM.write(255, HoraMinimaUmbral1); EEPROM.end();}
          if(HoraMaximaUmbral1 <= 24 && HoraMaximaUmbral1 >= 1){EEPROM.begin(500); EEPROM.write(256, HoraMaximaUmbral1); EEPROM.end();}
          if(estadoUmbral2 != 0){EEPROM.begin(500); EEPROM.write(257, estadoUmbral2); EEPROM.end();}
          if(tempObjUmbral2 > 20 && tempObjUmbral2 < 80){EEPROM.begin(500); EEPROM.write(258, tempObjUmbral2); EEPROM.end(); }
          if(HoraMinimaUmbral2 <= 24 && HoraMinimaUmbral2 >= 1){EEPROM.begin(500); EEPROM.write(259, HoraMinimaUmbral2); EEPROM.end();}
          if(HoraMaximaUmbral2 <= 24 && HoraMaximaUmbral2 >= 1){EEPROM.begin(500); EEPROM.write(260, HoraMaximaUmbral2); EEPROM.end();}
          if(res == 123){ConectarWifi();}
          if(ZonaHoraria != 0 ){EEPROM.begin(500); EEPROM.write(261, ZonaHoraria); EEPROM.end();}
          T.actualizarVariables();
}


void GenerarJsonWeb()
{

  
}


void Html(String html)
{
   htmlWeb = html;
}

String GetHtml()
{
   return htmlWeb;
}

};

Web W;


      void callback(char* topic, byte* payload, unsigned int length) 
      {
      
        String res = "";
        Serial.print("Mensaje Recibido: [");
        Serial.print(topic);
        Serial.print("] ");
        int cont = 0;
        for (int i = 0; i < length; i++) {
          char c = (char)payload[i];
          if(c == '{' ){cont++;}
          if(cont == 1){
          res = res + c;
          }
          if(c == '}'){cont++;}
        }
        
        
        Serial.println(res);
        W.ParseJson(res);
        
      }

      void reconnect() 
      {
        
        // Loop hasta que estamos conectados
        
          Serial.print("Intentando conexion MQTT...");
          // Crea un ID de cliente al azar
          String clientId = "ESP8266Client-";
          clientId += String(random(0xffff), HEX);
          // Attempt to connect
      
          
        int conection =  client.connect("", USERNAME, PASSWORD);
          
          
          if (conection) 
          {
            Serial.println("conectado");
            client.subscribe(SUB_TOPIC);
          } else {
            Serial.print("fallo, rc=");
            Serial.print(client.state());
            // espera 5 segundos antes de reintentar
            //delay(500000);
          }
         
        
      }


void RefreshCredentials() // this function reload the credentials.
{
  E.leer(0).toCharArray(ssid, 50);
  E.leer(50).toCharArray(password, 50);
  E.leer(100).toCharArray(ssidAP, 50);
  E.leer(150).toCharArray(passwordAP, 50);
  
}

void setup() {
  
 
  Serial.begin(4800);
   RefreshCredentials();
 // E.leer(0).toCharArray(ssid, 50);
 // E.leer(50).toCharArray(password, 50);
  //E.leer(100).toCharArray(ssidAP, 50);
  //E.leer(150).toCharArray(passwordAP, 50);
 String ID = String(ESP.getChipId());
String sub_topic = "TT/OUT/" + ID;
   String KeptAlive = "TT/IN/" + ID; 
  KeptAlive.toCharArray(KEPT_ALIVE, 50);
  sub_topic.toCharArray(SUB_TOPIC, 50);

  //delay(10000);
  server.begin();
 W.ConectarWifi();
   
  client.setServer(SERVER, SERVERPORT);
  client.setCallback(callback);

  ActualizacionEstado.onRun(actualizacionEstado);
  LeerSensores.onRun(leerSensores);
  Mqtt.onRun(mqttTrhead);
  timeClient.begin();
  T.actualizarEstado();
 
T.actualizarVariables(); 
timeClient.begin();
if(WiFi.status() == WL_CONNECTED)
{
reconnect();  
if (client.connected())
{
  T.setMqttStare(true);
}
}


 }

void loop() {

   Mqtt.run();   
   ActualizacionEstado.run();
   LeerSensores.run();
 WiFiClient client = server.available();
  
  if (client)
  {
    while (client.connected())
      {
        if (client.available())
          {
            String line = client.readStringUntil('\r');
            if(line[0] == 'G' || line[0] == 'P')
            {
              W.web(line);

              /*
              client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println(""); //  do not forget this one
    client.println("<!DOCTYPE HTML>");
    client.println("<html>"); 
    client.print(W.GetHtml()); 
    client.println("</html>");
          //client.println("dwwqvew dw evvewve");

  */
  
            client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json");
          client.println("Server: Arduino");
          client.println("Connection: keep-alive");
          client.println();
          client.println(htmlWeb);
          client.println();

  
    
       //  client.println("wdwfwqfwq w ewe");
    
    delay(1);
             break;
            }
           }
       }
  }
 
}

void actualizacionEstado()
{
  T.GenerarJson();
    setTime(timeClient.getEpochTime());
  
  T.actualizarEstado();
  if(WiFi.status() == WL_CONNECTION_LOST)
  {
     W.ConectarWifi();
  }
  delay(250);
 
  timeClient.update();
  
}


void leerSensores()
{
  if(Serial.available())
  {
    
  String input = "";
  while(Serial.available())
  {
    char c = Serial.read();
    input = input + c;
  }
   StaticJsonBuffer<100> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(input);
  byte estRestCred = root["estRestCred"];
  if(estRestCred == 1)
  {



    EEPROM.begin(500); 
    EEPROM.write(250, 2);
    EEPROM.write(251, 50);
    EEPROM.write(252, 2);
    EEPROM.write(253, 2);
    EEPROM.write(254, 40);
    EEPROM.write(255, 18);
    EEPROM.write(256, 20);
    EEPROM.write(257, 2);
    EEPROM.write(258, 40);
    EEPROM.write(259, 12);
    EEPROM.write(260, 13);
    EEPROM.write(261, -3);
    EEPROM.write(262, 0);     
    EEPROM.end();
    
    E.grabar(0, "");
    E.grabar(50, "");
    E.grabar(100, "TermoEnergy");
    E.grabar(150, "12345678");

  RefreshCredentials();
   
  }
  float estadoTemperatura = root["estTemp"];
  if(estadoTemperatura != 0){
  T.setTemp(estadoTemperatura);
  }
  T.SetEstado();
  delay(10);
}
}

void mqttTrhead()
{
if(T.getMqttState() == true)
{
  
   
    client.loop();
  
    unsigned long currentMillis = millis();
  
      if (currentMillis - previousMillis >= 10000) 
        { //envia la temperatura cada 10 segundos
        previousMillis = currentMillis;
         
        char k[(htmlWeb.length()+1)];
        htmlWeb.toCharArray(k, (htmlWeb.length()+1));
        client.publish(KEPT_ALIVE, k);
      }
}    
}
