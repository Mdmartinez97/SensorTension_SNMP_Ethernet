// Configuración IP Fija
#define MYIPADDR 192,168,0,10
#define MYIPMASK 255,255,255,0
#define MYDNS 192,168,0,1
#define MYGW 192,168,0,1

/* -------------- Direcciones OID para SNMP -------------

    Tensión alterna --> ".1.3.6.1.4.1.5.12"
    String de prueba --> ".1.3.6.1.4.1.5.13"

------------------------ PINOUT ------------------------
    <ESP32>
    CS = 5
    SCK = 18
    MISO = 19
    MOSI = 23
    Vpin (sensor tensión) = 34

    <WEMOS D1 R1 8266>
    CSp = D4 
    SCLK = D5
    SO = D6
    SI = D7
    Vpin (sensor tensión) = A0      

*/