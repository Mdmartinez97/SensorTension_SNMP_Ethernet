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

// Archivos Locales
#include "NetConfig.h"

// Configuración de sensor de tensión
#include <ZMPT101B.h> // Modelo del sensor
#define SENSITIVITY 500 //Sensibilidad
#define Vpin 34 // GPIO pin ESP32
ZMPT101B voltageSensor(Vpin, 50); // (GPIO Pin, Frecuencia de red en Hz)

// Inicializa libreria de cliente Ethernet
EthernetClient client;
EthernetUDP Udp;

// Agente SNMP
SNMPAgent snmp = SNMPAgent("public", "private");

// If we want to change the functionaality of an OID callback later, store them here.
TimestampCallback* timestampCallbackOID;

//std::string staticString = "ESP32 SNMP funcionando correctamente"; // String estática

// Variable a medir
int voltage; // Tensión AC
int CoreTemp; // Temperatura interna ESP

// Frecuencia de impresión de datos en puerto serie
unsigned long tiempoAnterior = 0;
unsigned long intervalo = 10000; // 10 segundos

void setup() {
  Serial.begin(115200);

  // Sensor de tensión
  pinMode(Vpin, INPUT);
  voltageSensor.setSensitivity(SENSITIVITY);

  //  Inicialización de conexión Ethernet
  Serial.println("Iniciando Ethernet...");
  Ethernet.init(5);   // CS pin
 
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
      Serial.println("IP ESTÁTICA OK!");
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
  //snmp.addReadOnlyStaticStringHandler(".1.3.6.1.4.1.5.XX", staticString); // String estática
  snmp.addIntegerHandler(".1.3.6.1.4.1.5.12", &voltage);
  snmp.addIntegerHandler(".1.3.6.1.4.1.5.13", &CoreTemp);

}
 
void loop() {
 // Llamado constante para no cerrar comunicaión Ethernet
 client.available();
 
 // Lectura de variables
 voltage = voltageSensor.getRmsVoltage();
 CoreTemp = temperatureRead();
  
 // Llamado constante para no cerrar comunicaión SNMP
 snmp.loop();

 // Impresión de datos por puerto serie
  unsigned long tiempoActual = millis();
    if (tiempoActual - tiempoAnterior >= intervalo) {
        Serial.print(voltage);
        Serial.println(" VRMS");
        Serial.print(CoreTemp);
        Serial.println(" °C");

        tiempoAnterior = tiempoActual;
    }
}
