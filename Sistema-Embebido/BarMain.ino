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
#define DOUT A1
#define CLK  A0
#define LED_PIN    6
#define LED_COUNT 16
#define BOTON_1_PIN 2
#define BOTON_2_PIN 3
#define ESPERANDO_ORDEN 0
#define PREPARANDO_TRAGO 1
#define CALIBRANDO 2
#define NO_SELECCIONADO -1
#define VASO_1 0
#define VASO_2 1
#define TRUE 1
#define FALSE 0

//Structs
typedef struct {
  char nombre[30];
  float porcentaje;
} t_ingrediente;

typedef struct {
  char nombre[30];
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
int encendido = 0;
float valorAnt=0.0;
float porcentaje_orden = 0.0;
int estado = 0;
int vaso=-1;
t_bebida bebidas[6];
char blue_buffer[3];
//SETUP
void setup() {
  Serial.begin(9600);
  inicioDisplay();
  lcd.print("Inicializando");
  inicioBluetooth();
  inicioBalanza();
  inicioNeoPixel();
  inicioBebidas();
  //Inicio Botones
  pinMode(BOTON_1_PIN, INPUT);
  digitalWrite(BOTON_1_PIN, HIGH);
  lcd.clear();
  lcd.print("Iniciado");
}

void loop() {
  if ( estado == CALIBRANDO){
    if( peso_vaso[vaso] == 0.0f){
      if( (vaso == VASO_1)?digitalRead(BOTON_1_PIN):digitalRead(BOTON_2_PIN)){
         peso_vaso[vaso] = balanza.get_value(10) / escala;
         lcd.clear();
         lcd.print("Coloque el vaso");
         lcd.setCursor(0,1);
         lcd.print(" lleno de Agua");
         delay(1000);
      }
      //se puede preguntar por si presiono el otro boton para cancelar
    } else {
      if((vaso == VASO_1)?digitalRead(BOTON_1_PIN):digitalRead(BOTON_2_PIN)){
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
          lcd.print("Elija un trago!");
        }
        estado = ESPERANDO_ORDEN;
        Serial.println(total_vaso[0]);
      }
    }
  }else if( estado == ESPERANDO_ORDEN ){ 
    int i = 0;
    if(blue.available() && vaso != NO_SELECCIONADO){
      while(blue.available()){
        blue_buffer[i] = blue.read();
        Serial.println(blue_buffer);
        i++;
        delay(50);
      }
      blue.flush();
      objetivo = total_vaso[vaso] * (atoi(blue_buffer))/100;
      lcd.clear();
      lcd.print(objetivo);
      if(vaso == NO_SELECCIONADO){
        //AVISAR A LA APP
      } else {
        //SELECCIONAMOS LA BEBIDA Y PASAMOS A PREPARARLO
        estado = PREPARANDO_TRAGO;
        estado = 1;
      }
    } else {
      if( digitalRead(BOTON_1_PIN ) == HIGH && digitalRead(BOTON_2_PIN) == LOW ){ //SELECCIONO EL VASO 1
        vaso = VASO_1;
        if(total_vaso[vaso] == 0.0f){
          estado = CALIBRANDO;
          lcd.clear();
          lcd.print("Coloque el vaso");
          lcd.setCursor(0,1);
          lcd.print(" 1 vacio");
          delay(1000);
        }
      }
    }
    //Serial.println(estado);
  } else if (estado == PREPARANDO_TRAGO ){
    //Serial.println("Preparando trago");
    int valor = balanza.get_value(10)/escala - peso_vaso[vaso];
    
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

void inicioBebidas(){
  t_bebida bebida;
  t_ingrediente ingr;
  //SUCAS LECCHI
  strcpy(bebida.nombre,"Sucas Lecchi\0");
  ingr = crearIngrediente( "Agua\0", 1.0f);
  bebida.ingredientes[0]= ingr;
  bebidas[0] = bebida;
  //Fernet con Coca
  strcpy(bebida.nombre,"Fernet con Coca\0");
  ingr = crearIngrediente ("Fernet\0", 0.3f);
  bebida.ingredientes[0]= ingr;
  ingr = crearIngrediente ("Coca\0", 0.7f);
  bebida.ingredientes[1]= ingr;
  bebidas[1] = bebida;
  //Destornillador
  strcpy(bebida.nombre,"Destornillador\0");
  ingr = crearIngrediente ("Vodka\0", 0.3f);
  bebida.ingredientes[0]= ingr;
  ingr = crearIngrediente ("Jugo de Naranja\0", 0.7f);
  bebida.ingredientes[1]= ingr;
  bebidas[2] = bebida;
  //Gancia con Sprite
  strcpy(bebida.nombre,"Gancia con Sprite\0");
  ingr = crearIngrediente ("Gancia\0", 0.2f);
  bebida.ingredientes[0]= ingr;
  ingr = crearIngrediente ("Sprite\0", 0.8f);
  bebida.ingredientes[1]= ingr;
  bebidas[3] = bebida;
  //Cuba Libre
  strcpy(bebida.nombre,"Cuba Libre\0");
  ingr = crearIngrediente ("Ron Dorado\0", 0.22f);
  bebida.ingredientes[0]= ingr;
  ingr = crearIngrediente ("Coca Cola\0", 0.73f);
  bebida.ingredientes[1]= ingr;
  ingr = crearIngrediente ("Jugo de Lima\0", 0.05f);
  bebida.ingredientes[2]= ingr;
  bebidas[4] = bebida;
  //Banana Mama
  strcpy(bebida.nombre,"Banana Mama\0");
  ingr = crearIngrediente ("Licor de Banana\0", 0.16f);
  bebida.ingredientes[0]= ingr;
  ingr = crearIngrediente ("Ron Dorado\0", 0.08f);
  bebida.ingredientes[1]= ingr;
  ingr = crearIngrediente ("Ron Blanco\0", 0.25f);
  bebida.ingredientes[2]= ingr;
  ingr = crearIngrediente ("Crema de Coco\0", 0.17f);
  bebida.ingredientes[3]= ingr;
  ingr = crearIngrediente ("Jugo de Anana\0", 0.34f);
  bebida.ingredientes[4]= ingr;
  bebidas[5] = bebida;

  
  
}

t_ingrediente crearIngrediente(char* nombre, float porcentaje){
  t_ingrediente ingr;
  strcpy(ingr.nombre,nombre);
  ingr.porcentaje = porcentaje;
  return ingr;
}


 
