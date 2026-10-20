# Sonar / Radar con Arduino

Radar que barre con un servo y un sensor ultrasónico, dibuja el resultado en una pantalla OLED y señaliza la distancia con un LED RGB. Un botón lo enciende y apaga. Al detectar un objeto a 60 cm o menos, el servo se detiene apuntándole hasta que desaparece.

## Componentes

| Cant. | Componente | Notas |
|---|---|---|
| 1 | Arduino Uno o Nano | Los pines PWM usados (3, 5, 6, 9, 11) coinciden con Uno/Nano |
| 1 | Sensor ultrasónico HC-SR04 | Funciona a 5 V |
| 1 | Servo SG90 o similar | Mueve el sensor |
| 1 | Pantalla OLED SSD1306 128x64, I2C | Dirección 0x3C |
| 1 | LED RGB cátodo común | Si es ánodo común, ver "Ajustes" |
| 3 | Resistencias 220 Ω | Una por color del LED |
| 1 | Pulsador | |
| 1 | Protoboard y cables dupont | |
| 1 | Cable USB | Programación y alimentación |

Opcional: fuente externa de 5 V para el servo (recomendado si el servo vibra o la placa se reinicia).

## Conexiones

| Componente | Pin del componente | Pin Arduino |
|---|---|---|
| Pulsador | Un terminal | D2 |
| Pulsador | Otro terminal | GND |
| LED RGB | Rojo (con 220 Ω) | D3 |
| LED RGB | Verde (con 220 Ω) | D5 |
| LED RGB | Azul (con 220 Ω) | D11 |
| LED RGB | Cátodo común | GND |
| Servo | Señal | D6 |
| Servo | VCC | 5V |
| Servo | GND | GND |
| HC-SR04 | TRIG | D9 |
| HC-SR04 | ECHO | D10 |
| HC-SR04 | VCC | 5V |
| HC-SR04 | GND | GND |
| OLED | SDA | A4 |
| OLED | SCL | A5 |
| OLED | VCC | 5V o 3.3V (según tu módulo) |
| OLED | GND | GND |

Notas:
- Todos los GND deben compartirse (y el de la fuente externa del servo, si la usas).
- El botón usa `INPUT_PULLUP`, por lo que no necesita resistencia externa.
- Monta el HC-SR04 sobre el brazo del servo para que gire con él.
- En Mega los pines I2C son 20 (SDA) y 21 (SCL).

## Software

1. Instala el [Arduino IDE](https://www.arduino.cc/en/software).
2. Abre **Herramientas > Administrar bibliotecas** e instala:
   - `Adafruit GFX Library`
   - `Adafruit SSD1306`
   - (`Wire`, `Servo` y `math.h` ya vienen incluidas)
3. Si el IDE pregunta por dependencias de Adafruit, acepta instalarlas todas (`Adafruit BusIO`).

## Pasos

1. Monta el circuito según la tabla de conexiones.
2. Conecta el Arduino al PC por USB.
3. Abre `sonar_comentado.ino` en el Arduino IDE.
4. Selecciona la placa en **Herramientas > Placa** (por ejemplo, Arduino Uno) y el puerto en **Herramientas > Puerto**.
5. Pulsa **Subir**.
6. Abre el **Monitor Serie** a **9600 baudios**. Debe mostrar `SONAR LISTO`.
7. La OLED mostrará "SONAR ARDUINO - APAGADO". Pulsa el botón para encender el radar.
8. Prueba poniendo un objeto frente al sensor.

## Funcionamiento

- **Barrido:** el servo se mueve entre 15° y 165° en pasos de 3°.
- **LED RGB:**
  - Azul: radar recién encendido.
  - Verde: libre (sin objeto o a más de 60 cm).
  - Amarillo: objeto entre 26 y 60 cm.
  - Rojo: objeto a 25 cm o menos.
- **Pantalla:** muestra ángulo (A), distancia (D), arcos de referencia, línea de barrido y un punto en la posición del objeto (hasta 100 cm).
- **Objetivo:** a 60 cm o menos el radar se detiene, muestra "OBJETIVO!" y reanuda el barrido cuando el objeto desaparece.
- **Botón:** alterna encendido y apagado. Al apagar, el servo vuelve a 90°.

## Ajustes en el código

| Constante | Función |
|---|---|
| `ANGULO_MIN` / `ANGULO_MAX` | Límites del barrido |
| `PASO` | Grados por medición (menor = más resolución, más lento) |
| `VELOCIDAD_SERVO` | Pausa en ms tras cada movimiento (menor = más rápido) |
| `DISTANCIA_OBJETIVO` | Distancia en cm a la que se detiene |
| `OLED_ADDR` | Dirección I2C de la pantalla (0x3C o 0x3D) |

Si tu LED RGB es de **ánodo común**, conecta el común a 5V e invierte los valores en `rgb()` (por ejemplo, `analogWrite(PIN_ROJO, 255 - r)`).

## Solución de problemas

| Problema | Posible causa y solución |
|---|---|
| Monitor Serie: `ERROR: OLED no detectada` | Revisa SDA/SCL y alimentación. Prueba la dirección `0x3D`. Usa un escáner I2C para confirmarla. |
| El servo tiembla o el Arduino se reinicia | Alimenta el servo con fuente externa de 5 V y une los GND. |
| Distancia siempre `--` o `Sin objeto` | Revisa TRIG/ECHO y la alimentación a 5 V del HC-SR04. |
| El botón no responde o responde tarde | Comprueba que va a GND. Mantén pulsado un instante, ya que el código lee el botón entre mediciones. |
| Colores del LED invertidos | Es de ánodo común (ver "Ajustes en el código"). |
| El radar se ve al revés | Intercambia `180, 0` por `0, 180` en el `map()` de `dibujarRadar()`. |
