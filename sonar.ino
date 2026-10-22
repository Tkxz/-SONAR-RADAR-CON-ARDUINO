#include <Servo.h> // Incluye la librería para controlar el servomotor.

// -------------------- PINES -------------------- // Separa visualmente la configuración de pines.
const int PIN_BOTON = 2; // Define D2 como entrada del botón, conectado también a GND.
const int PIN_ROJO = 3; // Define D3 para el canal rojo del LED RGB.
const int PIN_VERDE = 5; // Define D5 para el canal verde del LED RGB.
const int PIN_SERVO = 6; // Define D6 como señal de control del servo SG90.
const int PIN_TRIG = 9; // Define D9 como salida de disparo del HC-SR04.
const int PIN_ECHO = 10; // Define D10 como entrada del eco del HC-SR04.
const int PIN_AZUL = 11; // Define D11 para el canal azul del LED RGB.

// -------------------- CONFIGURACIÓN -------------------- // Separa los parámetros ajustables.
const int ANGULO_MIN = 15; // Establece el límite izquierdo del movimiento del servo.
const int ANGULO_MAX = 165; // Establece el límite derecho del movimiento del servo.
const int DISTANCIA_DETECCION = 80; // Define en centímetros la distancia máxima para detectar un objeto.
const int DISTANCIA_PERDIDA = 100; // Define el alcance usado al buscar un objeto que se perdió.
const int PASO_ESCANEO = 3; // Define cuántos grados avanza el servo en cada paso de búsqueda.
const int PASO_SEGUIMIENTO = 5; // Define cuántos grados prueba a cada lado al buscar el objeto.

// -------------------- ESTADO DEL SISTEMA -------------------- // Separa las variables de estado.
Servo servoRadar; // Crea el objeto que controla el servomotor.
bool radarEncendido = false; // Guarda si el radar está encendido.
bool siguiendoObjeto = false; // Guarda si el sistema está intentando seguir un objeto.
bool botonAnterior = HIGH; // Guarda la lectura anterior del botón para detectar pulsaciones.
unsigned long ultimoCambioBoton = 0; // Guarda el instante de la última pulsación aceptada.
const unsigned long ANTIRREBOTE_MS = 200; // Define el tiempo antirrebote del botón en milisegundos.
int anguloActual = 90; // Inicia el ángulo del servo en el centro.
int direccionEscaneo = 1; // Define que el primer barrido avance hacia la derecha.
int distanciaActual = -1; // Guarda la distancia medida; -1 significa que no hubo lectura válida.

// -------------------- CONTROL DEL LED RGB -------------------- // Agrupa las funciones del LED RGB.
void rgb(int rojo, int verde, int azul) { // Recibe la intensidad de cada canal del LED RGB.
  analogWrite(PIN_ROJO, rojo); // Ajusta el brillo del canal rojo.
  analogWrite(PIN_VERDE, verde); // Ajusta el brillo del canal verde.
  analogWrite(PIN_AZUL, azul); // Ajusta el brillo del canal azul.
} // Termina la función que ajusta los colores.

void apagarRGB() { // Define una función para apagar el LED RGB.
  rgb(0, 0, 0); // Establece en cero la intensidad de los tres canales.
} // Termina la función para apagar el LED.

void actualizarRGB(int distancia) { // Elige el color del LED según la distancia detectada.
  if (!radarEncendido) { // Comprueba si el sistema está apagado.
    apagarRGB(); // Apaga el LED cuando el radar está apagado.
    return; // Sale de la función sin ejecutar las siguientes condiciones.
  } // Termina la comprobación de apagado.

  if (distancia == -1) { // Comprueba si no se obtuvo una medición válida.
    rgb(0, 0, 255); // Muestra azul cuando no hay una lectura válida.
  } else if (distancia <= 25) { // Comprueba si el objeto está a 25 cm o menos.
    rgb(255, 0, 0); // Muestra rojo para un objeto muy cercano.
  } else if (distancia <= 50) { // Comprueba si el objeto está a 50 cm o menos.
    rgb(255, 150, 0); // Muestra amarillo o naranja para una distancia intermedia.
  } else { // Se ejecuta si el objeto está a más de 50 cm.
    rgb(0, 255, 0); // Muestra verde para una distancia mayor.
  } // Termina la selección de color.
} // Termina la función de actualización del LED.

// -------------------- MEDICIÓN DEL HC-SR04 -------------------- // Agrupa la función de medición.
int medirDistancia() { // Mide y devuelve la distancia aproximada en centímetros.
  digitalWrite(PIN_TRIG, LOW); // Asegura que TRIG comience en nivel bajo.
  delayMicroseconds(2); // Espera dos microsegundos antes del pulso.
  digitalWrite(PIN_TRIG, HIGH); // Inicia el pulso ultrasónico.
  delayMicroseconds(10); // Mantiene TRIG alto durante diez microsegundos.
  digitalWrite(PIN_TRIG, LOW); // Finaliza el pulso de disparo.

  unsigned long duracion = pulseIn(PIN_ECHO, HIGH, 30000); // Mide la duración del eco con un límite de 30 ms.
  if (duracion == 0) { // Comprueba si no se recibió un eco dentro del tiempo límite.
    return -1; // Devuelve -1 para indicar una medición no válida.
  } // Termina la comprobación de eco ausente.

  int distancia = duracion * 0.0343 / 2.0; // Convierte el tiempo del eco a centímetros.
  if (distancia < 2 || distancia > 400) { // Comprueba si la distancia queda fuera del rango típico del sensor.
    return -1; // Devuelve -1 si la medición está fuera del rango aceptado.
  } // Termina la validación de distancia.
  return distancia; // Devuelve la distancia válida medida.
} // Termina la función de medición.

// -------------------- BOTÓN DE ENCENDIDO/APAGADO -------------------- // Agrupa el manejo del botón.
void revisarBoton() { // Lee el botón y cambia el estado del radar cuando se pulsa.
  bool estadoActual = digitalRead(PIN_BOTON); // Lee el nivel actual del botón.
  if (estadoActual == LOW && botonAnterior == HIGH && millis() - ultimoCambioBoton >= ANTIRREBOTE_MS) { // Detecta una nueva pulsación estable.
    radarEncendido = !radarEncendido; // Invierte el estado de encendido del radar.
    ultimoCambioBoton = millis(); // Guarda el instante de la pulsación aceptada.

    if (radarEncendido) { // Comprueba si el botón acaba de encender el radar.
      siguiendoObjeto = false; // Inicia el sistema en modo de búsqueda.
      anguloActual = 90; // Coloca el ángulo lógico en el centro.
      direccionEscaneo = 1; // Configura el siguiente barrido hacia la derecha.
      Serial.println("RADAR ENCENDIDO"); // Informa por el monitor serial que el radar se encendió.
    } else { // Se ejecuta cuando el botón apaga el radar.
      siguiendoObjeto = false; // Cancela el seguimiento actual.
      anguloActual = 90; // Restablece el ángulo lógico al centro.
      servoRadar.write(90); // Ordena al servo regresar al centro.
      apagarRGB(); // Apaga el LED RGB.
      Serial.println("RADAR APAGADO"); // Informa por el monitor serial que el radar se apagó.
    } // Termina la selección de encendido o apagado.
  } // Termina la detección de pulsación.
  botonAnterior = estadoActual; // Guarda el estado actual para la próxima lectura.
} // Termina la función del botón.

// -------------------- MEDIR EN UN ÁNGULO -------------------- // Agrupa el movimiento y la medición.
int medirEnAngulo(int angulo) { // Apunta el sensor a un ángulo y devuelve la distancia medida.
  angulo = constrain(angulo, ANGULO_MIN, ANGULO_MAX); // Limita el ángulo para evitar superar los topes definidos.
  servoRadar.write(angulo); // Mueve el servo al ángulo solicitado.
  delay(100); // Espera a que el servo se estabilice antes de medir.
  return medirDistancia(); // Mide y devuelve la distancia desde esa posición.
} // Termina la función de medición por ángulo.

// -------------------- BÚSQUEDA AUTOMÁTICA -------------------- // Agrupa el barrido de búsqueda.
void escanear() { // Mueve el sensor de lado a lado buscando objetos cercanos.
  servoRadar.write(anguloActual); // Mueve el servo a la posición de barrido actual.
  delay(60); // Espera brevemente para estabilizar la lectura.
  distanciaActual = medirDistancia(); // Mide la distancia en el ángulo actual.
  actualizarRGB(distanciaActual); // Actualiza el LED según la distancia medida.

  Serial.print("BUSCANDO | Angulo: "); // Imprime el estado de búsqueda y una etiqueta del ángulo.
  Serial.print(anguloActual); // Imprime el ángulo actual.
  Serial.print(" | Distancia: "); // Imprime una etiqueta para la distancia.
  if (distanciaActual == -1) { // Comprueba si no hubo una lectura válida.
    Serial.println("Sin lectura"); // Informa que no se detectó un eco válido.
  } else { // Se ejecuta si la medición fue válida.
    Serial.print(distanciaActual); // Imprime la distancia en centímetros.
    Serial.println(" cm"); // Añade la unidad y termina la línea.
  } // Termina la impresión de la distancia.

  if (distanciaActual > 0 && distanciaActual <= DISTANCIA_DETECCION) { // Comprueba si hay un objeto dentro del alcance de detección.
    siguiendoObjeto = true; // Cambia el sistema al modo de seguimiento.
    Serial.println("OBJETO DETECTADO: INICIANDO SEGUIMIENTO"); // Informa que se encontró un objeto.
    return; // Sale de la función para comenzar el seguimiento en el siguiente ciclo.
  } // Termina la detección de objetos durante el barrido.

  anguloActual += PASO_ESCANEO * direccionEscaneo; // Avanza el ángulo según la dirección del barrido.
  if (anguloActual >= ANGULO_MAX) { // Comprueba si se alcanzó el límite derecho.
    anguloActual = ANGULO_MAX; // Mantiene el ángulo dentro del límite derecho.
    direccionEscaneo = -1; // Invierte la dirección para barrer hacia la izquierda.
  } // Termina la comprobación del límite derecho.
  if (anguloActual <= ANGULO_MIN) { // Comprueba si se alcanzó el límite izquierdo.
    anguloActual = ANGULO_MIN; // Mantiene el ángulo dentro del límite izquierdo.
    direccionEscaneo = 1; // Invierte la dirección para barrer hacia la derecha.
  } // Termina la comprobación del límite izquierdo.
} // Termina la función de búsqueda.

// -------------------- SEGUIMIENTO APROXIMADO -------------------- // Agrupa la lógica para seguir un objeto.
void seguirObjeto() { // Comprueba el centro y los lados para intentar mantener el objeto en el sensor.
  int distanciaCentro = medirEnAngulo(anguloActual); // Mide de nuevo en la posición actual.

  if (distanciaCentro > 0 && distanciaCentro <= DISTANCIA_DETECCION) { // Comprueba si el objeto sigue delante.
    distanciaActual = distanciaCentro; // Actualiza la distancia del objeto.
    actualizarRGB(distanciaActual); // Actualiza el LED con la nueva distancia.
    Serial.print("SIGUIENDO | Angulo: "); // Imprime el estado de seguimiento y la etiqueta del ángulo.
    Serial.print(anguloActual); // Imprime el ángulo actual.
    Serial.print(" | Distancia: "); // Imprime la etiqueta de distancia.
    Serial.print(distanciaActual); // Imprime la distancia actual.
    Serial.println(" cm"); // Añade la unidad y termina la línea.
    return; // Mantiene el ángulo actual mientras el objeto siga delante.
  } // Termina la comprobación del centro.

  int anguloIzq = anguloActual - PASO_SEGUIMIENTO; // Calcula el ángulo de prueba a la izquierda.
  int distanciaIzq = medirEnAngulo(anguloIzq); // Mueve el sensor a la izquierda y mide.
  int anguloDer = anguloActual + PASO_SEGUIMIENTO; // Calcula el ángulo de prueba a la derecha.
  int distanciaDer = medirEnAngulo(anguloDer); // Mueve el sensor a la derecha y mide.

  bool objetoIzq = distanciaIzq > 0 && distanciaIzq <= DISTANCIA_PERDIDA; // Indica si hay un objeto detectable a la izquierda.
  bool objetoDer = distanciaDer > 0 && distanciaDer <= DISTANCIA_PERDIDA; // Indica si hay un objeto detectable a la derecha.

  if (objetoIzq && !objetoDer) { // Comprueba si solo se detecta un objeto a la izquierda.
    anguloActual -= PASO_SEGUIMIENTO; // Gira el ángulo actual hacia la izquierda.
    distanciaActual = distanciaIzq; // Guarda la distancia medida a la izquierda.
  } else if (objetoDer && !objetoIzq) { // Comprueba si solo se detecta un objeto a la derecha.
    anguloActual += PASO_SEGUIMIENTO; // Gira el ángulo actual hacia la derecha.
    distanciaActual = distanciaDer; // Guarda la distancia medida a la derecha.
  } else if (objetoIzq && objetoDer) { // Comprueba si ambos lados detectan objetos.
    if (distanciaIzq < distanciaDer) { // Comprueba si el objeto de la izquierda está más cerca.
      anguloActual -= PASO_SEGUIMIENTO; // Gira hacia la izquierda.
      distanciaActual = distanciaIzq; // Guarda la distancia izquierda.
    } else { // Se ejecuta si el objeto de la derecha está más cerca o igual de cerca.
      anguloActual += PASO_SEGUIMIENTO; // Gira hacia la derecha.
      distanciaActual = distanciaDer; // Guarda la distancia derecha.
    } // Termina la comparación de distancias laterales.
  } else { // Se ejecuta si ninguno de los lados encuentra un objeto dentro del alcance.
    siguiendoObjeto = false; // Cancela el seguimiento porque el objeto se perdió.
    actualizarRGB(-1); // Indica mediante el LED que no hay una lectura válida.
    Serial.println("OBJETO PERDIDO: VOLVIENDO A BUSCAR"); // Informa que se reanuda el barrido.
    return; // Sale para volver al modo de búsqueda.
  } // Termina la selección de dirección.

  anguloActual = constrain(anguloActual, ANGULO_MIN, ANGULO_MAX); // Evita que el ángulo exceda los límites permitidos.
  servoRadar.write(anguloActual); // Mueve el servo hacia la nueva dirección calculada.
  actualizarRGB(distanciaActual); // Actualiza el LED según la distancia elegida.
  Serial.print("AJUSTANDO SEGUIMIENTO | Angulo: "); // Informa que el ángulo se está ajustando.
  Serial.print(anguloActual); // Imprime el nuevo ángulo.
  Serial.print(" | Distancia: "); // Imprime la etiqueta de distancia.
  Serial.print(distanciaActual); // Imprime la distancia lateral seleccionada.
  Serial.println(" cm"); // Añade la unidad y termina la línea.
} // Termina la función de seguimiento.

// -------------------- INICIALIZACIÓN -------------------- // Agrupa la configuración inicial del Arduino.
void setup() { // Se ejecuta una vez al iniciar o reiniciar el Arduino.
  Serial.begin(9600); // Inicia la comunicación serial a 9600 baudios.
  pinMode(PIN_BOTON, INPUT_PULLUP); // Configura el botón con resistencia pull-up interna.
  pinMode(PIN_TRIG, OUTPUT); // Configura TRIG como salida digital.
  pinMode(PIN_ECHO, INPUT); // Configura ECHO como entrada digital.
  pinMode(PIN_ROJO, OUTPUT); // Configura el canal rojo del RGB como salida.
  pinMode(PIN_VERDE, OUTPUT); // Configura el canal verde del RGB como salida.
  pinMode(PIN_AZUL, OUTPUT); // Configura el canal azul del RGB como salida.
  digitalWrite(PIN_TRIG, LOW); // Mantiene el disparador ultrasónico apagado al iniciar.
  servoRadar.attach(PIN_SERVO); // Vincula el objeto Servo al pin D6.
  servoRadar.write(90); // Coloca el servo en el centro al iniciar.
  apagarRGB(); // Asegura que el LED RGB comience apagado.
  Serial.println("SISTEMA SONAR LISTO"); // Informa que la configuración inicial terminó.
  Serial.println("Pulsa el boton para encender o apagar."); // Explica cómo controlar el sistema.
} // Termina la configuración inicial.

// -------------------- BUCLE PRINCIPAL -------------------- // Agrupa la lógica que se repite continuamente.
void loop() { // Se repite mientras el Arduino esté encendido.
  revisarBoton(); // Revisa si el usuario pulsó el botón.
  if (!radarEncendido) { // Comprueba si el radar debe permanecer apagado.
    delay(10); // Espera brevemente para reducir lecturas innecesarias del botón.
    return; // Finaliza este ciclo sin mover el servo ni medir.
  } // Termina la comprobación del estado de encendido.
  if (siguiendoObjeto) { // Comprueba si el sistema tiene un objeto en seguimiento.
    seguirObjeto(); // Intenta mantener el sensor apuntando al objeto.
  } else { // Se ejecuta cuando no hay ningún objeto en seguimiento.
    escanear(); // Barre el área para encontrar un objeto.
  } // Termina la selección entre búsqueda y seguimiento.
} // Termina el bucle principal; Arduino vuelve a ejecutarlo automáticamente.
