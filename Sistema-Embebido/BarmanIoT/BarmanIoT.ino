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
#define CALIBRANDO 3
#define PREPARANDO_TRAGO 4
#define CONTROLANDO_INGREDIENTE 5
#define PIDIENDO_TEMPERATURA 6
#define CONTROLANDO_TEMPERATURA 7
#define FINALIZANDO_TRAGO 8
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
SoftwareSerial blue(10, 11); //El pin 10 sera el Rx, y el pin 11 sera el Tx
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
// Addr, En, Rw, Rs, d4, d5, d6, d7, backlighpin, polarity (azul al A5 y violeta al a4)   
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
HX711 balanza(DOUT, CLK);

float escala = 437.52;
float objetivo = 0;
float total_vaso[2];
float peso_vaso[2];
float ya_cargado = 0.0f;
int encendido = 0;
float valorAnt=0.0f;
int estado=ESPERANDO_ORDEN;
int vaso=NO_SELECCIONADO;
t_bebida bebida_actual;
int ingrediente_actual;
bool escrito = false;
bool calibrar = false;
bool buzzer_on = false;
unsigned long previousMillis;

void setup() {
  Serial.begin(9600);
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
  unsigned long currentMillis = millis();
  if( estado == ESPERANDO_ORDEN){
    if(!escrito){
      Serial.println("Esperando orden");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Esperando orden"); 
      escrito = true;
    }
    if(blue.available()){
      estado = RECIBIENDO_ORDEN;
      escrito = false;
    }
  } else if (estado == RECIBIENDO_ORDEN) {
    Serial.println("Recibiendo orden");
    char orden = blue.read();
    switch (orden){
      case ES_TRAGO:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Trago");
        lcd.setCursor(0,1);
        lcd.print("seleccionado!");
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
        Serial.println(bebida_actual.nombre);
        Serial.println(bebida_actual.cantidad);
        for( int k = 0; k < j; k++){
          Serial.println(bebida_actual.ingredientes[k].nombre);
          Serial.println(bebida_actual.ingredientes[k].porcentaje);
        }
        estado = SELECCION_VASO;
      break;
      case ES_CALIBRAR:
        calibrar = true;
      break;
      default: 
        Serial.println("Orden invalida!");
      break;
    }
  } else if (estado == SELECCION_VASO && !cancelarTrago() ) {
    if ( !escrito ) {
      lcd.clear();
      lcd.print("  Seleccione");
      lcd.setCursor(0,1);
      lcd.print("   su vaso!");
      escrito = true;
    }  
    if( digitalRead(BOTON_1_PIN ) == HIGH && digitalRead(BOTON_2_PIN) == LOW ){ //SELECCIONO EL VASO 1
      vaso=VASO_1;
      Serial.println("selecciono el vaso 1");
    } else if (digitalRead(BOTON_2_PIN ) == HIGH && digitalRead(BOTON_1_PIN) == LOW) { //SELECCIONO EL VASO 2
      vaso=VASO_2;
      Serial.println("selecciono el vaso 2");
    }
    if(calibrar && vaso != NO_SELECCIONADO){
      peso_vaso[vaso] = 0.0f;
      calibrar = false;
    }
    if( vaso != NO_SELECCIONADO){
      (peso_vaso[vaso] == 0.0f)?estado=CALIBRANDO:estado=PREPARANDO_TRAGO;
      escrito = false;
      if(estado == CALIBRANDO){
          lcd.clear();
          lcd.print("Coloque el vaso");
          lcd.setCursor(0,1);
          lcd.print("vacio");
          delay(1000); 
      }
    }
  } else if ( estado == PREPARANDO_TRAGO  && !cancelarTrago()) {
    if(ingrediente_actual<bebida_actual.cantidad){
      lcd.clear();
      lcd.print("Agregue:");
      lcd.setCursor(0,1);
      lcd.print(bebida_actual.ingredientes[ingrediente_actual].nombre);
      objetivo=total_vaso[vaso]*bebida_actual.ingredientes[ingrediente_actual].porcentaje/100;
      estado=CONTROLANDO_INGREDIENTE; 
    } else {
      objetivo=0.0f;
      ya_cargado=0.0f;
      ingrediente_actual=0;
      vaso=NO_SELECCIONADO;
      estado = PIDIENDO_TEMPERATURA;
    }
  } else if ( estado == CONTROLANDO_INGREDIENTE && !cancelarTrago()) {
    int valor = balanza.get_value(10)/escala - peso_vaso[vaso]-ya_cargado;
    if(valor>=0){
       float porcLeds=valor/objetivo;  
       int cantLeds= LED_COUNT*porcLeds;  
       if(cantLeds==0){
          strip.clear();
          strip.show();
          noTone(BUZZER_PIN);
          buzzer_on = false;
       }else if(porcLeds<=1 && valor!=valorAnt){
          valorAnt=valor;
          llenarAnillo(strip.Color(  0, 255,   0), 50, cantLeds);
          encendido=1;
          noTone(BUZZER_PIN);
          buzzer_on = false;
       } else if(porcLeds>1) {
          //prendo led
          pasoDePeso(encendido);
          if( currentMillis - previousMillis >= 100 ){
            previousMillis = currentMillis;
            if( buzzer_on ) {
              noTone(BUZZER_PIN);
              buzzer_on = false;
            } else {
              tone(BUZZER_PIN, 1000);
              buzzer_on = true;
            }
          }          
       }
       if((vaso == VASO_1)?digitalRead(BOTON_1_PIN):digitalRead(BOTON_2_PIN)){
         valor = balanza.get_value(10)/escala - peso_vaso[vaso]-ya_cargado;
         ya_cargado+=valor;
         ingrediente_actual++;
         if (buzzer_on == true){
          noTone(BUZZER_PIN);
          buzzer_on = false;
         }
         estado=PREPARANDO_TRAGO;
       }
    } else {
      buzzer_on = false;
      noTone(BUZZER_PIN);
      lcd.clear();
      lcd.print("  Sacaste el  ");
      lcd.setCursor(0,1);
      lcd.print("    vaso!    ");
      delay(1000);
      lcd.clear();
      lcd.print("Prepare de nuevo");
      delay(1000);
      // ESCRIBIR "VUELVA A PREPARARLO"
      estado = PREPARANDO_TRAGO;
    }
  } else if ( estado == CALIBRANDO  && !cancelarTrago()) {
    if( peso_vaso[vaso] == 0.0f){
      if( (vaso == VASO_1)?digitalRead(BOTON_1_PIN) == HIGH:digitalRead(BOTON_2_PIN) == HIGH){
         peso_vaso[vaso] = balanza.get_value(10) / escala;
         lcd.clear();
         lcd.print("Coloque el vaso");
         lcd.setCursor(0,1);
         lcd.print(" lleno de Agua");
         delay(1000);
      }
    } else {
      if( (vaso == VASO_1)?digitalRead(BOTON_1_PIN) == HIGH:digitalRead(BOTON_2_PIN) == HIGH){
        float peso_con_agua = balanza.get_value(10) / escala;
        if(peso_vaso[vaso] > peso_con_agua){
          lcd.clear();
          lcd.print("ERROR al calibrar");
          lcd.setCursor(0,1);
          lcd.print("Vuelva a intentarlo");
          vaso = NO_SELECCIONADO;
          peso_vaso[vaso] = 0.0f;
        } else {
          total_vaso[vaso] = peso_con_agua - peso_vaso[vaso];
          lcd.clear();
          lcd.print("VASO CALIBRADO");
          lcd.setCursor(0,1);
          lcd.print("A preparar!");
          delay(1000);
        }
        estado = PREPARANDO_TRAGO;
        Serial.println(total_vaso[0]);
      }
    }
  } else if ( estado == PIDIENDO_TEMPERATURA && !cancelarTrago()) {
    if(!escrito){
      lcd.clear();
      lcd.print("Acerque el vaso");
      lcd.setCursor(0,1);
      lcd.print(" al termometro");
      escrito = true;
      Serial.println("Pedí temperatura");
    }
    if( (vaso == VASO_1)?digitalRead(BOTON_2_PIN) == HIGH:digitalRead(BOTON_1_PIN) == HIGH){
      Serial.println("Acercó el vaso");
      Serial.println(estado);
      estado = CONTROLANDO_TEMPERATURA;
      Serial.println(estado);
      escrito = false;
    }
  } else if (estado == CONTROLANDO_TEMPERATURA ) {
    if(!escrito){
      lcd.clear();
      lcd.print("   Controlando  ");
      lcd.setCursor(0,1);
      lcd.print("   temperatura  ");
      escrito = true;
    }
    //tomo una temperatura
    float temp1 = 5.0f * analogRead(TEMP_PIN) * 100.0f / 1023.0f;
    Serial.print("la temperatura es ");
    Serial.println(temp1);
    delay(1000); //espero 1 segundo
    //tomo la segunda temperatura
    float temp2 = 5.0f * analogRead(TEMP_PIN) * 100.0f / 1023.0f;
    Serial.print("la temperatura es ");
    Serial.println(temp2);
    if( (temp2 - 5.0f > temp1) || ((temp1+temp2)/2 > 15 ) ) {
      lcd.clear();
      lcd.print("Agregue hielo");
      delay(2000);
    }
    escrito = false;
    estado = FINALIZANDO_TRAGO;
  } else if (estado == FINALIZANDO_TRAGO){ 
    blue.write(TRAGO_FINALIZADO);
    lcd.clear();
    lcd.print("    Trago    ");
    lcd.setCursor(0,1);
    lcd.print("  Finalizado!");
    delay(2000);
    estado = ESPERANDO_ORDEN;
  } else if (cancelarTrago()) {
    lcd.clear();
    lcd.print("Trago cancelado");
    delay(2000);
    lcd.clear();
    lcd.print("Esperando orden");
    estado = ESPERANDO_ORDEN;
  }else {
    Serial.println("Estado Invalido");
    delay(2000);
    estado = ESPERANDO_ORDEN;
  }
}
boolean cancelarTrago(){
  if(blue.available()){
    char val = blue.read();
    if (val == ES_CANCELAR){
      estado = ESPERANDO_ORDEN;
      ya_cargado = 0.0f;
      vaso = NO_SELECCIONADO;
      escrito=false;
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
      encendido=0;
    } else {
      llenarAnillo(strip.Color(255,   0,   0), 50, LED_COUNT);
      encendido=1;
    }
}

//INICIOS
void inicioBluetooth(){
  blue.begin(9600); //Se inicia el tmserial
  blue.flush();
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
