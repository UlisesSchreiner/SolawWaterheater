#include "Arduino.h"
#include <EEPROM.h>

class Eeprom
{
  private:
           int addrSsid = 0;
           //tamaño ssid = EEPROM.read(21);
  public:
          void grabar(int addr, String a) {
            EEPROM.begin(500);
  int tamano = a.length(); 
  char inchar[50]; 
  a.toCharArray(inchar, tamano+1);
  for (int i = 0; i < tamano; i++) {
    EEPROM.write(addr+i, inchar[i]);
  }
  for (int i = tamano; i < 50; i++) {
    EEPROM.write(addr+i, 255);
  }
  EEPROM.commit();
  //EEPROM.end();
}


String leer(int addr) {
  EEPROM.begin(500);
   byte lectura;
   String strlectura;
   for (int i = addr; i < addr+50; i++) {
      lectura = EEPROM.read(i);
      if (lectura != 255) {
        strlectura += (char)lectura;
      }
   }
   return strlectura;
   EEPROM.end();
}

};
