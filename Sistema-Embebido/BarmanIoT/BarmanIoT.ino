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
//PINES
#define DOUT A1 //Data de la balanza (cable naranja)
#define CLK  A0 //Clock de la balanza (cable amarillo)
#define TEMP_PIN  A2
#define LED_PIN    6
#define LED_COUNT 16
#define BOTON_1_PIN 2
#define BOTON_2_PIN 3
#define BUZZER_PIN 9
//ESTADOS
#define ESPERANDO_ORDEN 0
#define RECIBIENDO_ORDEN 1
#define SELECCION_VASO 2
#define CALIBRANDO_VACIO 3
#define CALIBRANDO_LLENO 4
#define PREPARANDO_TRAGO 5
#define CONTROLANDO_INGREDIENTE 6
#define PIDIENDO_TEMPERATURA 7
#define CONTROLANDO_TEMPERATURA 8
#define FINALIZANDO_TRAGO 9
#define ERROR_CALIBRANDO 10
#define CALIBRADO_EXITOSO 11
#define SACO_VASO 12

//VASOS
#define NO_SELECCIONADO -1
#define VASO_1 0
#define VASO_2 1
//ORDENES
#define ES_TRAGO '0'
#define ES_CALIBRAR '1'
#define ES_CANCELAR '2'
#define TRAGO_FINALIZADO "100"

//Structs
typedef struct {
  char nombre[16];
  int porcentaje;
} t_ingrediente;

typedef struct {
  char nombre[16];
  int cantidad;
  t_ingrediente ingredientes[5]; 
} t_bebida;


//DECLARACIONES
SoftwareSerial blue(10,11); //El pin 10 sera el Rx, y el pin 11 sera el Tx
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
// Addr, En, Rw, Rs, d4, d5, d6, d7, backlighpin, polarity (azul al A5 y violeta al a4)   
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
HX711 balanza(DOUT, CLK);

float escala = 437.52f;
float objetivo = 0.0f;
float total_vaso[2]={0.0f,0.0f};
float peso_vaso[2]={0.0f,0.0f};
float ya_cargado = 0.0f;
int encendido = 0;
float valorAnt=0.0f;
int estado=ESPERANDO_ORDEN;
int vaso=NO_SELECCIONADO;
t_bebida bebida_actual;
int ingrediente_actual=0;
int estadoPrevio = -1;
bool calibrar = false;
bool presionado = false;
unsigned long currentMillis;
unsigned long previousMillis;
unsigned long esperaNoBloq=2000;

void setup() {
  inicioDisplay();
  lcd.print("Inicializando");
  inicioBluetooth();
  inicioBalanza();
  inicioNeoPixel();
  //Inicio Botones
  pinMode(BOTON_1_PIN, INPUT);
  pinMode(BOTON_2_PIN, INPUT);
  //Inicio Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  lcd.clear();
  lcd.print("Iniciado");
}

void loop() {
if(estado == ESPERANDO_ORDEN){
    if(estado!=estadoPrevio){
        estadoPrevio=estado;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Esperando orden"); 
    }
    if(blue.available())
      estado = RECIBIENDO_ORDEN;
} else if (estado == RECIBIENDO_ORDEN) {
      if(estado!=estadoPrevio){
        estadoPrevio=estado;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Orden");
        lcd.setCursor(0,1);
        lcd.print("recibida!");
        resetPreviousMillis();  
      }
      currentMillis=millis();
      if(currentMillis-previousMillis >= esperaNoBloq){
        char orden = blue.read();
        if(orden == ES_TRAGO){
          char lectura=NULL;
          for(int i = 0; i < 16; i++){
            lectura = blue.read();
            if(lectura!='*'){
              bebida_actual.nombre[i] = lectura;
            } else {
               bebida_actual.nombre[i]='\0';
            }
          }
          int j = 0;
          while(blue.available()){
            for(int i = 0; i<16; i++){
                lectura = blue.read();
                if(lectura!='*'){
                  bebida_actual.ingredientes[j].nombre[i] = lectura;
                } else {
                  bebida_actual.ingredientes[j].nombre[i]='\0';
                }
            }
            char blue_buffer[3];
            for(int i = 0; i < 3; i++){
              blue_buffer[i] = blue.read();
            }
            bebida_actual.ingredientes[j].porcentaje = atoi(blue_buffer);
            j++;
          }
          bebida_actual.cantidad = j;
          estado = SELECCION_VASO;  
        } else if(orden == ES_CALIBRAR){
          calibrar=true;
          estado = ESPERANDO_ORDEN; 
        }
      }
} else if (estado == SELECCION_VASO && !cancelarTrago() ) {
       if(estado!=estadoPrevio) {
          estadoPrevio=estado;
          lcd.clear();
          lcd.print("  Seleccione");
          lcd.setCursor(0,1);
          lcd.print("   su vaso!");
          resetPreviousMillis();
        }
        currentMillis=millis();
        if(currentMillis-previousMillis >= esperaNoBloq){  
           if( digitalRead(BOTON_1_PIN ) == HIGH && digitalRead(BOTON_2_PIN) == LOW ){ //SELECCIONO EL VASO 1
            vaso=VASO_1;
          } else if (digitalRead(BOTON_2_PIN ) == HIGH && digitalRead(BOTON_1_PIN) == LOW) { //SELECCIONO EL VASO 2
            vaso=VASO_2;
          }
          if(calibrar && vaso != NO_SELECCIONADO){
            peso_vaso[vaso] = 0.0f;
            calibrar = false;
          }
          if( vaso != NO_SELECCIONADO){
            (peso_vaso[vaso] == 0.0f)?estado=CALIBRANDO_VACIO:estado=PREPARANDO_TRAGO;
          } 
        }
} else if ( estado == PREPARANDO_TRAGO  && !cancelarTrago()) {
    if(ingrediente_actual<bebida_actual.cantidad){
      if(estado!=estadoPrevio){
        estadoPrevio=estado;
        lcd.clear();
        lcd.print("Agregue:");
        lcd.setCursor(0,1);
        lcd.print(bebida_actual.ingredientes[ingrediente_actual].nombre);  
      }
      objetivo=total_vaso[vaso]*bebida_actual.ingredientes[ingrediente_actual].porcentaje/100;
      estado=CONTROLANDO_INGREDIENTE;  
      
    }else{
      objetivo=0.0f;
      ya_cargado=0.0f;
      ingrediente_actual=0;
      vaso=NO_SELECCIONADO;
      estado = CONTROLANDO_TEMPERATURA;
    }
}else if(estado == CONTROLANDO_INGREDIENTE && !cancelarTrago()) {
    if(estado!=estadoPrevio)
      estadoPrevio=estado;
    int valor = balanza.get_value(10)/escala - peso_vaso[vaso]-ya_cargado;
    if(valor>=0){
      float porcLeds=valor/objetivo;  
      int cantLeds= LED_COUNT*porcLeds;  
      if(cantLeds==0){
        strip.clear();
        strip.show();
        noTone(BUZZER_PIN);
      } else if(porcLeds<=1 && valor!=valorAnt){
        valorAnt=valor;
        llenarAnillo(strip.Color(  0, 255,   0), 50, cantLeds);
        encendido=1;
        noTone(BUZZER_PIN);
      } else if(porcLeds>1) {
        pasoDePeso(encendido);          
      }
      if((vaso == VASO_1)?digitalRead(BOTON_1_PIN):digitalRead(BOTON_2_PIN) && !presionado){
        valor = balanza.get_value(10)/escala - peso_vaso[vaso]-ya_cargado;
        ya_cargado+=valor;
        ingrediente_actual++;
        if (encendido){
          pasoDePeso(encendido);
        }
          estado=PREPARANDO_TRAGO;
        }
      } else {
        if (encendido){
          pasoDePeso(encendido);
        }
        estado=SACO_VASO; 
      }
}else if( estado == SACO_VASO && !cancelarTrago()){
  if(estado!=estadoPrevio){
    estadoPrevio=estado;
      lcd.clear();
      lcd.print("Saco el vaso");
      lcd.setCursor(0,1);
      lcd.print("Reintente");
    resetPreviousMillis();
  }
  currentMillis=millis();
  if(currentMillis-previousMillis>=esperaNoBloq){
      estado = PREPARANDO_TRAGO;
  }
} else if ( estado == CALIBRANDO_VACIO  && !cancelarTrago()) {
    if(estado != estadoPrevio){
        estadoPrevio = estado;
        lcd.clear();
        lcd.print("Coloque el vaso");
        lcd.setCursor(0,1);
        lcd.print("vacio");
        resetPreviousMillis();
     }
     currentMillis=millis();
     if(currentMillis-previousMillis >= esperaNoBloq){
        if( (vaso == VASO_1)?digitalRead(BOTON_1_PIN) == HIGH:digitalRead(BOTON_2_PIN) == HIGH){
         peso_vaso[vaso] = balanza.get_value(10) / escala;
         estado = CALIBRANDO_LLENO;
        }
     }
} else if( estado == CALIBRANDO_LLENO && !cancelarTrago()){
       if(estado!=estadoPrevio){
          estadoPrevio=estado;
          lcd.clear();
          lcd.print("Coloque el vaso");
          lcd.setCursor(0,1);
          lcd.print(" lleno de Agua");
          resetPreviousMillis();
     }
       currentMillis=millis();
       if(currentMillis-previousMillis >= esperaNoBloq){
         if( (vaso == VASO_1)?digitalRead(BOTON_1_PIN) == HIGH:digitalRead(BOTON_2_PIN) == HIGH){
            float peso_con_agua = balanza.get_value(10) / escala;
            if(peso_vaso[vaso] > peso_con_agua){
              estado=ERROR_CALIBRANDO;
            } else {
              total_vaso[vaso] = peso_con_agua - peso_vaso[vaso];
              estado=CALIBRADO_EXITOSO;
          }
       }
     }
} else if (estado == CALIBRADO_EXITOSO && !cancelarTrago()) {
    if(estado!=estadoPrevio){
        estadoPrevio=estado;
        lcd.clear();
        lcd.print("VASO CALIBRADO");
        lcd.setCursor(0,1);
        lcd.print("A preparar!");
        resetPreviousMillis();
      }
      currentMillis=millis();
      if(currentMillis-previousMillis>=esperaNoBloq*3)
        estado = PREPARANDO_TRAGO;  
} else if (estado == ERROR_CALIBRANDO && !cancelarTrago()) {
    if(estado!=estadoPrevio){
        estadoPrevio=estado;
        lcd.clear();
        lcd.print("ERROR al calibrar");
        lcd.setCursor(0,1);
        lcd.print("Reintente");
        resetPreviousMillis();
     }
       currentMillis=millis();
     if(currentMillis-previousMillis >= esperaNoBloq){
        peso_vaso[vaso] = 0.0f;
        estado=CALIBRANDO_VACIO;
    }
}else if (estado == CONTROLANDO_TEMPERATURA ) {
    if(estado!=estadoPrevio){
      estadoPrevio=estado;
      lcd.clear();
      lcd.print("   Controlando  ");
      lcd.setCursor(0,1);
      lcd.print("   temperatura  ");

      float temp1 = 5.0f * analogRead(TEMP_PIN) * 100.0f / 1023.0f;
      delay(1000); //espera bloqueante porque no hay nada que bloquee
      float temp2 = 5.0f * analogRead(TEMP_PIN) * 100.0f / 1023.0f;
      if( (temp2 - 5.0f > temp1) || ((temp1+temp2)/2 > 15 ) ) {
        lcd.clear();
        lcd.print("Agregue hielo");         
      } else {
          lcd.clear();
          lcd.print(" Temperatura  ");
          lcd.setCursor(0,1);
          lcd.print("  Correcta  ");
       }
       resetPreviousMillis();
     }
     
     currentMillis=millis();
     if(currentMillis-previousMillis >= esperaNoBloq){
         estado = FINALIZANDO_TRAGO; 
     }
} else if (estado == FINALIZANDO_TRAGO){ 
      if(estado!=estadoPrevio){
        estadoPrevio=estado;
        lcd.clear();
        lcd.print("   Trago  ");
        lcd.setCursor(0,1);
        lcd.print("   Finalizado!  ");
        blue.write(TRAGO_FINALIZADO);
        resetPreviousMillis();
       }
         currentMillis=millis();
         if(currentMillis-previousMillis >= esperaNoBloq){
            estado=ESPERANDO_ORDEN;
         }
  }
  if(vaso != NO_SELECCIONADO) {
    if ( (vaso == VASO_1)?digitalRead(BOTON_1_PIN) == LOW:digitalRead(BOTON_2_PIN) == LOW ){
      presionado = false;
    }
  }
}

boolean cancelarTrago(){
  if(blue.available()){
    char val = blue.read();
    if (val == ES_CANCELAR){
      estado = ESPERANDO_ORDEN;
      ya_cargado = 0.0f;
      vaso = NO_SELECCIONADO;
      estadoPrevio = NO_SELECCIONADO;
      return true;
    }
}
  return false;
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
      noTone(BUZZER_PIN);
      encendido=0;
    } else {
      llenarAnillo(strip.Color(255,   0,   0), 50, LED_COUNT);
      tone(BUZZER_PIN, 1000);
      encendido=1;
    }
}

//INICIOS
void inicioBluetooth(){
  blue.begin(9600); //Se inicia el tmserial
}

void inicioBalanza(){
  balanza.tare(10);  //El peso actual es considerado Tara.
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

void resetPreviousMillis(){
  currentMillis=millis();
  previousMillis=currentMillis;
}
