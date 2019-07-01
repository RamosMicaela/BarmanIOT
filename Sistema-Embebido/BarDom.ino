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
#define LED_PIN    6
#define LED_COUNT 16
#define BOTON_1_PIN 2
#define BOTON_2_PIN 3
//ESTADOS
#define ESPERANDO_ORDEN 0
//#define PREPARANDO_TRAGO 1
//#define CALIBRANDO 2
#define RECIBIENDO_TRAGO 1
#define SELECCION_VASO 2
#define CALIBRANDO 3
#define PREPARANDO_TRAGO 4
#define CONTROLANDO_INGREDIENTE 5
//
#define NO_SELECCIONADO -1
#define VASO_1 0
#define VASO_2 1
#define TRUE 1
#define FALSE 0
#define SUCAS_LECCHI 0
#define FERNET_CON_COCA 1
#define DESTORNILLADOR 2
#define GANCIA_CON_SPRITE 3
#define CUBA_LIBRE 4
#define BANANA_MAMA 5
#define CANCELAR_TRAGO 6

//Structs
typedef struct {
  char nombre[17];
  int porcentaje;
} t_ingrediente;

typedef struct {
  char nombre[17];
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
float valorAnt=0.0;
float porcentaje_orden = 0.0;
int estado=ESPERANDO_ORDEN;
int vaso=NO_SELECCIONADO;
t_bebida bebida_actual;
int ingrediente_actual;
//t_bebida bebidas[6];
char blue_buffer[3];

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
  lcd.clear();
  lcd.print("Iniciado");
}

void loop() {
  if ( estado == ESPERANDO_ORDEN ) {
    Serial.println("Esperando orden");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Esperando orden"); // guardar para que pueda escribirlo antes de volver a borrar
    if(blue.available())
      estado=RECIBIENDO_TRAGO;
  } else if ( estado == RECIBIENDO_TRAGO ) {
    Serial.println("Recibiendo Trago");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Trago");
    lcd.setCursor(0,1);
    lcd.print("seleccionado!");
    //Revisar si llega 0 o 1 o 2
    //si llega 0 es una bebida
    //si llega 1 pide calibrar
    //si llega 2 pide cancelar
    //Recibir info bebida
    for(int i = 0; i < 16; i++){
     bebida_actual.nombre[i] = blue.read(); 
    }
    int j = 0;
    while(blue.available()){
      for(int i = 0; i<16; i++){
        bebida_actual.ingredientes[j].nombre[i] = blue.read();
      }
      for(int i = 0; i < 3; i++){
        blue_buffer[i] = blue.read();
      }
      bebida_actual.ingredientes[j].porcentaje = atoi(blue_buffer);
      j++;
    }
    bebida_actual.cantidad = j;
    estado = SELECCION_VASO;
  } else if ( estado == SELECCION_VASO ){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Seleccione su vaso");
    if( digitalRead(BOTON_1_PIN ) == HIGH && digitalRead(BOTON_2_PIN) == LOW ){ //SELECCIONO EL VASO 1
      vaso=VASO_1;    
    } else if (digitalRead(BOTON_2_PIN ) == HIGH && digitalRead(BOTON_1_PIN) == LOW) { //SELECCIONO EL VASO 2
      vaso=VASO_2;  
    }
    if( vaso != NO_SELECCIONADO){
      (peso_vaso[vaso] == 0.0f)?estado=CALIBRANDO:estado=PREPARANDO_TRAGO;
    }
  } else if ( estado == PREPARANDO_TRAGO ) {
    if(ingrediente_actual<bebida_actual.cantidad){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Agregue:");
      lcd.print(bebida_actual.ingredientes[ingrediente_actual].nombre);
      objetivo=total_vaso[vaso]*bebida_actual.ingredientes[ingrediente_actual].porcentaje/100;
      estado=CONTROLANDO_INGREDIENTE; 
    } else {
      objetivo=0.0;
      ya_cargado=0.0;
      ingrediente_actual=0;
      //bebida_actual=NULL;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Trago Finalizado!");
      estado=ESPERANDO_ORDEN;
    }
  } else if ( estado == CALIBRANDO ) {    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Coloque el vaso");
    lcd.setCursor(0,1);
    lcd.print("vacio");
    delay(1000); 
    // hay que esperar a que ponga el vaso y pedir por que pulse el boton antes de setear el peso del vaso vacio
    peso_vaso[vaso] = balanza.get_value(10) / escala;
    lcd.setCursor(0,1);
    lcd.print("lleno de Agua");
    delay(1000);
    float llenoAgua = (balanza.get_value(10) / escala);
    if(llenoAgua<= peso_vaso[vaso]){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("ERROR al calibrar");
      lcd.setCursor(0,1);
      lcd.print("Vuelva a intentarlo");
      peso_vaso[vaso]=0.0;
    }else{
      total_vaso[vaso]= llenoAgua-peso_vaso[vaso]; 
      estado= PREPARANDO_TRAGO;
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
