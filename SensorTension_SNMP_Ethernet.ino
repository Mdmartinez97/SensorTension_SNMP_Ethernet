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
#include <EthernetENC.h>
#include <EthernetUdp.h>
#include <SNMP_Agent.h>

#if ESP32
  // En caso de utilizar ESP32
  #include <WiFi.h>
  #define CSpin 5 // Módulo Ethernet
  #define Vpin 34 // Módulo sensor de tensión
#elif defined(ESP8266)
  // En caso de utilizar ESP8266
  #include <ESP8266WiFi.h>
  #define CSpin 2 // Módulo Ethernet
  #define Vpin A0 // Módulo sensor de tensión
#endif

// Archivo local de configuración
#include "NetConfig.h"

// Configuración de sensor de tensión
#include <ZMPT101B.h> // Modelo del sensor
#define SENSITIVITY 500 //Sensibilidad
ZMPT101B voltageSensor(Vpin, 50); // (GPIO Pin, Frecuencia de red en Hz)

// Inicializa libreria de cliente Ethernet
EthernetClient client;
EthernetUDP Udp;

// Agente SNMP
SNMPAgent snmp = SNMPAgent("public", "private");

// If we want to change the functionaality of an OID callback later, store them here.
TimestampCallback* timestampCallbackOID;

//std::string staticString = "SNMP funcionando correctamente"; // String estática

// Variable a medir
int voltage; // Tensión AC

// Frecuencia de impresión de datos en puerto serie
unsigned long tiempoAnterior = 0;
unsigned long intervalo = 10000; // 10 segundos

void setup() {
  Serial.begin(115200);

//---------------------------- Traspaso de MAC --------------------------
 // Configura estación WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // Desconectarse, si estuviera conectado
  delay(100);
  
  // Obtener la dirección MAC de ESP32 WiFi
  byte mac[6];
  WiFi.macAddress(mac);

  // Esta dirección MAC será asignada al módulo Etheret ENC28J60

  // Mostrar la dirección MAC como cadena hexadecimal
  Serial.print("Dirección MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(mac[i], HEX);
    if (i != 5) {
      Serial.print(":");
    }
  }
  Serial.println("");
//-----------------------------------------------------------------------

  // Sensor de tensión
  pinMode(Vpin, INPUT);
  voltageSensor.setSensitivity(SENSITIVITY);

  //  Inicialización de conexión Ethernet
  Serial.println("Iniciando Ethernet...");
  Ethernet.init(CSpin);   
 
  if (Ethernet.begin(mac)) {
      Serial.println("DHCP OK!");
  } else{
      Serial.println("Configuración DHCP fallida.");
     
      if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        Serial.println("Módulo Ethernet No encontrado. Hardware ausente. Reiniciar.");
        while (true) {
          delay(1); // Loop infinito
        }
      }
        if (Ethernet.linkStatus() == LinkOFF) {
          Serial.println("Cable Ethernet no conectado.");
        }
 
      IPAddress ip(MYIPADDR);
      IPAddress dns(MYDNS);
      IPAddress gw(MYGW);
      IPAddress sn(MYIPMASK);
      Ethernet.begin(mac, ip, dns, gw, sn);
      Serial.println("IP ESTÁTICA CONFIGURADA");
    }
  delay(2000);
 
 // Imprimir datos de conexión
  Serial.print("Local IP : ");
  Serial.println(Ethernet.localIP());
  Serial.print("Subnet Mask : ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("Gateway IP : ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("DNS Server : ");
  Serial.println(Ethernet.dnsServerIP());

  // Iniciar SNMP
  snmp.setUDP(&Udp);
  snmp.begin();

  // Direcciónes IOD
  //snmp.addReadOnlyStaticStringHandler(".1.3.6.1.4.1.5.13", staticString); // String estática
  snmp.addIntegerHandler(".1.3.6.1.4.1.5.12", &voltage);
}
 
void loop() {
 // Llamado constante para no cerrar comunicaión Ethernet
 client.available();
 
 // Lectura de variables
 voltage = voltageSensor.getRmsVoltage();
  
 // Llamado constante para no cerrar comunicaión SNMP
 snmp.loop();

 // Impresión de datos por puerto serie
  unsigned long tiempoActual = millis();
    if (tiempoActual - tiempoAnterior >= intervalo) {
        Serial.print(voltage);
        Serial.println(" VRMS");

        tiempoAnterior = tiempoActual;
    }
}
