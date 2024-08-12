#ifndef FUNCONES_H
#define FUNCIONES_H

extern SSD1306Wire display;
extern SNMPAgent snmp;
extern EthernetUDP Udp;
// Función para convertir una dirección MAC a String
String macToString(const byte* mac) {
  String macStr = "";
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 0x10) {
      macStr += "0";
    }
    macStr += String(mac[i], HEX);
    if (i < 5) {
      macStr += ":";
    }
  }
  macStr.toUpperCase();
  return macStr;
}

// Función para convertir una IPAddress a String
String ipToString(IPAddress ip) {
  String ipStr = "";
  for (int i = 0; i < 4; i++) {
    ipStr += String(ip[i]);
    if (i < 3) {
      ipStr += ".";
    }
  }
  return ipStr;
}
//Encabezado
void Encabezado(){
  display.drawString(40, 0, "Sensor IoT");
  display.drawLine(0, 12, 128, 12);
}

void IniciarEthernet(byte mac[]) {

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

void resetEthernet() {
  // Apaga el Ethernet
  Ethernet.end();

  // Espera un poco para asegurarte de que el módulo se ha apagado
  delay(1000);

  // Reinicia el Ethernet
  Ethernet.begin(mac);
  Serial.println("Ethernet reiniciado");
}



#endif