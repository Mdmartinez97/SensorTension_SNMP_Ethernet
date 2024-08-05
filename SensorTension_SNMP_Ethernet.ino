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
  
  // Configura estación WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // Desconectarse, si estuviera conectado
  delay(100);
  
  // Obtener la dirección MAC de ESP32 WiFi
  WiFi.macAddress(mac); // Esta dirección MAC será asignada al módulo Ethernet ENC28J60

  // Inicializar la pantalla
  display.init();
  display.flipScreenVertically();
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

}

void loop() {

  //Llamado constante para no cerrar comunicaión Ethernet
  client.available();

  // Llamado constante para no cerrar comunicación SNMP
  snmp.loop();

  // Lectura de variables
  voltage = voltageSensor.getRmsVoltage();
  vol_act = voltage;

  if (Ethernet.linkStatus() == LinkOFF) {
    display.clear();
    Encabezado();
    display.drawString(0, 15, "Cable Ethernet no conectado");
    display.display();
    eth_flag = 0;
  } else {
    if (eth_flag == 0) {
      // Apaga el Ethernet
      resetEthernet();

      display.clear();
      Encabezado();
      display.drawString(0, 15, "Cable Ethernet conectado");
      display.display();
      eth_flag = 1;
      // Llamar función Ethernet
      IniciarEthernet(mac);
      Iniciar_SNMP();
    }
  }


  // Impresión de datos por puerto serie
  unsigned long tiempoActual = millis();
  if (tiempoActual - tiempoAnterior >= intervalo) {
    Serial.print(voltage);
    Serial.println(" VRMS");
    tiempoAnterior = tiempoActual;
    if (eth_flag == 1) {
      if (vol_act != vol_ant) {
        vol_ant = vol_act;
        clearLine(0, 40, 128, 10); // (x, y, ancho, alto)
        display.drawString(0, 40, "VRMS: " + String(voltage));
        display.display();
      }
    }
  }
}
/**
void IniciarEthernet() {

  // Inicialización de conexión Ethernet
  Serial.println("Iniciando Ethernet...");
  display.clear();
  Encabezado();
  display.drawString(0, 15, "Iniciando Ethernet!");
  display.display();
  Ethernet.init(CSpin);

  if (Ethernet.begin(mac) != 0) {
    eth_flag = 1;
    Serial.println("DHCP OK!");
    display.drawString(0, 25, "DHCP OK!");
    display.display();
  } else {
    Serial.println("Configuración DHCP fallida.");
    display.drawString(0, 25, "Configuración DHCP fallida");
    display.display();

    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Módulo Ethernet No encontrado. Hardware ausente. Reiniciar.");
      display.drawString(0, 35, "Módulo Ethernet No encontrado. Hardware ausente. Reiniciar");
      display.display();
      while (true) {
        delay(1); // Loop infinito
      }
    }

    IPAddress ip(MYIPADDR);
    IPAddress dns(MYDNS);
    IPAddress gw(MYGW);
    IPAddress sn(MYIPMASK);
    Ethernet.begin(mac, ip, dns, gw, sn);
    Serial.println("IP ESTÁTICA CONFIGURADA");
    display.drawString(0, 45, "IP ESTÁTICA CONFIGURADA");
    display.display();
  }
  delay(10000);

 // Imprimir datos de conexión
  Serial.print("Local IP : ");
  Serial.println(Ethernet.localIP());
  Serial.print("Subnet Mask : ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("Gateway IP : ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("DNS Server : ");
  Serial.println(Ethernet.dnsServerIP());
  // Mostrar la dirección MAC como cadena hexadecimal
  Serial.print("Dirección MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(mac[i], HEX);
    if (i != 5) {
      Serial.print(":");
    }
  }
  Serial.println("");  


  // Configurar el texto
  display.clear();
  Encabezado();
  display.drawString(0, 15, "IP: "+ipToString(Ethernet.localIP()));
  display.drawString(0, 25, "MAC: "+macToString(mac));
  display.display();

}

// Función para borrar una línea específica
void clearLine(int x, int y, int width, int height) {
  display.setColor(BLACK);
  display.fillRect(x, y, width, height);
  display.setColor(WHITE);
  display.display();
}

//Encabezado
void Encabezado(){
  display.drawString(40, 0, "Sensor IoT");
  display.drawLine(0, 12, 128, 12);
}
**/
void Iniciar_SNMP(){
    // Iniciar SNMP
  snmp.setUDP(&Udp);
  snmp.begin();

  // Direcciones OID
  snmp.addReadOnlyStaticStringHandler(".1.3.6.1.4.1.5.13", "Prueba"); // String estática
  snmp.addIntegerHandler(".1.3.6.1.4.1.5.12", &voltage);
}
/**
void resetEthernet() {
  // Apaga el Ethernet
  Ethernet.end();

  // Espera un poco para asegurarte de que el módulo se ha apagado
  delay(1000);

  // Reinicia el Ethernet
  Ethernet.begin(mac);
  Serial.println("Ethernet reiniciado");
}
**/