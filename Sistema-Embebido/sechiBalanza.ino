#include <HX711.h>

#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

#define DOUT  A1
#define CLK  A0

HX711 balanza(DOUT, CLK);

//creamos el objeto display LCD I2C
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 
// Addr, En, Rw, Rs, d4, d5, d6, d7, backlighpin, polarity (azul al A5 y violeta al a4)   

float escala = 437.52;
void setup() {
  Serial.begin(9600);
  lcd.begin(16,2);
  lcd.setCursor(0,0); 
  lcd.print("Calibrando...");
  
  Serial.print("Lectura del valor del ADC:  ");
  Serial.println(balanza.read());
  Serial.println("No ponga ningun  objeto sobre la balanza");
  Serial.println("Destarando...");
  
  //balanza.set_scale(); //La escala por defecto es 1
  //balanza.set_scale(437.62);                          
  balanza.tare(20);  //El peso actual es considerado Tara.
  Serial.println("Coloque un peso conocido:");

  lcd.clear();
  lcd.print(0.00);  
}


void loop() {

  Serial.print("Valor de lectura:  ");
  int valor = balanza.get_value(10)/escala;
  Serial.println(valor);
  lcd.clear();
  lcd.print("Esto pesa:");
  lcd.setCursor(0,1);  
  lcd.print(valor);
      
  delay(100);
}
