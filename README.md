# Sonar Radar con Arduino

## 1. Descripción del proyecto

Este proyecto consiste en un sistema de detección y seguimiento de objetos mediante Arduino. Utiliza un sensor ultrasónico HC-SR04, un servomotor, una pantalla LCD I2C y un LED RGB.

El sistema realiza un barrido angular, mide distancias, detecta objetos cercanos e intenta seguirlos mediante pequeños movimientos del servomotor.

## 2. Objetivos

- Detectar objetos mediante ondas ultrasónicas.
- Medir distancias en centímetros.
- Controlar la orientación de un servomotor.
- Mostrar información en una pantalla LCD.
- Indicar visualmente la distancia detectada mediante colores.
- Implementar un botón para encender y apagar el sistema.

## 3. Características principales

- Escaneo automático entre 15° y 165°.
- Detección de objetos hasta 80 cm.
- Intento de recuperación del objeto hasta 100 cm durante el seguimiento.
- Seguimiento mediante movimientos de 5°.
- Visualización del ángulo y la distancia.
- Indicador luminoso RGB.
- Control mediante pulsador.
- Mensajes de detección por el monitor serie.

## 4. Componentes necesarios

| Componente | Cantidad |
|---|---:|
| Placa Arduino compatible | 1 |
| Sensor ultrasónico HC-SR04 | 1 |
| Servomotor | 1 |
| Pantalla LCD de 16 × 2 con I2C | 1 |
| LED RGB | 1 |
| Pulsador | 1 |
| Resistencias para el LED RGB | Según el montaje |
| Protoboard y cables | Según necesidad |

## 5. Conexiones

| Componente | Pin |
|---|---|
| Pulsador | D2 |
| LED RGB rojo | D3 |
| LED RGB verde | D5 |
| Servomotor | D6 |
| HC-SR04 TRIG | D9 |
| HC-SR04 ECHO | D10 |
| LED RGB azul | D11 |
| LCD SDA | SDA de Arduino |
| LCD SCL | SCL de Arduino |

Conecta la alimentación y la tierra de cada componente respetando sus especificaciones. Si el servomotor necesita una fuente externa, conecta también su GND al GND de Arduino.

## 6. Indicador de colores

| Color | Estado |
|---|---|
| Azul | Sin medición válida |
| Rojo | Objeto a 25 cm o menos |
| Naranja | Objeto entre 26 y 50 cm |
| Verde | Distancia superior a 50 cm |
| Apagado | Radar desactivado |

## 7. Funcionamiento

1. Al encender Arduino, la pantalla muestra el mensaje de inicialización.
2. El sistema espera hasta que se pulse el botón.
3. El servomotor comienza a explorar el área.
4. El sensor ultrasónico mide la distancia en cada posición.
5. Si encuentra un objeto a 80 cm o menos, comienza el seguimiento.
6. El sistema comprueba posiciones cercanas para intentar localizar nuevamente el objeto.
7. Si no encuentra el objeto dentro del rango de recuperación, vuelve al escaneo automático.
8. Al pulsar nuevamente el botón, el radar se apaga y el servomotor regresa a 90°.

## 8. Instalación

1. Instala Arduino IDE desde https://www.arduino.cc/en/software.
2. Instala las bibliotecas `LiquidCrystal_I2C` y `Servo`.
3. Conecta los componentes según la tabla de conexiones.
4. Abre el archivo `SonarRadar.ino`.
5. Selecciona la placa y el puerto correctos.
6. Compila y carga el programa.
7. Abre el monitor serie a 9600 baudios.

La dirección I2C de la pantalla está configurada en `0x27`. Si el módulo utiliza otra dirección, modifica ese valor en el código.

## 9. Estructura del proyecto

```text
Sonar-Radar/
├── SonarRadar.ino
└── README.md
```

## 10. Consideraciones técnicas

- Las mediciones inválidas se representan con el valor `-1`.
- El sensor puede presentar errores según la superficie, el tamaño y la orientación del objeto.
- El seguimiento es aproximado y no garantiza mantener objetos en movimiento rápido.
- El LED RGB debe ser compatible con el tipo de conexión y la configuración utilizada.
- La alimentación del servomotor debe soportar su consumo eléctrico.

## 11. Tecnologías utilizadas

- Arduino
- C/C++ para microcontroladores
- Sensor ultrasónico HC-SR04
- Servomotor
- Pantalla LCD con comunicación I2C
- LED RGB

## 12. Resultado esperado

Un radar interactivo que explora un área, mide distancias, muestra información en una pantalla LCD y utiliza señales luminosas para representar el estado de detección.
