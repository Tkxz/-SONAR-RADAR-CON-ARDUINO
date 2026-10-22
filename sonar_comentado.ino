
#include <Wire.h>                 // Permite la comunicacion I2C con la pantalla LCD.
#include <LiquidCrystal_I2C.h>    // Permite controlar la pantalla LCD mediante I2C.
#include <Servo.h>                // Permite controlar el servomotor.

// ---------- CONFIGURACION DE COMPONENTES ----------

LiquidCrystal_I2C lcd(0x27, 16, 2); // Crea una pantalla I2C en la direccion 0x27, de 16 columnas y 2 filas.
Servo servoRadar;                    // Crea el objeto que controlara el servomotor.

// ---------- PINES ----------

const int PIN_BOTON = 2;   // Define el pin 2 para el boton de encendido y apagado.

const int PIN_ROJO = 3;    // Define el pin 3 para el color rojo del LED RGB.
const int PIN_VERDE = 5;   // Define el pin 5 para el color verde del LED RGB.
const int PIN_SERVO = 6;   // Define el pin 6 para la senal del servomotor.

const int PIN_TRIG = 9;    // Define el pin 9 para enviar el pulso del sensor ultrasonico.
const int PIN_ECHO = 10;   // Define el pin 10 para recibir el eco del sensor.

const int PIN_AZUL = 11;   // Define el pin 11 para el color azul del LED RGB.

// ---------- PARAMETROS DEL RADAR ----------

const int ANGULO_MIN = 15;             // Establece el angulo minimo del barrido en 15 grados.
const int ANGULO_MAX = 165;            // Establece el angulo maximo del barrido en 165 grados.

const int DISTANCIA_DETECCION = 80;    // Define 80 cm como distancia maxima para detectar un objeto.
const int DISTANCIA_PERDIDA = 100;     // Permite buscar el objeto hasta 100 cm durante el seguimiento.

const int PASO_ESCANEO = 3;            // Define el avance del barrido automatico en pasos de 3 grados.
const int PASO_SEGUIMIENTO = 5;        // Define los movimientos de seguimiento en pasos de 5 grados.

// ---------- VARIABLES DE ESTADO ----------

bool radarEncendido = false;    // Indica si el radar esta encendido; comienza apagado.
bool siguiendoObjeto = false;   // Indica si el sistema esta intentando seguir un objeto.
bool botonAnterior = HIGH;      // Guarda el estado anterior del boton para detectar pulsaciones.

int anguloActual = 90;           // Guarda el angulo actual del servo; comienza en el centro.
int direccionEscaneo = 1;        // Indica la direccion del barrido: 1 avanza y -1 retrocede.
int distanciaActual = -1;        // Guarda la distancia medida; -1 representa una lectura invalida.

// ---------- CONTROL DEL LED RGB ----------

// Esta funcion recibe las intensidades de rojo, verde y azul.
void rgb(int r, int g, int b) {
  analogWrite(PIN_ROJO, r);    // Ajusta el brillo del canal rojo.
  analogWrite(PIN_VERDE, g);   // Ajusta el brillo del canal verde.
  analogWrite(PIN_AZUL, b);    // Ajusta el brillo del canal azul.
}

// Esta funcion apaga el LED RGB.
void apagarRGB() {
  rgb(0, 0, 0);                // Establece en cero la intensidad de los tres colores.
}

// Esta funcion selecciona un color segun la distancia medida.
void actualizarRGB(int distancia) {

  if (!radarEncendido) {       // Comprueba si el radar esta apagado.
    apagarRGB();               // Apaga todos los colores.
    return;                    // Termina la funcion sin ejecutar las condiciones siguientes.
  }

  if (distancia == -1) {       // Comprueba si la medicion no es valida.
    rgb(0, 0, 255);            // Enciende el color azul.
  }
  else if (distancia <= 25) {  // Comprueba si el objeto esta a 25 cm o menos.
    rgb(255, 0, 0);             // Enciende el color rojo.
  }
  else if (distancia <= 50) {  // Comprueba si la distancia esta entre 26 y 50 cm.
    rgb(255, 150, 0);           // Enciende rojo y verde para producir naranja.
  }
  else {                       // Se ejecuta cuando la distancia supera los 50 cm.
    rgb(0, 255, 0);             // Enciende el color verde.
  }
}

// ---------- SENSOR ULTRASONICO ----------

// Esta funcion mide la distancia y devuelve -1 si la lectura no es valida.
int medirDistancia() {

  digitalWrite(PIN_TRIG, LOW);     // Mantiene el pin TRIG apagado antes de iniciar la medicion.
  delayMicroseconds(2);            // Espera 2 microsegundos para estabilizar la senal.

  digitalWrite(PIN_TRIG, HIGH);    // Envia un pulso para activar el sensor.
  delayMicroseconds(10);           // Mantiene el pulso activo durante 10 microsegundos.
  digitalWrite(PIN_TRIG, LOW);     // Finaliza el pulso de activacion.

  unsigned long duracion =         // Declara una variable para almacenar el tiempo del eco.
    pulseIn(PIN_ECHO, HIGH, 30000); // Mide el eco hasta 30000 microsegundos como maximo.

  if (duracion == 0) {             // Comprueba si no se recibio un eco dentro del tiempo limite.
    return -1;                     // Devuelve -1 para indicar una medicion invalida.
  }

  int distancia =                  // Declara la variable que almacenara la distancia calculada.
    duracion * 0.0343 / 2.0;        // Convierte el tiempo en centimetros y divide por dos por el recorrido de ida y vuelta.

  if (distancia < 2 || distancia > 400) { // Comprueba si la distancia esta fuera del rango util establecido.
    return -1;                     // Devuelve -1 si la lectura no es valida.
  }

  return distancia;                // Devuelve la distancia medida en centimetros.
}

// ---------- PANTALLA DE ESPERA ----------

// Muestra el mensaje que aparece mientras el radar esta apagado.
void pantallaApagada() {

  lcd.clear();                     // Borra el contenido anterior de la pantalla.

  lcd.setCursor(1, 0);             // Coloca el cursor en la columna 1 de la primera fila.
  lcd.print("RADAR APAGADO");      // Muestra el mensaje de radar apagado.

  lcd.setCursor(0, 1);             // Coloca el cursor al comienzo de la segunda fila.
  lcd.print("PULSA EL BOTON");     // Indica al usuario como encender el sistema.
}

// ---------- INTERFAZ DEL RADAR ----------

// Recibe el angulo, la distancia y el estado de deteccion para actualizar la LCD.
void dibujarRadarLCD(int angulo, int distancia, bool objeto) {

  // No se borra toda la pantalla para reducir el parpadeo.

  lcd.setCursor(0, 0);             // Coloca el cursor al inicio de la primera fila.

  if (objeto) {                    // Comprueba si hay un objeto detectado.
    lcd.print("OBJ! ");            // Muestra una marca de deteccion.
  }
  else {                           // Se ejecuta cuando no hay un objeto detectado.
    lcd.print("RADAR");            // Muestra el nombre del sistema.
  }

  lcd.setCursor(5, 0);             // Coloca el cursor en la zona reservada para el barrido.
  lcd.print("           ");        // Borra los caracteres anteriores de esa zona.

  int posicion = map(              // Convierte el angulo del servo en una columna de la pantalla.
    angulo,                        // Valor de entrada: angulo actual.
    ANGULO_MIN,                    // Limite inferior del angulo.
    ANGULO_MAX,                    // Limite superior del angulo.
    5,                             // Columna minima de representacion.
    15                             // Columna maxima de representacion.
  );

  posicion = constrain(posicion, 5, 15); // Impide que la posicion salga de las columnas 5 a 15.
  lcd.setCursor(posicion, 0);      // Situa el cursor donde se representara el barrido.

  if (angulo < 70) {               // Comprueba si el servo apunta hacia el lado izquierdo.
    lcd.print("\\");               // Dibuja una barra inclinada hacia la izquierda.
  }
  else if (angulo > 110) {         // Comprueba si el servo apunta hacia el lado derecho.
    lcd.print("/");                // Dibuja una barra inclinada hacia la derecha.
  }
  else {                           // Se ejecuta cuando el servo apunta aproximadamente al centro.
    lcd.print("|");                // Dibuja una barra vertical.
  }

  if (objeto) {                    // Comprueba nuevamente si se detecto un objeto.
    lcd.setCursor(posicion, 0);    // Coloca el cursor en la posicion detectada.
    lcd.print("X");                // Representa el objeto mediante una X.
  }

  lcd.setCursor(0, 1);             // Coloca el cursor al comienzo de la segunda fila.
  lcd.print("A:");                 // Introduce la etiqueta del angulo.

  if (angulo < 100) {              // Comprueba si el angulo tiene menos de tres digitos.
    lcd.print("0");                // Agrega un cero para mantener un formato uniforme.
  }

  if (angulo < 10) {               // Comprueba si el angulo tiene un solo digito.
    lcd.print("0");                // Agrega otro cero para completar tres posiciones.
  }

  lcd.print(angulo);               // Muestra el valor numerico del angulo.
  lcd.print(" D:");                // Introduce la etiqueta de distancia.

  if (distancia == -1) {           // Comprueba si la distancia no es valida.
    lcd.print("---");              // Muestra guiones en lugar de una distancia.
  }
  else {                           // Se ejecuta si hay una distancia valida.

    if (distancia < 100) {         // Comprueba si la distancia tiene menos de tres digitos.
      lcd.print(" ");              // Agrega un espacio para alinear los valores.
    }

    if (distancia < 10) {          // Comprueba si la distancia tiene un solo digito.
      lcd.print(" ");              // Agrega otro espacio para mantener el formato.
    }

    lcd.print(distancia);          // Muestra la distancia en centimetros.
  }

  lcd.print("cm ");                // Agrega la unidad de medida y espacios de relleno.
}

// ---------- CONTROL DEL BOTON ----------

// Detecta una pulsacion y cambia el estado del radar.
void revisarBoton() {

  bool estado = digitalRead(PIN_BOTON); // Lee el estado actual del boton.

  if (estado == LOW && botonAnterior == HIGH) { // Detecta el paso de liberado a presionado.
    radarEncendido = !radarEncendido;            // Invierte el estado del radar.

    delay(200);                                  // Espera para reducir los rebotes del pulsador.

    if (radarEncendido) {                        // Comprueba si el radar acaba de encenderse.
      siguiendoObjeto = false;                   // Inicia el sistema en modo de escaneo.

      lcd.clear();                               // Limpia la pantalla.
      lcd.setCursor(1, 0);                       // Coloca el cursor en la primera fila.
      lcd.print("RADAR ONLINE");                 // Muestra que el radar esta activo.

      lcd.setCursor(2, 1);                       // Coloca el cursor en la segunda fila.
      lcd.print("BUSCANDO...");                  // Informa que comenzara la busqueda.

      rgb(0, 0, 255);                            // Enciende el LED azul como indicador inicial.
      delay(700);                                // Mantiene el mensaje visible durante 700 ms.
      lcd.clear();                               // Limpia la pantalla antes del escaneo.
    }
    else {                                       // Se ejecuta si el radar acaba de apagarse.
      siguiendoObjeto = false;                   // Cancela el seguimiento.
      anguloActual = 90;                         // Restablece el angulo central.

      servoRadar.write(90);                      // Ordena al servo volver a 90 grados.

      apagarRGB();                               // Apaga el LED RGB.
      pantallaApagada();                         // Muestra el mensaje de espera.
    }
  }

  botonAnterior = estado;                        // Guarda el estado del boton para la siguiente lectura.
}

// ---------- MEDICION EN UN ANGULO ----------

// Mueve el servo a un angulo y mide la distancia desde esa posicion.
int medirEnAngulo(int angulo) {

  angulo = constrain(angulo, ANGULO_MIN, ANGULO_MAX); // Limita el angulo al rango permitido.

  servoRadar.write(angulo);                      // Ordena al servo colocarse en el angulo indicado.
  delay(70);                                     // Espera a que el servo tenga tiempo para moverse.

  return medirDistancia();                       // Devuelve la distancia medida en esa posicion.
}

// ---------- ESCANEO AUTOMATICO ----------

// Barre el area y activa el seguimiento cuando encuentra un objeto.
void escanear() {

  servoRadar.write(anguloActual);                // Mueve el servo al angulo actual.
  delay(60);                                     // Espera brevemente antes de medir.

  distanciaActual = medirDistancia();            // Actualiza la distancia detectada.
  actualizarRGB(distanciaActual);               // Actualiza el color del LED segun la distancia.

  bool objeto =                                  // Crea una variable para representar la deteccion.
    distanciaActual > 0 &&                       // Exige una distancia positiva.
    distanciaActual <= DISTANCIA_DETECCION;      // Exige que el objeto este a 80 cm o menos.

  dibujarRadarLCD(anguloActual, distanciaActual, objeto); // Actualiza la informacion de la pantalla.

  if (objeto) {                                  // Comprueba si se detecto un objeto cercano.
    siguiendoObjeto = true;                      // Activa el modo de seguimiento.

    Serial.println("OBJETO DETECTADO");          // Envia un mensaje al monitor serie.
    return;                                      // Termina el escaneo actual sin avanzar el angulo.
  }

  anguloActual += PASO_ESCANEO * direccionEscaneo; // Avanza o retrocede segun la direccion del barrido.

  if (anguloActual >= ANGULO_MAX) {              // Comprueba si se alcanzo el extremo derecho.
    anguloActual = ANGULO_MAX;                   // Ajusta el angulo al limite maximo.
    direccionEscaneo = -1;                       // Invierte la direccion del barrido.
  }

  if (anguloActual <= ANGULO_MIN) {              // Comprueba si se alcanzo el extremo izquierdo.
    anguloActual = ANGULO_MIN;                   // Ajusta el angulo al limite minimo.
    direccionEscaneo = 1;                        // Invierte la direccion del barrido.
  }
}

// ---------- SEGUIMIENTO DEL OBJETO ----------

// Comprueba la posicion actual y busca el objeto a ambos lados.
void seguirObjeto() {

  int centro = medirEnAngulo(anguloActual);      // Mide nuevamente en el angulo donde se encontro el objeto.

  if (centro > 0 && centro <= DISTANCIA_DETECCION) { // Comprueba si sigue dentro de los 80 cm.
    distanciaActual = centro;                    // Actualiza la distancia del objeto.

    actualizarRGB(distanciaActual);              // Actualiza el color del LED.
    dibujarRadarLCD(anguloActual, distanciaActual, true); // Muestra el objeto en la pantalla.
    return;                                      // Mantiene el angulo y termina esta iteracion.
  }

  int anguloIzq = anguloActual - PASO_SEGUIMIENTO; // Calcula una posicion 5 grados a la izquierda.
  int anguloDer = anguloActual + PASO_SEGUIMIENTO; // Calcula una posicion 5 grados a la derecha.

  int distanciaIzq = medirEnAngulo(anguloIzq);   // Mide la distancia hacia la izquierda.
  int distanciaDer = medirEnAngulo(anguloDer);   // Mide la distancia hacia la derecha.

  bool izquierda =                               // Determina si se encontro el objeto a la izquierda.
    distanciaIzq > 0 &&                          // Comprueba que la lectura sea valida.
    distanciaIzq <= DISTANCIA_PERDIDA;           // Acepta distancias de hasta 100 cm.

  bool derecha =                                 // Determina si se encontro el objeto a la derecha.
    distanciaDer > 0 &&                          // Comprueba que la lectura sea valida.
    distanciaDer <= DISTANCIA_PERDIDA;           // Acepta distancias de hasta 100 cm.

  if (izquierda && !derecha) {                   // Comprueba si solo se detecta a la izquierda.
    anguloActual -= PASO_SEGUIMIENTO;            // Mueve el angulo 5 grados hacia la izquierda.
    distanciaActual = distanciaIzq;              // Guarda la distancia de ese lado.
  }
  else if (derecha && !izquierda) {              // Comprueba si solo se detecta a la derecha.
    anguloActual += PASO_SEGUIMIENTO;            // Mueve el angulo 5 grados hacia la derecha.
    distanciaActual = distanciaDer;              // Guarda la distancia de ese lado.
  }
  else if (izquierda && derecha) {               // Comprueba si ambos lados detectan un objeto.

    if (distanciaIzq < distanciaDer) {           // Compara las distancias de ambos lados.
      anguloActual -= PASO_SEGUIMIENTO;          // Elige el lado izquierdo si esta mas cerca.
      distanciaActual = distanciaIzq;            // Guarda la distancia izquierda.
    }
    else {                                       // Se ejecuta si la derecha es igual o mas cercana.
      anguloActual += PASO_SEGUIMIENTO;          // Elige el lado derecho.
      distanciaActual = distanciaDer;            // Guarda la distancia derecha.
    }
  }
  else {                                         // Se ejecuta si no se encuentra el objeto en ninguno de los lados.
    siguiendoObjeto = false;                     // Desactiva el modo de seguimiento.

    lcd.clear();                                 // Limpia la pantalla para mostrar el aviso.
    lcd.setCursor(0, 0);                         // Coloca el cursor en la primera fila.
    lcd.print("OBJETO PERDIDO");                 // Informa que el objeto ya no se localiza.

    lcd.setCursor(2, 1);                         // Coloca el cursor en la segunda fila.
    lcd.print("BUSCANDO...");                    // Indica que comenzara una nueva busqueda.

    delay(300);                                  // Mantiene el mensaje visible durante 300 ms.
    lcd.clear();                                 // Limpia el aviso antes de volver al escaneo.
    return;                                      // Termina el seguimiento actual.
  }

  anguloActual = constrain(anguloActual, ANGULO_MIN, ANGULO_MAX); // Impide que el angulo supere los limites.

  servoRadar.write(anguloActual);                // Mueve el servo hacia la nueva posicion seleccionada.

  actualizarRGB(distanciaActual);                // Actualiza el color segun la ultima distancia.
  dibujarRadarLCD(anguloActual, distanciaActual, true); // Muestra la posicion y la deteccion en la LCD.
}

// ---------- INICIALIZACION DEL SISTEMA ----------

// Configura los pines, el servo, la pantalla y el monitor serie.
void setup() {

  Serial.begin(9600);                            // Inicia la comunicacion serie a 9600 baudios.

  pinMode(PIN_BOTON, INPUT_PULLUP);              // Configura el boton con resistencia pull-up interna.
  pinMode(PIN_TRIG, OUTPUT);                     // Configura TRIG como salida.
  pinMode(PIN_ECHO, INPUT);                      // Configura ECHO como entrada.

  pinMode(PIN_ROJO, OUTPUT);                     // Configura el canal rojo como salida.
  pinMode(PIN_VERDE, OUTPUT);                    // Configura el canal verde como salida.
  pinMode(PIN_AZUL, OUTPUT);                     // Configura el canal azul como salida.

  servoRadar.attach(PIN_SERVO);                  // Asocia el servo al pin 6.
  servoRadar.write(90);                          // Coloca el servo en el centro.

  Wire.begin();                                  // Inicia la comunicacion I2C.
  lcd.init();                                    // Inicializa la pantalla LCD.
  lcd.backlight();                               // Enciende la iluminacion de la pantalla.

  apagarRGB();                                   // Asegura que el LED RGB comience apagado.

  lcd.clear();                                   // Limpia la pantalla.
  lcd.setCursor(1, 0);                           // Coloca el cursor en la primera fila.
  lcd.print("SONAR RADAR");                      // Muestra el nombre del proyecto.

  lcd.setCursor(2, 1);                           // Coloca el cursor en la segunda fila.
  lcd.print("INICIANDO...");                     // Muestra el mensaje de inicializacion.

  delay(1000);                                   // Espera un segundo para mostrar el mensaje.
  pantallaApagada();                             // Deja la pantalla en modo de espera.
}

// ---------- BUCLE PRINCIPAL ----------

// Arduino ejecuta esta funcion repetidamente mientras esta encendido.
void loop() {

  revisarBoton();                                // Comprueba si el usuario cambio el estado del radar.

  if (!radarEncendido) {                         // Comprueba si el sistema permanece apagado.
    delay(10);                                   // Espera brevemente para evitar iteraciones demasiado rapidas.
    return;                                      // Finaliza esta iteracion y espera la siguiente.
  }

  if (siguiendoObjeto) {                         // Comprueba si existe un objeto en seguimiento.
    seguirObjeto();                              // Ejecuta la logica para localizarlo.
  }
  else {                                         // Se ejecuta si no hay ningun objeto en seguimiento.
    escanear();                                  // Continua el barrido automatico.
  }
}
