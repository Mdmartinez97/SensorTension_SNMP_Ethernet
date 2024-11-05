/*----------------------SENSOR DE TENSIÓN IoT-----------------
*
*      * Medición de tensión alterna
*      * Conexión Ethernet
*      * Comunicación SNMP
*
* Desarrollado por Marcos Martinez y Nahuel Oggioni - Mayo 2024
*
* ver archivo de configuración "NetConfig.h"
*/

#include <SPI.h>
#include <EthernetUdp.h>
#include <EthernetENC.h>
#include <SNMP_Agent.h>

// Librerías Display
#include <OLEDDisplay.h>
#include <OLEDDisplayUi.h>
#include <SSD1306Wire.h>
#include <Wire.h>


#if ESP32
  #include <WiFi.h>
  #define CSpin 5 // Módulo Ethernet
  #define Vpin 34 // Módulo sensor de tensión
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #define CSpin 2 // Módulo Ethernet
  #define Vpin A0 // Módulo sensor de tensión
#endif

//Apagado display
const int botonPin = 0;
bool displayEncendido = true;
bool estadoBotonAnterior = HIGH;
unsigned long tiempoUltimoBoton = 0;
const unsigned long tiempoLimite = 30000; // 30 segundos


int eth_flag = 0;
int vol_ant;
int vol_act;
byte mac[6];

// Archivo local de configuración y funciones
#include "NetConfig.h"
#include "Funciones.h"

// Configuración de sensor de tensión
#include <ZMPT101B.h>
#define SENSITIVITY 500 // Sensibilidad
ZMPT101B voltageSensor(Vpin, 50); // (GPIO Pin, Frecuencia de red en Hz)

// Inicializa librería de cliente Ethernet
EthernetClient client;
EthernetUDP Udp;
EthernetServer server(80);

// Agente SNMP
SNMPAgent snmp = SNMPAgent("public", "private");

// Variable a medir
int voltage; // Tensión AC

// Frecuencia de impresión de datos en puerto serie
unsigned long tiempoAnterior = 0;
unsigned long intervalo = 1000; // 1 segundo

// Inicializar display para SSD1306
SSD1306Wire display(0x3C, 4, 5); // Dirección 0x3C, SDA=D2 (GPIO 4), SCL=D1 (GPIO 5)


void setup() {
  Serial.begin(115200);
  
  pinMode(botonPin, INPUT_PULLUP); // Configura el pin del botón con resistencia pull-up interna

  // Configura estación WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // Desconectarse, si estuviera conectado
  delay(100);
  
  // Obtener la dirección MAC de ESP32 WiFi
  WiFi.macAddress(mac); // Esta dirección MAC será asignada al módulo Ethernet ENC28J60

  // Inicializar la pantalla
  display.init();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.clear();

  // Sensor de tensión
  pinMode(Vpin, INPUT);
  voltageSensor.setSensitivity(SENSITIVITY);

  // Llamar función Ethernet
  IniciarEthernet(mac);

  // Iniciar SNMP
  Iniciar_SNMP();

  tiempoUltimoBoton = millis(); // Inicializa el tiempo de la última interacción

}

void loop() {

  //Llamado constante para no cerrar comunicaión Ethernet
  client.available();

  // Llamado constante para no cerrar comunicación SNMP
  snmp.loop();

  // Lectura de variables
  voltage = voltageSensor.getRmsVoltage();
  vol_act = voltage;

  // Verificar el estado de la conexión Ethernet
  if (Ethernet.linkStatus() == LinkOFF) {
      // Si el Ethernet está desconectado y el flag es 1
      if (eth_flag == 1) {
          encenderDisplay();
      }
      eth_flag = 0;  // Actualizar el flag a 0
  } else { // Si el Ethernet está conectado
      // Si el flag es 0 y no se ha procesado antes
      if (eth_flag == 0) {
          encenderDisplay();
          eth_flag = 1;  // Actualizar el flag a 1
          clearLine(0, 52, 128, 10);  // Limpiar la línea del display
      }
  }

  //Encender/Apagar Display

  bool estadoBoton = digitalRead(botonPin);

  // Detectar cambio de estado del botón (de no presionado a presionado)
  if (estadoBoton == LOW && estadoBotonAnterior == HIGH) {
    encenderDisplay();
  }
  // Verificar si han pasado 30 segundos desde la última interacción
  if (displayEncendido && (millis() - tiempoUltimoBoton >= tiempoLimite)) {
    displayEncendido = false;
    display.displayOff(); // Apagar el display
  }
  estadoBotonAnterior = estadoBoton;
  // Añadir un pequeño retardo para evitar rebotes del botón
  delay(50);
  //}

  ImprimirDatos();

}

void Iniciar_SNMP(){
  // Iniciar SNMP
  snmp.setUDP(&Udp);
  snmp.begin();

  // Direcciones OID
  snmp.addReadOnlyStaticStringHandler(".1.3.6.1.4.1.5.13", "Prueba"); // String estática
  snmp.addIntegerHandler(".1.3.6.1.4.1.5.12", &voltage);
}


void ImprimirDatos(){
  display.drawString(0, 15, "IP: "+ipToString(Ethernet.localIP()));
  display.drawString(0, 25, "MAC: "+macToString(mac));
  //Impresión de datos por puerto serie y display
  unsigned long tiempoActual = millis();
  if (tiempoActual - tiempoAnterior >= intervalo) {
    Serial.print(voltage);
    Serial.println(" VRMS");
    tiempoAnterior = tiempoActual;
      if (vol_act != vol_ant) {
        vol_ant = vol_act;
        clearLine(0, 40, 128, 10); // (x, y, ancho, alto)
        display.drawString(0, 40, "VRMS: " + String(voltage));
        display.display();
      }
  }
  if (eth_flag ==0){
    display.drawString(0, 52, "Ethernet no conectado");

  }
}

// Función para encender el display y reiniciar el temporizador
void encenderDisplay() {
    displayEncendido = true;  // Asegura que el display esté encendido
    tiempoUltimoBoton = millis();  // Reinicia el temporizador
    display.displayOn();  // Enciende el display
}
