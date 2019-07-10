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
#define ESCALA 437.52f
#define CERO 0.0f
#define UNO 1
#define MIN 1
#define TARA 10
#define CIEN 100
#define BAUDIOSBT 9600
#define FULLCOLOR 255
#define BRILLO 50
#define CELDASLCD 16
#define FILASLCD 2
#define FILA1 0
#define FILA2 1
#define CELDA1 0
#define ESPERA_NO_BLOQ 2000
#define ESPERA_VACIAR_VASO 6000
#define FREQ_ALTA 2000
#define FREQ_BAJA 1000
//PINES
#define DOUT A1 //Data de la balanza (cable naranja)
#define CLK  A0 //Clock de la balanza (cable amarillo)
#define TEMP_PIN  A2
#define LED_PIN    6
#define LED_COUNT 16
#define BOTON_1_PIN 2
#define BOTON_2_PIN 3
#define BUZZER_PIN 9
#define RXBT 11 
#define TXBT 10
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
#define PREVIO_FIN 13

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
SoftwareSerial blue(TXBT,RXBT); //El pin 10 sera el Rx de la arduino, y el pin 11 sera el Tx de la arduino
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
// Addr, En, Rw, Rs, d4, d5, d6, d7, backlighpin, polarity (azul al A5 y violeta al a4)   
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
HX711 balanza(DOUT, CLK);

float objetivo = CERO;
float total_vaso[2]={CERO,CERO};
float peso_vaso[2]={CERO,CERO};
float ya_cargado = CERO;
bool encendido = false;
float valor=CERO;
float valorAnt=CERO;
int estado=ESPERANDO_ORDEN;
int estadoPrevio = NO_SELECCIONADO;
int vaso=NO_SELECCIONADO;
t_bebida bebida_actual;
int ingrediente_actual=CERO;
bool calibrar = false;
unsigned long currentMillis;
unsigned long previousMillis;
bool alto = false;

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
    iniciarEstado("Esperando orden","");
    if(blue.available())
      estado = RECIBIENDO_ORDEN;
} else if (estado == RECIBIENDO_ORDEN) {
    iniciarEstado("Orden","Recibida!");
    pasoARecibirOrden();
} else if (estado == SELECCION_VASO && !cancelarTrago() ) {
    iniciarEstado("Seleccione","su vaso!");
    pasoASeleccionVaso();
} else if ( estado == PREPARANDO_TRAGO  && !cancelarTrago()) {
    (ingrediente_actual<bebida_actual.cantidad)?pasoAControlarIngrediente():pasoAControlarTemperatura();
}else if(estado == CONTROLANDO_INGREDIENTE && !cancelarTrago()) {
    valor = round(balanza.get_value(TARA)/ESCALA) - peso_vaso[vaso]-ya_cargado;
    if(valor>=CERO){
      controlarIngrediente();
    } else if(ya_cargado>CERO){
     apagarAlertas();
     estado=SACO_VASO; 
    }
}else if( estado == SACO_VASO && !cancelarTrago()){
    iniciarEstado("Saco el vaso","Reintente");
    pasoAPreparandoTrago(ESPERA_NO_BLOQ);
} else if ( estado == CALIBRANDO_VACIO  && !cancelarTrago()) {
    iniciarEstado("Coloque el vaso","vacio");
    valor = round(balanza.get_value(TARA)/ESCALA);
    pasoACalibrandoLleno();
} else if( estado == CALIBRANDO_LLENO && !cancelarTrago()){
    iniciarEstado("Coloque el vaso","lleno de agua");
    valor = round(balanza.get_value(TARA) / ESCALA);
    pasoAResCalibracion(); 
} else if (estado == CALIBRADO_EXITOSO && !cancelarTrago()) {
    iniciarEstado("Vaso calibrado","A preparar!");
    pasoAPreparandoTrago(ESPERA_VACIAR_VASO);  
} else if (estado == ERROR_CALIBRANDO && !cancelarTrago()) {
    iniciarEstado("Error al calibrar","Reintente");
    pasoACalibrandoVacio();
}else if (estado == CONTROLANDO_TEMPERATURA ) {
    iniciarEstado("Controlando","temperatura");
    float temp1 = 5.0f * analogRead(TEMP_PIN) * 100.0f / 1023.0f;
    controlarTemperatura(temp1);
} else if (estado == FINALIZANDO_TRAGO){
    iniciarEstado("Trago","finalizado!");
    pasoAEsperandoOrden();
} else if (estado == PREVIO_FIN){
    iniciarEstado("","");
    pasoAFinalizandoTrago();
  }
}


boolean cancelarTrago(){
  if(blue.available()){
    char val = blue.read();
    if (val == ES_CANCELAR){
      estado = ESPERANDO_ORDEN;
      ya_cargado = CERO;
      vaso = NO_SELECCIONADO;
      estadoPrevio = NO_SELECCIONADO;
      ingrediente_actual=CERO;
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
      apagarAlertas();
      encendido=false;
    } else {
      llenarAnillo(strip.Color(FULLCOLOR, CERO, CERO), BRILLO, LED_COUNT);
      if(alto){
        tone(BUZZER_PIN, FREQ_ALTA);
        alto=false;
      }else{
        tone(BUZZER_PIN,FREQ_BAJA);
        alto=true;
      }
      encendido=true;
    }
}

//INICIOS
void inicioBluetooth(){
  blue.begin(BAUDIOSBT); //Se inicia el tmserial
}

void inicioBalanza(){
  balanza.tare(TARA);  //El peso actual es considerado Tara.
}

void inicioNeoPixel(){
   // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(BRILLO); // Set BRIGHTNESS to about 1/5 (max = 255)
}

void inicioDisplay(){
   lcd.begin(CELDASLCD,FILASLCD);
   lcd.setCursor(CELDA1,FILA1); 
}

void resetPreviousMillis(){
  currentMillis=millis();
  previousMillis=currentMillis;
}

void imprimirMsj(char* arriba, char* abajo){
  lcd.clear();
  lcd.print(arriba);
  lcd.setCursor(CELDA1,FILA2);
  lcd.print(abajo);
}

boolean tiempoCumplido(unsigned long espera){
  return currentMillis - previousMillis >= espera;
}

void pasoARecibirOrden(){
  if(tiempoCumplido(ESPERA_NO_BLOQ)){
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
  currentMillis=millis();
}

void pasoASeleccionVaso(){
  if(tiempoCumplido(ESPERA_NO_BLOQ)){   
    if( digitalRead(BOTON_1_PIN ) == HIGH && digitalRead(BOTON_2_PIN) == LOW ){ //SELECCIONO EL VASO 1
      vaso=VASO_1;
    } else if (digitalRead(BOTON_2_PIN ) == HIGH && digitalRead(BOTON_1_PIN) == LOW) { //SELECCIONO EL VASO 2
      vaso=VASO_2;
    }
    if(calibrar && vaso != NO_SELECCIONADO){
      peso_vaso[vaso] = CERO;
      calibrar = false;
    }
    if( vaso != NO_SELECCIONADO){
      (peso_vaso[vaso] == CERO)?estado=CALIBRANDO_VACIO:estado=PREPARANDO_TRAGO;
    }
  }
  currentMillis=millis();
}

void iniciarEstado(char* arriba, char* abajo) {
  if(estado!=estadoPrevio){
        estadoPrevio=estado;
        if(arriba!="" || abajo!="")
          imprimirMsj(arriba,abajo);
        resetPreviousMillis();
        valorAnt=CERO;
     }
}

boolean vasoEstabilizado(){
  if(!tiempoCumplido(ESPERA_NO_BLOQ)){
    if(valor-valorAnt>=MIN){
      valorAnt=valor;
      resetPreviousMillis();
    }
    return false;
  }
  return true;
}

void pasoAControlarIngrediente(){
  iniciarEstado("Agregue:",bebida_actual.ingredientes[ingrediente_actual].nombre);
  objetivo=total_vaso[vaso]*bebida_actual.ingredientes[ingrediente_actual].porcentaje/CIEN;
  estado=CONTROLANDO_INGREDIENTE;
}

void pasoAControlarTemperatura(){
  objetivo=CERO;
  ya_cargado=CERO;
  ingrediente_actual=CERO;
  vaso=NO_SELECCIONADO;
  estado = CONTROLANDO_TEMPERATURA;
}

void controlarIngrediente(){
  float porcLeds=valor/objetivo;  
  int cantLeds= LED_COUNT*porcLeds;
  if(cantLeds==CERO){
    apagarAlertas();
  } else if(porcLeds<=UNO && valor!=valorAnt){
    mostrarLlenado(cantLeds);
  } else if(porcLeds>UNO) {
    indicarAlto();
  }
}

void apagarAlertas(){
  strip.clear();
  strip.show();
  noTone(BUZZER_PIN);
}

void mostrarLlenado(int cantLeds){
  valorAnt=valor;
  llenarAnillo(strip.Color(CERO,FULLCOLOR,CERO), BRILLO, cantLeds);
  encendido=true;
  noTone(BUZZER_PIN);
}

void indicarAlto(){
  pasoDePeso(encendido);
  iniciarEstado("","");
  currentMillis=millis();
  if(vasoEstabilizado()){
    valor = round(balanza.get_value(TARA)/ESCALA) - peso_vaso[vaso]-ya_cargado;
    ya_cargado+=valor;
    ingrediente_actual++;
    apagarAlertas();
    estado=PREPARANDO_TRAGO;          
  }
}

void pasoACalibrandoLleno(){
  currentMillis=millis();
  if(valor>MIN){
    if(vasoEstabilizado()){
      peso_vaso[vaso]=valor;
      estado=CALIBRANDO_LLENO;     
    } 
  } else {
    resetPreviousMillis();
  }
}

void pasoAResCalibracion(){
  currentMillis=millis();
  if(valor>peso_vaso[vaso]+MIN){
    if(vasoEstabilizado()){
      if(peso_vaso[vaso] >= valor){
        estado=ERROR_CALIBRANDO;
      } else {
        total_vaso[vaso] = valor - peso_vaso[vaso];
        estado=CALIBRADO_EXITOSO;
      }
    }
  } else {
    resetPreviousMillis();
  }
}

void pasoAEsperandoOrden(){
  if(tiempoCumplido(ESPERA_NO_BLOQ)){
    blue.write(TRAGO_FINALIZADO);
    estado=ESPERANDO_ORDEN;
  }
  currentMillis=millis();
}

void pasoAFinalizandoTrago(){
  if(tiempoCumplido(ESPERA_NO_BLOQ)){
    estado = FINALIZANDO_TRAGO; 
  }
  currentMillis=millis();
}

void pasoACalibrandoVacio(){
  if(tiempoCumplido(ESPERA_NO_BLOQ)){
    peso_vaso[vaso] = CERO;
    estado=CALIBRANDO_VACIO;
  }
  currentMillis=millis();
}

void pasoAPreparandoTrago(unsigned long espera){
  if(tiempoCumplido(espera))
       estado = PREPARANDO_TRAGO;
  currentMillis=millis();  
}

void controlarTemperatura(float temp1){
  if(tiempoCumplido(ESPERA_NO_BLOQ)){
  float temp2 = 5.0f * analogRead(TEMP_PIN) * 100.0f / 1023.0f;
    ( (temp2 - 5.0f > temp1) || ((temp1+temp2)/2 > 15 ))?imprimirMsj("Agregue hielo",""):imprimirMsj("Temperatura","correcta");
    estado=PREVIO_FIN;
  }
  currentMillis=millis();
}
