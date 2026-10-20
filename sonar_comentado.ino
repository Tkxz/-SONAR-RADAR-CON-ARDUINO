/*
  SONAR / RADAR CON ARDUINO
  --------------------------------------------------------------
  - Un servo gira un sensor ultrasónico (HC-SR04) entre 15° y 165°.
  - Una pantalla OLED SSD1306 (I2C) dibuja el radar en tiempo real.
  - Un LED RGB indica la distancia: verde = libre, amarillo = cerca,
    rojo = muy cerca, azul = radar recién encendido.
  - Un botón enciende/apaga el radar.
  - Si detecta un objeto a <= DISTANCIA_OBJETIVO cm, el servo se
    detiene apuntando a él hasta que el objeto desaparezca.
*/

#include <Wire.h>             // Comunicación I2C (para la OLED)
#include <Servo.h>            // Control del servomotor
#include <Adafruit_GFX.h>     // Primitivas gráficas (líneas, círculos, texto)
#include <Adafruit_SSD1306.h> // Driver de la pantalla OLED SSD1306
#include <math.h>             // cos(), sin()

// ==================== OLED ====================

#define SCREEN_WIDTH 128   // Ancho de la pantalla en píxeles
#define SCREEN_HEIGHT 64   // Alto de la pantalla en píxeles
#define OLED_RESET -1      // -1 = sin pin de reset dedicado (comparte el de Arduino)
#define OLED_ADDR 0x3C     // Dirección I2C típica de la SSD1306

// Objeto de la pantalla, usando el bus I2C (Wire)
Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// ==================== SERVO ====================

Servo servoRadar;  // Servo que hace girar el sensor

// ==================== PINES ====================

const int PIN_BOTON = 2;   // Botón on/off (con pull-up interno)

const int PIN_ROJO = 3;    // LED RGB - rojo (PWM)
const int PIN_VERDE = 5;   // LED RGB - verde (PWM)
const int PIN_SERVO = 6;   // Señal del servo

const int PIN_TRIG = 9;    // Sensor ultrasónico: disparo
const int PIN_ECHO = 10;   // Sensor ultrasónico: eco

const int PIN_AZUL = 11;   // LED RGB - azul (PWM)


// ==================== CONFIGURACIÓN ====================

const int ANGULO_MIN = 15;    // Límite izquierdo del barrido (grados)
const int ANGULO_MAX = 165;   // Límite derecho del barrido (grados)

const int PASO = 3;           // Grados que avanza el servo en cada medición

// Pausa tras mover el servo (ms): menor valor = barrido más rápido
const int VELOCIDAD_SERVO = 25;

// Distancia (cm) a la que se considera "objetivo" y el radar se detiene
const int DISTANCIA_OBJETIVO = 60;


// ==================== ESTADO ====================

bool radarEncendido = false;     // true = radar funcionando
bool objetivoDetectado = false;  // true = servo detenido sobre un objetivo

bool estadoBotonAnterior = HIGH; // Último estado leído del botón (HIGH = suelto)

unsigned long ultimoRebote = 0;  // Instante de la última pulsación válida

const unsigned long TIEMPO_REBOTE = 200; // Anti-rebote (ms)


// ==================== RGB ====================

// Fija el color del LED RGB (valores 0-255 por canal)
void rgb(int r, int g, int b) {

  analogWrite(PIN_ROJO, r);
  analogWrite(PIN_VERDE, g);
  analogWrite(PIN_AZUL, b);
}

// Apaga el LED
void apagarRGB() {

  rgb(0, 0, 0);
}


// ==================== SENSOR ULTRASÓNICO ====================

// Devuelve la distancia en cm, o -1 si no hay lectura válida
int medirDistancia() {

  // Pulso limpio de 10 µs en TRIG para iniciar la medición
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);

  digitalWrite(PIN_TRIG, LOW);

  // Duración del pulso de eco en µs (timeout 30 ms ≈ 5 m)
  unsigned long duracion =
    pulseIn(PIN_ECHO, HIGH, 30000);

  // 0 = no llegó eco dentro del timeout
  if (duracion == 0) {
    return -1;
  }

  // Distancia = tiempo * velocidad del sonido (0.0343 cm/µs) / 2 (ida y vuelta)
  int distancia =
    duracion * 0.0343 / 2.0;

  // Fuera del rango fiable del sensor (~400 cm)
  if (distancia > 400) {
    return -1;
  }

  return distancia;
}


// ==================== CONTROL RGB ====================

// Cambia el color del LED según la distancia medida
void actualizarRGB(int distancia) {

  // Radar apagado: LED apagado
  if (!radarEncendido) {
    apagarRGB();
    return;
  }

  if (distancia == -1 || distancia > 60) {

    // Verde = libre (sin objeto o lejos)
    rgb(0, 255, 0);
  }
  else if (distancia > 25) {

    // Amarillo/naranja = objeto cerca (26-60 cm)
    rgb(255, 150, 0);
  }
  else {

    // Rojo = objeto muy cerca (<= 25 cm)
    rgb(255, 0, 0);
  }
}


// ==================== PANTALLA APAGADA ====================

// Pantalla de espera cuando el radar está apagado
void pantallaApagada() {

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(27, 10);
  display.println("SONAR ARDUINO");

  display.setTextSize(2);
  display.setCursor(20, 27);
  display.println("APAGADO");

  display.setTextSize(1);
  display.setCursor(13, 52);
  display.println("Pulsa el boton");

  display.display(); // Envía el búfer a la pantalla
}


// ==================== RADAR OLED ====================

// Dibuja el radar completo: datos, arcos, línea de barrido y objetivo
void dibujarRadar(int angulo, int distancia) {

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // ----- Datos numéricos (arriba) -----
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("A:");        // Ángulo actual
  display.print(angulo);

  display.setCursor(72, 0);
  display.print("D:");        // Distancia actual

  if (distancia == -1) {
    display.print("--");      // Sin lectura
  }
  else {
    display.print(distancia);
    display.print("cm");
  }


  // ----- Centro del radar (parte inferior central de la pantalla) -----
  const int cx = 64;
  const int cy = 63;


  // ----- Arcos de referencia (semicírculos concéntricos) -----
  display.drawCircle(cx, cy, 15, SSD1306_WHITE);
  display.drawCircle(cx, cy, 30, SSD1306_WHITE);
  display.drawCircle(cx, cy, 45, SSD1306_WHITE);

  // Tapa la mitad inferior de los círculos para dejar solo semicírculos
  display.fillRect(
    0,
    cy + 1,
    SCREEN_WIDTH,
    SCREEN_HEIGHT - cy,
    SSD1306_BLACK
  );


  // ----- Línea base horizontal -----
  display.drawLine(
    15,
    cy,
    113,
    cy,
    SSD1306_WHITE
  );


  // ----- Líneas guía (90°, y diagonales izquierda y derecha) -----
  display.drawLine(
    cx,
    cy,
    cx,
    18,
    SSD1306_WHITE
  );

  display.drawLine(
    cx,
    cy,
    32,
    31,
    SSD1306_WHITE
  );

  display.drawLine(
    cx,
    cy,
    96,
    31,
    SSD1306_WHITE
  );


  // ----- Línea de barrido -----
  // Convierte el ángulo del servo (15-165) al ángulo de pantalla (180-0)
  float anguloPantalla =
    map(
      angulo,
      ANGULO_MIN,
      ANGULO_MAX,
      180,
      0
    );

  // Grados a radianes
  float rad =
    anguloPantalla * PI / 180.0;

  // Extremo de la línea (radio 45 px) con trigonometría
  int x =
    cx + cos(rad) * 45;

  int y =
    cy - sin(rad) * 45;

  display.drawLine(
    cx,
    cy,
    x,
    y,
    SSD1306_WHITE
  );


  // ----- Punto del objetivo -----
  // Solo se dibuja si hay objeto dentro de 100 cm
  if (distancia > 0 && distancia <= 100) {

    // Escala 0-100 cm a un radio de 5-45 px
    int radioObjeto =
      map(
        distancia,
        0,
        100,
        5,
        45
      );

    // Posición del punto sobre la línea de barrido
    int px =
      cx + cos(rad) * radioObjeto;

    int py =
      cy - sin(rad) * radioObjeto;

    display.fillCircle(
      px,
      py,
      3,
      SSD1306_WHITE
    );
  }


  // ----- Aviso de objetivo fijado -----
  if (objetivoDetectado) {

    display.setTextSize(1);
    display.setCursor(2, 10);
    display.println("OBJETIVO!");
  }

  display.display(); // Muestra todo lo dibujado
}


// ==================== BOTÓN ====================

// Lee el botón y alterna el estado del radar (con anti-rebote)
void revisarBoton() {

  bool estadoActual =
    digitalRead(PIN_BOTON);

  // Detecta flanco de bajada (pulsación) respetando el anti-rebote
  if (
    estadoActual == LOW &&
    estadoBotonAnterior == HIGH &&
    millis() - ultimoRebote > TIEMPO_REBOTE
  ) {

    radarEncendido = !radarEncendido; // Alterna encendido/apagado
    ultimoRebote = millis();

    if (radarEncendido) {

      Serial.println("RADAR ENCENDIDO");

      objetivoDetectado = false;

      rgb(0, 0, 255); // Azul = arrancando

      delay(200);
    }
    else {

      Serial.println("RADAR APAGADO");

      objetivoDetectado = false;

      servoRadar.write(90); // Servo al centro

      apagarRGB();

      pantallaApagada();
    }
  }

  // Guarda el estado para detectar el siguiente flanco
  estadoBotonAnterior = estadoActual;
}


// ==================== MOVIMIENTO ====================

// Mueve el servo a un ángulo, mide, actualiza LED/pantalla y detecta objetivos
void moverRadar(int angulo) {

  revisarBoton(); // Permite apagar el radar en pleno barrido

  if (!radarEncendido) {
    return;
  }

  servoRadar.write(angulo);

  delay(VELOCIDAD_SERVO); // Tiempo para que el servo llegue a la posición

  int distancia =
    medirDistancia();

  actualizarRGB(distancia);

  dibujarRadar(
    angulo,
    distancia
  );


  // ----- Detener al detectar un objetivo -----
  if (
    distancia > 0 &&
    distancia <= DISTANCIA_OBJETIVO
  ) {

    objetivoDetectado = true;

    servoRadar.write(angulo); // Mantiene la posición

    rgb(255, 0, 0);

    // Reporte por el monitor serie
    Serial.println();
    Serial.println("====================");
    Serial.println(" OBJETIVO DETECTADO");
    Serial.println("====================");

    Serial.print("Angulo: ");
    Serial.print(angulo);

    Serial.print(" | Distancia: ");
    Serial.print(distancia);

    Serial.println(" cm");

    // Redibuja para mostrar el cartel "OBJETIVO!"
    dibujarRadar(
      angulo,
      distancia
    );

    return;
  }


  // ----- Información normal por serie -----
  Serial.print("Angulo: ");
  Serial.print(angulo);

  Serial.print(" | Distancia: ");

  if (distancia == -1) {
    Serial.println("Sin objeto");
  }
  else {
    Serial.print(distancia);
    Serial.println(" cm");
  }
}


// ==================== INICIO ====================

void setup() {

  Serial.begin(9600);

  // ----- Configuración de pines -----
  pinMode(PIN_BOTON, INPUT_PULLUP); // Suelto = HIGH, pulsado = LOW

  pinMode(PIN_ROJO, OUTPUT);
  pinMode(PIN_VERDE, OUTPUT);
  pinMode(PIN_AZUL, OUTPUT);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);


  // ----- Servo -----
  servoRadar.attach(PIN_SERVO);
  servoRadar.write(90); // Posición central inicial

  apagarRGB();


  // ----- OLED -----
  Wire.begin();

  if (
    !display.begin(
      SSD1306_SWITCHCAPVCC, // Genera el voltaje de la pantalla internamente
      OLED_ADDR
    )
  ) {

    Serial.println(
      "ERROR: OLED no detectada"
    );

    // Si no hay pantalla, el programa se queda bloqueado aquí
    while (true) {
      delay(100);
    }
  }


  // ----- Pantalla de arranque -----
  display.clearDisplay();

  display.setTextColor(
    SSD1306_WHITE
  );

  display.setTextSize(1);

  display.setCursor(22, 18);
  display.println("SONAR INICIANDO");

  display.setCursor(37, 35);
  display.println("ESPERE...");

  display.display();

  delay(1000);

  pantallaApagada(); // Queda en espera hasta pulsar el botón

  Serial.println("SONAR LISTO");
}


// ==================== LOOP PRINCIPAL ====================

void loop() {

  revisarBoton();

  // Radar apagado: no hace nada más
  if (!radarEncendido) {

    delay(10);
    return;
  }


  // ----- Modo "objetivo fijado": servo detenido, solo vigila -----
  if (objetivoDetectado) {

    int distancia =
      medirDistancia();

    actualizarRGB(distancia);


    // El objetivo sigue presente: permanece detenido
    if (
      distancia > 0 &&
      distancia <= DISTANCIA_OBJETIVO
    ) {

      rgb(255, 0, 0);

      delay(50);

      return;
    }


    // El objetivo desapareció: reanuda el barrido
    objetivoDetectado = false;

    Serial.println(
      "OBJETIVO PERDIDO - REANUDANDO"
    );

    rgb(0, 255, 0);

    delay(100);
  }


  // ----- Barrido izquierda -> derecha -----
  for (
    int angulo = ANGULO_MIN;
    angulo <= ANGULO_MAX;
    angulo += PASO
  ) {

    // Sale del barrido si se apagó el radar o se fijó un objetivo
    if (
      !radarEncendido ||
      objetivoDetectado
    ) {
      break;
    }

    moverRadar(angulo);
  }


  // ----- Barrido derecha -> izquierda -----
  for (
    int angulo = ANGULO_MAX;
    angulo >= ANGULO_MIN;
    angulo -= PASO
  ) {

    if (
      !radarEncendido ||
      objetivoDetectado
    ) {
      break;
    }

    moverRadar(angulo);
  }
}
