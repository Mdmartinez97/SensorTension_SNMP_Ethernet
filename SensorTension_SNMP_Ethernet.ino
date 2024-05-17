#include <SPI.h>
#include <EthernetENC.h>
#include <EthernetUdp.h>
#include <SNMP_Agent.h>

#include <ZMPT101B.h>
#define SENSITIVITY 500 //Sensibilidad, calibrar con ejemplo en librería
#define Vpin 34
ZMPT101B voltageSensor(Vpin, 50); // (GPIO Pin, Frecuencia de red)


//ETHERNET
// Enter a MAC address for your controller below.
byte mac[] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x01 };
 
// Set the static IP address to use if the DHCP fails to assign
#define MYIPADDR 192,168,0,28
#define MYIPMASK 255,255,255,0
#define MYDNS 192,168,0,1
#define MYGW 192,168,0,1
 
// Initialize the Ethernet client library
EthernetClient client;
EthernetUDP Udp;

//SNMP
SNMPAgent snmp = SNMPAgent("public", "private");

// If we want to change the functionaality of an OID callback later, store them here.
TimestampCallback* timestampCallbackOID;

//std::string staticString = "ESP32 SNMP funcionando correctamente"; // String estática

  int voltage;
  int CoreTemp;

  unsigned long tiempoAnterior = 0;
  unsigned long intervalo = 10000; // 10 segundos

void setup() {
    Serial.begin(115200);

    pinMode(Vpin, INPUT);
    voltageSensor.setSensitivity(SENSITIVITY);

    Serial.println("Iniciando Ethernet...");
    
    Ethernet.init(5);   // CS pin
 
    if (Ethernet.begin(mac)) {
        Serial.println("DHCP OK!");
    }else{
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
 
 
    Serial.print("Local IP : ");
    Serial.println(Ethernet.localIP());
    Serial.print("Subnet Mask : ");
    Serial.println(Ethernet.subnetMask());
    Serial.print("Gateway IP : ");
    Serial.println(Ethernet.gatewayIP());
    Serial.print("DNS Server : ");
    Serial.println(Ethernet.dnsServerIP());
 
   Serial.println("Ethernet inicializado correctamente.");

  //SNMP
  snmp.setUDP(&Udp);
  snmp.begin();

  //snmp.addReadOnlyStaticStringHandler(".1.3.6.1.4.1.5.XX", staticString); // String estática
  snmp.addIntegerHandler(".1.3.6.1.4.1.5.12", &voltage);
  snmp.addIntegerHandler(".1.3.6.1.4.1.5.13", &CoreTemp);

}
 
void loop() {
  client.available(); // Llamado constante

  voltage = voltageSensor.getRmsVoltage();
  CoreTemp = temperatureRead();
  
  snmp.loop(); // Llamado constante

  unsigned long tiempoActual = millis();
    if (tiempoActual - tiempoAnterior >= intervalo) {
        Serial.print(voltage);
        Serial.println(" VRMS");
        Serial.print(CoreTemp);
        Serial.println(" °C");

        tiempoAnterior = tiempoActual;
    }

}
