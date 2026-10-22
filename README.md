# Sonar con Arduino UNO R4 Minima

## Descripción

Este proyecto utiliza un sensor ultrasónico HC-SR04 montado sobre un servomotor SG90 para explorar un área, detectar objetos cercanos y realizar un seguimiento aproximado de su dirección. Un botón permite encender o apagar el sistema y un LED RGB comunica el estado de la distancia.

El proyecto no utiliza pantalla. Los mensajes de diagnóstico y las distancias se muestran en el Monitor Serial del IDE de Arduino.

## Componentes

- Arduino UNO R4 Minima.
- Sensor ultrasónico HC-SR04.
- Servomotor SG90.
- Botón pulsador.
- LED RGB.
- Resistencias limitadoras adecuadas para el LED RGB, si el módulo no las incorpora.
- Cables Dupont y protoboard.
- Alimentación adecuada para el Arduino y los componentes.

## Conexiones

| Componente | Pin del Arduino |
|---|---|
| Botón, una terminal | D2 |
| Botón, otra terminal | GND |
| LED RGB, canal rojo | D3 |
| LED RGB, canal verde | D5 |
| Señal del servo SG90 | D6 |
| HC-SR04 TRIG | D9 |
| HC-SR04 ECHO | D10 |
| LED RGB, canal azul | D11 |
| GND del HC-SR04 | GND |
| GND del LED RGB | GND común, según el tipo de LED |
| GND del servo | GND común |
| VCC del HC-SR04 | Alimentación especificada para el módulo, normalmente 5 V |
| Alimentación del servo | Fuente adecuada según el servo |
| Alimentación del LED RGB | Mediante sus pines de control y resistencias adecuadas |

### Alimentación y seguridad

- Comprueba las especificaciones de cada módulo antes de conectarlo.
- No conectes una fuente de alimentación a un pin sin verificar primero el voltaje requerido.
- El servo puede consumir más corriente de la que conviene extraer del pin de alimentación del Arduino. Si usas una fuente externa para el servo, conecta su GND al GND del Arduino.
- No conectes directamente una salida de alimentación a un pin digital.
- Comprueba si el LED RGB es de ánodo común o de cátodo común. El código asume un LED RGB de cátodo común con los canales controlados directamente por D3, D5 y D11. Un LED de ánodo común necesita lógica invertida.
- Verifica la compatibilidad de niveles eléctricos entre el HC-SR04 y la placa. Si el pin ECHO entrega 5 V, consulta las especificaciones de la placa y utiliza adaptación de nivel si corresponde.

## Software necesario

1. Instala el IDE de Arduino.
2. Instala la librería `Servo` desde el gestor de bibliotecas si no está disponible en tu instalación.
3. Abre el archivo `sonar_sin_pantalla.ino`.
4. Selecciona la placa Arduino UNO R4 Minima y el puerto correcto.
5. Compila y carga el programa.
6. Abre el Monitor Serial a 9600 baudios.

No se requieren librerías de pantallas ni la librería `Wire`.

## Funcionamiento

### 1. Encendido y apagado

El sistema inicia apagado. Pulsa el botón conectado entre D2 y GND para alternar entre encendido y apagado. La entrada usa `INPUT_PULLUP`, por lo que el botón se lee como `LOW` al pulsarlo.

### 2. Búsqueda de objetos

Al encenderse, el servo barre entre 15 y 165 grados. En cada posición, el HC-SR04 mide la distancia. El sistema considera detectado un objeto cuando obtiene una lectura válida de hasta 80 cm.

### 3. Seguimiento aproximado

Cuando encuentra un objeto, el programa vuelve a medir en el ángulo actual. Si deja de detectarlo, mide en posiciones cercanas a la izquierda y a la derecha y mueve el servo hacia la dirección con la lectura más cercana. Si no encuentra el objeto, vuelve al barrido.

Este seguimiento es aproximado. Con un único sensor ultrasónico, el sistema no puede identificar de forma fiable un objeto entre varios ni seguir con precisión objetos que se mueven rápidamente.

### 4. Indicador LED RGB

Con la configuración de cátodo común del código:

- Azul: no hay una medición válida.
- Rojo: objeto a 25 cm o menos.
- Amarillo/naranja: objeto entre 26 y 50 cm.
- Verde: objeto a más de 50 cm.
- Apagado: radar apagado.

## Ajustes principales

Estos valores se encuentran al comienzo del código:

| Constante | Valor inicial | Función |
|---|---:|---|
| `ANGULO_MIN` | 15 | Límite izquierdo del servo |
| `ANGULO_MAX` | 165 | Límite derecho del servo |
| `DISTANCIA_DETECCION` | 80 cm | Distancia máxima para iniciar el seguimiento |
| `DISTANCIA_PERDIDA` | 100 cm | Alcance para buscar el objeto perdido a los lados |
| `PASO_ESCANEO` | 3 grados | Incremento del barrido |
| `PASO_SEGUIMIENTO` | 5 grados | Incremento del ajuste de seguimiento |

Si el servo golpea los topes mecánicos, reduce el rango angular. No fuerces el servomotor contra sus límites.

## Solución de problemas

### El servo no se mueve

- Verifica la señal en D6.
- Comprueba la alimentación y GND del servo.
- Asegúrate de que la fuente pueda entregar la corriente necesaria.
- Si usas una fuente externa, une su GND con el GND del Arduino.

### El sensor no mide correctamente

- Revisa TRIG en D9 y ECHO en D10.
- Comprueba VCC y GND del HC-SR04.
- Evita apuntar a superficies muy inclinadas, blandas o estrechas.
- Prueba con un objeto plano situado a una distancia conocida.

### El botón no responde

- Comprueba que esté conectado entre D2 y GND.
- No conectes el botón a 5 V cuando se utiliza `INPUT_PULLUP`.

### El LED RGB muestra colores incorrectos

- Verifica el pin común del LED.
- Comprueba si es de ánodo común o cátodo común.
- Confirma que cada canal tenga la resistencia adecuada.
- Si es de ánodo común, invierte los valores de salida en la función `rgb()`.

### El objeto se pierde con facilidad

- Prueba a aumentar `DISTANCIA_PERDIDA`.
- Reduce la velocidad del objeto durante las pruebas.
- Aumenta el tiempo de estabilización en `medirEnAngulo()`.
- Ten en cuenta que un único HC-SR04 ofrece seguimiento aproximado, no reconocimiento de objetos.

## Formato del Monitor Serial

Ejemplos de mensajes:

```text
SISTEMA SONAR LISTO
Pulsa el boton para encender o apagar.
RADAR ENCENDIDO
BUSCANDO | Angulo: 72 | Distancia: 43 cm
OBJETO DETECTADO: INICIANDO SEGUIMIENTO
SIGUIENDO | Angulo: 72 | Distancia: 41 cm
OBJETO PERDIDO: VOLVIENDO A BUSCAR
RADAR APAGADO
```

## Estructura del proyecto

```text
sonar_sin_pantalla/
├── sonar_sin_pantalla.ino
└── README.md
```

## Limitaciones

- No incluye pantalla.
- El seguimiento se basa únicamente en mediciones de distancia.
- Las superficies, la forma del objeto y las condiciones del entorno pueden afectar las lecturas ultrasónicas.
- El código no distingue de forma fiable entre varios objetos cercanos.
- El LED RGB requiere una conexión y una configuración eléctrica compatibles con su tipo.

## Licencia

Puedes adaptar este proyecto para fines educativos. Añade aquí la licencia que corresponda si lo publicas o distribuyes.
