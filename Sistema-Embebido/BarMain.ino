#include <Adafruit_NeoPixel.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <HX711.h>
#include <SoftwareSerial.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

//DEFINES
#define DOUT  A1
#define CLK  A0
#define LED_PIN    6
#define LED_COUNT 16
#define ESPERANDO_ORDEN 0
#define PREPARANDO_TRAGO 1
#define CALIBRANDO 2

//DECLARACIONES
SoftwareSerial blue(10, 11); //El pin 10 sera el Rx, y el pin 11 sera el Tx
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
// Addr, En, Rw, Rs, d4, d5, d6, d7, backlighpin, polarity (azul al A5 y violeta al a4)   
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
HX711 balanza(DOUT, CLK);


float escala = 437.52;
float objetivo = 0;
float total = 1000.0;
int encendido = 0;
float valorAnt=0.0;
float porcentaje_orden = 0.0;
/*
enum Tipo_estado {
    ESPERANDO_ORDEN, 0 
    CALIBRANDO,  1
    PREPARANDO_TRAGO,  2   
};

Tipo_estado estado = ESPERANDO_ORDEN;
*/
int estado = 0;

char aux[3];
//SETUP
void setup() {
  Serial.begin(9600);
  inicioDisplay();
  lcd.print("Inicializando");
  inicioBluetooth();
  inicioBalanza();
  inicioNeoPixel();  
}

void loop() {
  if ( estado == CALIBRANDO){
      
  }else if( estado == ESPERANDO_ORDEN ){ 
    int i = 0;
    if(blue.available()){
      while(blue.available()){
        aux[i] = blue.read();
        Serial.println(aux);
        i++;
        delay(50);
      }
      blue.flush();
      objetivo = total * (atoi(aux))/100;
      lcd.clear();
      lcd.print(objetivo);
      estado = PREPARANDO_TRAGO;
      estado = 1;
    }
    Serial.println(estado);
  } else if (estado == PREPARANDO_TRAGO ){
    Serial.println("Preparando trago");
    int valor = balanza.get_value(10)/escala;
    
    lcd.clear();
    lcd.print("Esto pesa (grs):");
    
     
    lcd.setCursor(0,1);  
    lcd.print(valor);
    float porcentaje=valor/objetivo;
     
    int cantLeds= LED_COUNT*porcentaje;
     
    if(cantLeds==0){
      strip.clear();
      strip.show();
    }else if(porcentaje<=1 && valor!=valorAnt){
      valorAnt=valor;
      llenarAnillo(strip.Color(  0, 255,   0), 50, cantLeds);
      encendido=1;
    } else if(porcentaje>1) {
      pasoDePeso(encendido);
    }
  } 
}

void llenarAnillo(uint32_t color, int wait, int cant) {
  strip.clear();
  for(int i=0; i<cant; i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
  }
}

void pasoDePeso(int enc){
    if(enc){ 
      strip.clear();
      strip.show();
      encendido=0;
    } else {
      llenarAnillo(strip.Color(255,   0,   0), 50, LED_COUNT);
      encendido=1;
    }
}

//INICIOS
void inicioBluetooth(){
  blue.begin(9600); //Se inicia el tmserial
  blue.write("Bluetooth encendido..."); //Lo que se va a imprimir una vez conectado 
  blue.println(); //Espacio
}

void inicioBalanza(){
  balanza.tare(20);  //El peso actual es considerado Tara.
}

void inicioNeoPixel(){
   // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
}

void inicioDisplay(){
   lcd.begin(16,2);
   lcd.setCursor(0,0); 
}
 
