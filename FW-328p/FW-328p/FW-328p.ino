
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <avr/wdt.h>
#include <Thread.h>
#include <ThreadController.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <EEPROM.h>


#define ledPin 8
#define DallasPin 9
#define RelePin 13
#define resetPin 10
#define pullSwuitch 5

SoftwareSerial WIFI(3, 2); // RX | TX

OneWire oneWire(DallasPin);
DallasTemperature sensors(&oneWire);
Thread CadaUnMinuto = Thread();
Thread ControlRele = Thread();


class Termotanque
{
  private:
          byte estadoRele;     // rele on/off 
          byte estadoConexion; // con/sin internet
          int contadorRele;    // 
          int contadorTemperatura; 
          int contadorWDserial;    
          float temperaturaGlobal;

  public:
          void SumarContTemp(){contadorTemperatura ++;}
          int GetContTemp(){return contadorTemperatura;}
          void ResetWDserial(){contadorWDserial = 0;}
          void sumarWDserial(){contadorWDserial++;}
          void sumarContRele(){contadorRele++;}
          int GetContRele(){return contadorRele;}
          int GetContadorWDserial(){return contadorWDserial;}
          
          void GetTemp() // obtiene la temp y rsetea el cont temp
          {
            sensors.requestTemperatures();
            temperaturaGlobal = sensors.getTempCByIndex(0);
            contadorTemperatura = 0;
          }
          
          void setEstadoConexion(byte estCone) // fijar estado conexion 
          {
              estadoConexion = estCone;
          }
          
          void setEstadoRele(byte estRele) // fijar estado rele
            {
              estadoRele = estRele;
            }
            
          void sendEstado() // enviar estado al ESP, suma WD serial
           {
               String temp = String(temperaturaGlobal);
               String data = "{ \"estTemp\": \"" + temp + "\"}";
               WIFI.print(data);
               Serial.println(data);
               sumarWDserial();
           }
           
          void Rele() // prende o apaga el rele y resetea contador rele
           {
            if(temperaturaGlobal > 0 && temperaturaGlobal < 65) // si esta dentro del rango 0-65 ->
            {
              if(estadoRele == 1) // y ademas el estado es encendido ->
              {
                digitalWrite(RelePin, HIGH);
              }else{digitalWrite(RelePin, LOW);}
              
            }else{digitalWrite(RelePin, LOW);}
            contadorRele = 0;
           }
};

Termotanque T;

void setup(void) {

  Serial.begin(115200);
  WIFI.begin(4800);
  sensors.begin();

  pinMode(ledPin, OUTPUT);
  pinMode(RelePin, OUTPUT);
  pinMode(resetPin, OUTPUT); 
  pinMode(DallasPin,INPUT);
  pinMode(pullSwuitch,INPUT_PULLUP);
  digitalWrite(resetPin, LOW);

  CadaUnMinuto.onRun(cadaUnSegundo);
  ControlRele.onRun(controlRele);
}

void loop() {
  CadaUnMinuto.run();
  ControlRele.run();


  if(WIFI.available())
      {
         String input = "";
         while(WIFI.available())
         {
            char c = WIFI.read();
            input = input + c;
         }
         Serial.println(input);
         StaticJsonBuffer<50> jsonBuffer;
         JsonObject& root = jsonBuffer.parseObject(input);
     
         byte estRele = root["abc"];
         byte estCon = root["estConnection"];
    
        if(estCon != 0)
        {
         T.setEstadoConexion(estCon);
         Serial.println(estCon);
        }
        
        if(estRele != 0) // reser WD serial
        {
         T.setEstadoRele(estRele);
         T.ResetWDserial();
         Serial.println(estRele);
         }
      }

  
  if(T.GetContTemp() > 20){ wdt_enable(WDTO_30MS); while(1){};}
  if(T.GetContRele() > 10){ wdt_enable(WDTO_30MS); while(1){};}
  if(T.GetContadorWDserial() > 5) //reser ESP
  {
      digitalWrite(resetPin, HIGH);
      delay(10);
      digitalWrite(resetPin, LOW);
  }
  
}

void cadaUnSegundo()
{
  T.GetTemp();    // reser cont temp
  T.sendEstado(); // suma WD serial
  delay(1000);
}

void controlRele()
{
  T.Rele();            //resetea cont rele
  T.SumarContTemp();   // suma cont temp
  T.sumarContRele();   // suma cont rele 
  delay(250);
}
