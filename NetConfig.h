// Dirección MAC
// AA:AA:AA:AA:AA:"XX" --> reemplazar últimos dos dígitos con el número de sensor
byte mac[] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x01 };
 
// Configuración IP Fija --> según planilla
#define MYIPADDR 192,168,0,28
#define MYIPMASK 255,255,255,0
#define MYDNS 192,168,0,1
#define MYGW 192,168,0,1

/* Direcciones OID para SNMP
*
*  Tensión alterna --> ".1.3.6.1.4.1.5.12"
*  Temperatura interna ESP32 --> ".1.3.6.1.4.1.5.13"
*
*/