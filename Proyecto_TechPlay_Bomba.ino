#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <U8x8lib.h>
#include <Adafruit_MCP23X17.h>

// ============================================================
//              PROYECTO TECHPLAY - BOMBA
// ============================================================
//
// SISTEMA 1: CONTRASEÑA
// SISTEMA 2: SIMON DE COLORES
// SISTEMA 3: CABLES ESTILO AMONG US
//
// Arduino UNO
//
// OLED GRANDE:
// SDA -> A4
// SCL -> A5
//
// OLED CHICA:
// SDA -> A2
// SCL -> A3
//
// KEYPAD:
// D2 - D9
//
// Los minijuegos adicionales utilizan un MCP23017
// para ampliar la cantidad de entradas/salidas.
//
// ============================================================


// ============================================================
// OLED GRANDE
// ============================================================

#define ANCHO 128
#define ALTO 64

Adafruit_SSD1306 pantalla(
  ANCHO,
  ALTO,
  &Wire,
  -1
);


// ============================================================
// OLED CHICA
// ============================================================

U8X8_SSD1306_128X32_UNIVISION_SW_I2C pantallaChica(
  A3,
  A2,
  U8X8_PIN_NONE
);


// ============================================================
// EXPANSOR MCP23017
// ============================================================

Adafruit_MCP23X17 mcp;


// ============================================================
// KEYPAD
// ============================================================

const byte FILAS = 4;
const byte COLUMNAS = 4;

char teclas[FILAS][COLUMNAS] = {

  {'1', '2', '3', 'A'},
  {'#', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '4', 'D'}

};

byte pinesFilas[FILAS] = {
  2,
  3,
  4,
  5
};

byte pinesColumnas[COLUMNAS] = {
  6,
  7,
  8,
  9
};

Keypad teclado = Keypad(
  makeKeymap(teclas),
  pinesFilas,
  pinesColumnas,
  FILAS,
  COLUMNAS
);


// ============================================================
// CONTRASEÑA
// ============================================================

char clave[4];

byte posicion = 0;

const char CLAVE_CORRECTA[4] = {
  '5',
  '6',
  '7',
  '8'
};


// ============================================================
// CONTROL GENERAL DE LOS SISTEMAS
// ============================================================

bool sistemaPassword = false;

bool sistemaSimon = false;

bool sistemaCables = false;

byte sistemasCompletados = 0;


// ============================================================
// OLED CHICA - SECUENCIA INICIAL
// ============================================================

const char* coloresInicio[5] = {

  "ROJO",
  "AZUL",
  "AMARILLO",
  "VERDE",
  "BLANCO"

};

byte colorActual = 0;

unsigned long ultimoCambioColor = 0;

const unsigned long TIEMPO_COLOR = 1500;

bool secuenciaTerminada = false;


// ============================================================
// SIMON
// ============================================================
//
// MCP23017:
//
// GPA0 = LED ROJO
// GPA1 = LED AZUL
// GPA2 = LED AMARILLO
// GPA3 = LED VERDE
//
// GPA4 = BOTON ROJO
// GPA5 = BOTON AZUL
// GPA6 = BOTON AMARILLO
// GPA7 = BOTON VERDE
//
// ============================================================

#define LED_ROJO_SIMON       0
#define LED_AZUL_SIMON       1
#define LED_AMARILLO_SIMON   2
#define LED_VERDE_SIMON      3

#define BTN_ROJO_SIMON       4
#define BTN_AZUL_SIMON       5
#define BTN_AMARILLO_SIMON   6
#define BTN_VERDE_SIMON      7


const byte ledsSimon[4] = {

  LED_ROJO_SIMON,
  LED_AZUL_SIMON,
  LED_AMARILLO_SIMON,
  LED_VERDE_SIMON

};


const byte botonesSimon[4] = {

  BTN_ROJO_SIMON,
  BTN_AZUL_SIMON,
  BTN_AMARILLO_SIMON,
  BTN_VERDE_SIMON

};


// ============================================================
// SECUENCIA SIMON
// ============================================================

const byte MAX_RONDAS = 6;

byte secuenciaSimon[MAX_RONDAS];

byte rondaSimon = 1;

byte posicionSimon = 0;

bool simonIniciado = false;


// ============================================================
// CABLES ESTILO AMONG US
// ============================================================
//
// GPB0 = CABLE ROJO
// GPB1 = CABLE AZUL
// GPB2 = CABLE AMARILLO
// GPB3 = CABLE VERDE
//
// Cuando cada cable conecta con su terminal correcto,
// la entrada queda en LOW.
//
// ============================================================

#define CABLE_ROJO       8
#define CABLE_AZUL       9
#define CABLE_AMARILLO   10
#define CABLE_VERDE      11


bool rojoConectado = false;

bool azulConectado = false;

bool amarilloConectado = false;

bool verdeConectado = false;


// ============================================================
// LEDS INDICADORES DE LOS CABLES
// ============================================================

#define LED_CABLE_ROJO       12
#define LED_CABLE_AZUL       13
#define LED_CABLE_AMARILLO   14
#define LED_CABLE_VERDE      15


// ============================================================
// SETUP
// ============================================================

void setup() {

  // ==========================================================
  // OLED GRANDE
  // ==========================================================

  Wire.begin();


  if (!pantalla.begin(
        SSD1306_SWITCHCAPVCC,
        0x3C
      )) {

    if (!pantalla.begin(
          SSD1306_SWITCHCAPVCC,
          0x3D
        )) {

      while (true);

    }

  }


  pantalla.setTextColor(
    SSD1306_WHITE
  );


  // ==========================================================
  // OLED CHICA
  // ==========================================================

  pantallaChica.begin();

  pantallaChica.setPowerSave(0);

  mostrarColorChica();

  ultimoCambioColor = millis();


  // ==========================================================
  // MCP23017
  // ==========================================================

  if (mcp.begin_I2C()) {

    configurarSimon();

    configurarCables();

  }


  // ==========================================================
  // GENERAR SECUENCIA SIMON
  // ==========================================================

  randomSeed(
    analogRead(A0)
  );


  generarSecuenciaSimon();


  // ==========================================================
  // CONTRASEÑA
  // ==========================================================

  borrarClave();

  pantallaInicial();

}


// ============================================================
// LOOP PRINCIPAL
// ============================================================

void loop() {

  actualizarOLEDChica();


  // ==========================================================
  // SISTEMA 1
  // ==========================================================

  if (!sistemaPassword) {

    actualizarPassword();

  }


  // ==========================================================
  // SISTEMA 2
  // ==========================================================

  if (
    sistemaPassword &&
    !sistemaSimon
  ) {

    ejecutarSimon();

  }


  // ==========================================================
  // SISTEMA 3
  // ==========================================================

  if (
    sistemaPassword &&
    sistemaSimon &&
    !sistemaCables
  ) {

    actualizarCables();

  }


  // ==========================================================
  // TODOS TERMINADOS
  // ==========================================================

  if (
    sistemaPassword &&
    sistemaSimon &&
    sistemaCables
  ) {

    bombaDesactivada();

  }

}


// ============================================================
// ============================================================
//                  SISTEMA DE CONTRASEÑA
// ============================================================
// ============================================================

void actualizarPassword() {

  char tecla = teclado.getKey();


  if (!tecla) {

    return;

  }


  // ==========================================================
  // CORRECCIONES DEL KEYPAD ORIGINAL
  // ==========================================================

  char teclaCorregida = tecla;


  if (tecla == 'B') {

    teclaCorregida = '3';

  }

  else if (tecla == 'C') {

    teclaCorregida = '2';

  }

  else if (tecla == 'D') {

    teclaCorregida = '1';

  }

  else if (tecla == '9') {

    teclaCorregida = '5';

  }

  else if (tecla == '5') {

    teclaCorregida = '9';

  }

  else if (tecla == '0') {

    teclaCorregida = '7';

  }

  else if (tecla == '7') {

    teclaCorregida = '0';

  }

  else if (tecla == 'A') {

    return;

  }


  tecla = teclaCorregida;


  // ==========================================================
  // BORRAR
  // ==========================================================

  if (tecla == '*') {

    borrarClave();


    pantalla.clearDisplay();

    pantalla.setTextSize(2);

    pantalla.setCursor(
      20,
      22
    );

    pantalla.print(
      "BORRADO"
    );

    pantalla.display();


    delay(700);


    pantallaInicial();


    return;

  }


  // ==========================================================
  // CONFIRMAR
  // ==========================================================

  if (tecla == '#') {


    if (posicion != 4) {


      pantalla.clearDisplay();


      pantalla.setTextSize(1);


      pantalla.setCursor(
        18,
        18
      );

      pantalla.print(
        "FALTAN NUMEROS"
      );


      pantalla.setCursor(
        22,
        36
      );

      pantalla.print(
        "DEBEN SER 4"
      );


      pantalla.display();


      delay(1200);


      mostrarClave();


      return;

    }


    comprobarPassword();


    return;

  }


  // ==========================================================
  // GUARDAR NUMEROS
  // ==========================================================

  if (
    tecla >= '0' &&
    tecla <= '9'
  ) {


    if (posicion < 4) {


      clave[posicion] = tecla;


      posicion++;


      mostrarClave();

    }

  }

}


// ============================================================
// COMPROBAR PASSWORD
// ============================================================

void comprobarPassword() {

  bool correcta = true;


  for (
    byte i = 0;
    i < 4;
    i++
  ) {

    if (
      clave[i] !=
      CLAVE_CORRECTA[i]
    ) {

      correcta = false;

    }

  }


  if (correcta) {


    pantalla.clearDisplay();


    pantalla.setTextSize(1);


    pantalla.setCursor(
      25,
      13
    );

    pantalla.print(
      "CONTRASENA"
    );


    pantalla.setTextSize(2);


    pantalla.setCursor(
      15,
      31
    );

    pantalla.print(
      "CORRECTA"
    );


    pantalla.display();


    delay(1500);


    sistemaPassword = true;


    sistemaCompletado();


    iniciarSimon();

  }


  else {


    pantalla.clearDisplay();


    pantalla.setTextSize(1);


    pantalla.setCursor(
      25,
      13
    );

    pantalla.print(
      "CONTRASENA"
    );


    pantalla.setTextSize(2);


    pantalla.setCursor(
      5,
      31
    );

    pantalla.print(
      "INCORRECTA"
    );


    pantalla.display();


    delay(1500);


    borrarClave();


    pantallaInicial();

  }

}


// ============================================================
// ============================================================
//                        SIMON
// ============================================================
// ============================================================

void configurarSimon() {


  for (
    byte i = 0;
    i < 4;
    i++
  ) {


    mcp.pinMode(
      ledsSimon[i],
      OUTPUT
    );


    mcp.digitalWrite(
      ledsSimon[i],
      LOW
    );


    mcp.pinMode(
      botonesSimon[i],
      INPUT_PULLUP
    );

  }

}


// ============================================================
// GENERAR SECUENCIA
// ============================================================

void generarSecuenciaSimon() {


  for (
    byte i = 0;
    i < MAX_RONDAS;
    i++
  ) {


    secuenciaSimon[i] =
      random(0, 4);

  }

}


// ============================================================
// INICIAR SIMON
// ============================================================

void iniciarSimon() {


  rondaSimon = 1;

  posicionSimon = 0;

  simonIniciado = true;


  pantalla.clearDisplay();


  pantalla.setTextSize(2);


  pantalla.setCursor(
    34,
    10
  );

  pantalla.print(
    "SIMON"
  );


  pantalla.setTextSize(1);


  pantalla.setCursor(
    15,
    40
  );

  pantalla.print(
    "REPITE COLORES"
  );


  pantalla.display();


  delay(1500);

}


// ============================================================
// EJECUTAR SIMON
// ============================================================

void ejecutarSimon() {


  if (!simonIniciado) {

    iniciarSimon();

  }


  mostrarSecuenciaSimon();


  bool correcta =
    leerSecuenciaSimon();


  if (correcta) {


    rondaSimon++;


    if (
      rondaSimon >
      MAX_RONDAS
    ) {


      sistemaSimon = true;


      sistemaCompletado();


      mostrarInicioCables();


      return;

    }


    delay(500);

  }


  else {


    errorSimon();


    rondaSimon = 1;


    delay(800);

  }

}


// ============================================================
// MOSTRAR SECUENCIA SIMON
// ============================================================

void mostrarSecuenciaSimon() {


  int velocidad =

    700 -
    (rondaSimon * 60);


  if (velocidad < 250) {

    velocidad = 250;

  }


  for (
    byte i = 0;
    i < rondaSimon;
    i++
  ) {


    byte color =
      secuenciaSimon[i];


    prenderColorSimon(
      color
    );


    delay(
      velocidad
    );


    apagarSimon();


    delay(200);

  }

}


// ============================================================
// PRENDER COLOR SIMON
// ============================================================

void prenderColorSimon(
  byte color
) {


  apagarSimon();


  mcp.digitalWrite(
    ledsSimon[color],
    HIGH
  );


  pantallaChica.clear();


  if (color == 0) {


    pantallaChica.setFont(
      u8x8_font_px437wyse700b_2x2_r
    );


    pantallaChica.drawString(
      2,
      0,
      "ROJO"
    );

  }


  else if (color == 1) {


    pantallaChica.setFont(
      u8x8_font_px437wyse700b_2x2_r
    );


    pantallaChica.drawString(
      2,
      0,
      "AZUL"
    );

  }


  else if (color == 2) {


    pantallaChica.setFont(
      u8x8_font_8x13B_1x2_r
    );


    pantallaChica.drawString(
      0,
      0,
      "AMARILLO"
    );

  }


  else if (color == 3) {


    pantallaChica.setFont(
      u8x8_font_px437wyse700b_2x2_r
    );


    pantallaChica.drawString(
      1,
      0,
      "VERDE"
    );

  }

}


// ============================================================
// APAGAR SIMON
// ============================================================

void apagarSimon() {


  for (
    byte i = 0;
    i < 4;
    i++
  ) {


    mcp.digitalWrite(
      ledsSimon[i],
      LOW
    );

  }


  pantallaChica.clear();

}


// ============================================================
// ESPERAR BOTON SIMON
// ============================================================

int esperarBotonSimon() {


  while (true) {


    for (
      byte i = 0;
      i < 4;
      i++
    ) {


      if (
        mcp.digitalRead(
          botonesSimon[i]
        ) == LOW
      ) {


        delay(30);


        if (
          mcp.digitalRead(
            botonesSimon[i]
          ) == LOW
        ) {


          while (
            mcp.digitalRead(
              botonesSimon[i]
            ) == LOW
          ) {

            delay(5);

          }


          return i;

        }

      }

    }

  }

}


// ============================================================
// LEER SECUENCIA JUGADOR
// ============================================================

bool leerSecuenciaSimon() {


  for (
    byte i = 0;
    i < rondaSimon;
    i++
  ) {


    int boton =
      esperarBotonSimon();


    prenderColorSimon(
      boton
    );


    delay(180);


    apagarSimon();


    if (
      boton !=
      secuenciaSimon[i]
    ) {


      return false;

    }

  }


  return true;

}


// ============================================================
// ERROR SIMON
// ============================================================

void errorSimon() {


  pantalla.clearDisplay();


  pantalla.setTextSize(2);


  pantalla.setCursor(
    32,
    22
  );


  pantalla.print(
    "ERROR"
  );


  pantalla.display();


  for (
    byte j = 0;
    j < 3;
    j++
  ) {


    for (
      byte i = 0;
      i < 4;
      i++
    ) {


      mcp.digitalWrite(
        ledsSimon[i],
        HIGH
      );

    }


    delay(150);


    apagarSimon();


    delay(150);

  }

}


// ============================================================
// ============================================================
//                 CABLES ESTILO AMONG US
// ============================================================
// ============================================================

void configurarCables() {


  mcp.pinMode(
    CABLE_ROJO,
    INPUT_PULLUP
  );


  mcp.pinMode(
    CABLE_AZUL,
    INPUT_PULLUP
  );


  mcp.pinMode(
    CABLE_AMARILLO,
    INPUT_PULLUP
  );


  mcp.pinMode(
    CABLE_VERDE,
    INPUT_PULLUP
  );


  mcp.pinMode(
    LED_CABLE_ROJO,
    OUTPUT
  );


  mcp.pinMode(
    LED_CABLE_AZUL,
    OUTPUT
  );


  mcp.pinMode(
    LED_CABLE_AMARILLO,
    OUTPUT
  );


  mcp.pinMode(
    LED_CABLE_VERDE,
    OUTPUT
  );


  apagarLedsCables();

}


// ============================================================
// MOSTRAR INICIO CABLES
// ============================================================

void mostrarInicioCables() {


  pantalla.clearDisplay();


  pantalla.setTextSize(1);


  pantalla.setCursor(
    15,
    12
  );


  pantalla.print(
    "CONECTA LOS"
  );


  pantalla.setTextSize(2);


  pantalla.setCursor(
    25,
    30
  );


  pantalla.print(
    "CABLES"
  );


  pantalla.display();

}


// ============================================================
// ACTUALIZAR CABLES
// ============================================================

void actualizarCables() {


  rojoConectado =

    mcp.digitalRead(
      CABLE_ROJO
    ) == LOW;


  azulConectado =

    mcp.digitalRead(
      CABLE_AZUL
    ) == LOW;


  amarilloConectado =

    mcp.digitalRead(
      CABLE_AMARILLO
    ) == LOW;


  verdeConectado =

    mcp.digitalRead(
      CABLE_VERDE
    ) == LOW;


  // ==========================================================
  // LED ROJO
  // ==========================================================

  mcp.digitalWrite(

    LED_CABLE_ROJO,

    rojoConectado ?
    HIGH :
    LOW

  );


  // ==========================================================
  // LED AZUL
  // ==========================================================

  mcp.digitalWrite(

    LED_CABLE_AZUL,

    azulConectado ?
    HIGH :
    LOW

  );


  // ==========================================================
  // LED AMARILLO
  // ==========================================================

  mcp.digitalWrite(

    LED_CABLE_AMARILLO,

    amarilloConectado ?
    HIGH :
    LOW

  );


  // ==========================================================
  // LED VERDE
  // ==========================================================

  mcp.digitalWrite(

    LED_CABLE_VERDE,

    verdeConectado ?
    HIGH :
    LOW

  );


  // ==========================================================
  // TODOS CORRECTOS
  // ==========================================================

  if (

    rojoConectado &&

    azulConectado &&

    amarilloConectado &&

    verdeConectado

  ) {


    delay(300);


    sistemaCables = true;


    sistemaCompletado();

  }

}


// ============================================================
// APAGAR LEDS CABLES
// ============================================================

void apagarLedsCables() {


  mcp.digitalWrite(
    LED_CABLE_ROJO,
    LOW
  );


  mcp.digitalWrite(
    LED_CABLE_AZUL,
    LOW
  );


  mcp.digitalWrite(
    LED_CABLE_AMARILLO,
    LOW
  );


  mcp.digitalWrite(
    LED_CABLE_VERDE,
    LOW
  );

}


// ============================================================
// SISTEMA COMPLETADO
// ============================================================

void sistemaCompletado() {


  sistemasCompletados++;


  pantalla.clearDisplay();


  pantalla.setTextSize(2);


  pantalla.setCursor(
    45,
    5
  );


  pantalla.print(
    "UN"
  );


  pantalla.setCursor(
    15,
    25
  );


  pantalla.print(
    "SISTEMA"
  );


  pantalla.setCursor(
    30,
    45
  );


  pantalla.print(
    "MENOS"
  );


  pantalla.display();


  delay(1800);

}


// ============================================================
// BOMBA DESACTIVADA
// ============================================================

void bombaDesactivada() {


  pantalla.clearDisplay();


  pantalla.setTextSize(1);


  pantalla.setCursor(
    42,
    8
  );


  pantalla.print(
    "BOMBA"
  );


  pantalla.setTextSize(2);


  pantalla.setCursor(
    2,
    28
  );


  pantalla.print(
    "DESACTIVADA"
  );


  pantalla.display();


  pantallaChica.clear();


  pantallaChica.setFont(
    u8x8_font_8x13B_1x2_r
  );


  pantallaChica.drawString(
    0,
    0,
    "COMPLETO"
  );


  while (true) {

    delay(1000);

  }

}


// ============================================================
// OLED CHICA - SECUENCIA INICIAL
// ============================================================

void actualizarOLEDChica() {


  if (secuenciaTerminada) {

    return;

  }


  if (

    millis() -
    ultimoCambioColor >=
    TIEMPO_COLOR

  ) {


    ultimoCambioColor =
      millis();


    if (colorActual >= 4) {


      pantallaChica.clear();


      secuenciaTerminada =
        true;


      return;

    }


    colorActual++;


    mostrarColorChica();

  }

}


// ============================================================
// MOSTRAR COLOR OLED CHICA
// ============================================================

void mostrarColorChica() {


  pantallaChica.clear();


  if (colorActual == 0) {


    pantallaChica.setFont(
      u8x8_font_px437wyse700b_2x2_r
    );


    pantallaChica.drawString(
      2,
      0,
      "ROJO"
    );

  }


  else if (colorActual == 1) {


    pantallaChica.setFont(
      u8x8_font_px437wyse700b_2x2_r
    );


    pantallaChica.drawString(
      2,
      0,
      "AZUL"
    );

  }


  else if (colorActual == 2) {


    pantallaChica.setFont(
      u8x8_font_8x13B_1x2_r
    );


    pantallaChica.drawString(
      0,
      0,
      "AMARILLO"
    );

  }


  else if (colorActual == 3) {


    pantallaChica.setFont(
      u8x8_font_px437wyse700b_2x2_r
    );


    pantallaChica.drawString(
      1,
      0,
      "VERDE"
    );

  }


  else if (colorActual == 4) {


    pantallaChica.setFont(
      u8x8_font_px437wyse700b_2x2_r
    );


    pantallaChica.drawString(
      0,
      0,
      "BLANCO"
    );

  }

}


// ============================================================
// BORRAR CONTRASEÑA
// ============================================================

void borrarClave() {


  clave[0] = 0;

  clave[1] = 0;

  clave[2] = 0;

  clave[3] = 0;


  posicion = 0;

}


// ============================================================
// PANTALLA INICIAL
// ============================================================

void pantallaInicial() {


  pantalla.clearDisplay();


  pantalla.setTextSize(1);


  pantalla.setCursor(
    25,
    5
  );


  pantalla.print(
    "ESCRIBE LA"
  );


  pantalla.setCursor(
    25,
    17
  );


  pantalla.print(
    "CONTRASENA"
  );


  pantalla.setTextSize(2);


  pantalla.setCursor(
    38,
    29
  );


  pantalla.print(
    "____"
  );


  pantalla.setTextSize(1);


  pantalla.setCursor(
    4,
    52
  );


  pantalla.print(
    "* BORRAR"
  );


  pantalla.setCursor(
    72,
    52
  );


  pantalla.print(
    "# OK"
  );


  pantalla.display();

}


// ============================================================
// MOSTRAR CONTRASEÑA
// ============================================================

void mostrarClave() {


  pantalla.clearDisplay();


  pantalla.setTextSize(1);


  pantalla.setCursor(
    25,
    6
  );


  pantalla.print(
    "CONTRASENA"
  );


  pantalla.setTextSize(2);


  pantalla.setCursor(
    38,
    25
  );


  for (
    byte i = 0;
    i < posicion;
    i++
  ) {


    pantalla.print(
      clave[i]
    );

  }


  pantalla.setTextSize(1);


  pantalla.setCursor(
    4,
    52
  );


  pantalla.print(
    "* BORRAR"
  );


  pantalla.setCursor(
    72,
    52
  );


  pantalla.print(
    "# OK"
  );


  pantalla.display();

}