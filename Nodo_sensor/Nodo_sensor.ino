#include <DHT22.h>
/*Se define el Pin 12 para la conexion al Sensor,
  se debe utilizar una resistencia de 4,7K pull up 
*/
#define DHT22_PIN 12
#define DHTPIN 11    // Indicamos el pin donde conectaremos la patilla data de nuestro sensor

DHT22 dht22(DHT22_PIN);  //Se crea una instancia de la Clase y se Envia el Pin
#include "DHT.h"  //Añadimos la libreria con la cual trabaja nuestro sensor
 
// Descomenta el tipo de sensor que vas a emplear. En este caso usamos el DHT11
#define DHTTYPE DHT11   // DHT 11 
DHT dht(DHTPIN, DHTTYPE);  //Indica el pin con el que trabajamos y el tipo de sensor

int seleccion = 10;
int estado =9;
//int pinSleep= 5;  //Pin número 11-D6

float Voltaje=0;
float Tempe=0;
float Hum=0;
String mac="";
char completado='0';

void setup(void)
{
  delay(500);
  Serial.begin(9600);
  pinMode(seleccion,INPUT);
  pinMode(estado,OUTPUT);
  dht.begin();
  delay(6000);
  Iniciar_Nodo();
  Serial.println("Iniciando Sensor Remoto");
}

void loop(void){
  
   String confirmacion="";
   char caracter;
  
   if((mac=="") || (mac.length()!=16)){
      delay(100);
      Iniciar_Nodo();
   }
   else{
         Parpadeo_Led();
         int valor= analogRead(A0); // Lee El ADC con Valores Entre 0 y 1024.
         Voltaje= ((5.0*valor)/(1024)); 
     
   if(digitalRead(seleccion)){
       digitalWrite(estado,LOW);
       DHT22_ERROR_t evento;
       delay(2000); // Se hace un retraso de 2 Segundos que necesita el sensor para realizar la medicion
  
       evento = dht22.readData(); /* Se realiza la lectura y esta retorna un evento en donde puede
                                       ó no ocurrir un error, o una suma de verificacion erronea CHECKSUM*/
      switch(evento)
      {  
        case DHT_ERROR_NONE:    // No Hay Errores
      
        Tempe=dht22.getTemperatureC();
        Hum=dht22.getHumidity();
        break;
      
      case DHT_ERROR_CHECKSUM:  // Suma de Verificacion Erronea CHECKSUM
        Serial.print("check sum error ");  
        Tempe=dht22.getTemperatureC();
        Hum=dht22.getHumidity();
        break;
      
      case DHT_BUS_HUNG:
        Serial.println("BUS Hung "); // Dato Perdido
        break;
      
      case DHT_ERROR_NOT_PRESENT:
        Serial.println("Not Present "); // El sensor no esta presente
        break;
      
      case DHT_ERROR_ACK_TOO_LONG:
        Serial.println("ACK time out ");  // Suma de Verificacion Muy Larga
        break;
      
      case DHT_ERROR_SYNC_TIMEOUT:
        Serial.println("Sync Timeout "); // Tiempo de Sincronizacion terminado
        break;
      
      case DHT_ERROR_DATA_TIMEOUT:
        Serial.println("Tiempo de Espera Terminado"); 
        break;
      
      case DHT_ERROR_TOOQUICK:
        Serial.println("Polled to quick "); // Sondeo Rapido
        break;
      }
   }
   else
   {
      digitalWrite(estado,HIGH);
      delay(2000);
      digitalWrite(estado,LOW);
      // La lectura de la temperatura o de la humedad lleva sobre 250 milisegundos  
      // La lectura del sensor tambien puede estar sobre los 2 segundos (es un sensor muy lento)
      Hum = dht.readHumidity();  //Guarda la lectura de la humedad en la variable float h
      Tempe = dht.readTemperature();  //Guarda la lectura de la temperatura en la variable float t
 
      // Comprobamos si lo que devuelve el sensor es valido, si no son numeros algo esta fallando
      if (isnan(Tempe) || isnan(Hum)) // funcion que comprueba si son numeros las variables indicadas 
      {
        Serial.println("Fallo al leer del sensor DHT"); //Mostramos mensaje de fallo si no son numeros
      } 
     delay(500);
   }

     //Se Prepara el frame de datos y se envia 
     char temp1[sizeof(Tempe)];
     char hume1[sizeof(Hum)];
     char volt1[sizeof(Voltaje)];
     
     String str_out =String("TEM"+String(dtostrf(Tempe,0,2,temp1))+String(",")+"HUM"+String(dtostrf(Hum,0,2,hume1))+String(",")+"VOL"+String(dtostrf(Voltaje,0,1,volt1))+String(",")+"MAC"+mac);

    while(confirmacion!=mac){
         confirmacion="";
         delay(4000);
         Serial.println(str_out);

         delay(50);
         while (Serial.available() > 0) {
           caracter = Serial.read();
           if(caracter!=byte(13)){
              confirmacion.concat(caracter);
           }  
         }  
    }
     
     //Se pone a dormir el modúlo Xbee por minuto y medio
        Dormir_Xbee();
        delay(500);        
     }
}

void Iniciar_Nodo(){
   String content = "";
   char character;
   
   Serial.println("------------------");
   delay(1200);
   Serial.print("+++");
   delay(1200);
  
   bool bOK = false;
    
   while (Serial.available() > 0) {
    Serial.write(Serial.read());
     bOK = true;
   }  
   
  if(bOK==true)
  { 
    Serial.println("ATSH");
    delay(100);
    while (Serial.available() > 0) {
      character = Serial.read();
      if(character!=byte(13)){
        content.concat(character);
      }
    }
    
    Serial.println();
    Serial.println("ATSL");
    delay(100);
    while (Serial.available() > 0) {
      character = Serial.read();
      if(character!=byte(13)){
      content.concat(character);
      }
    }
    
    Serial.println();
    Serial.println("ATCN");
    delay(100);
    Serial.println();
  }
  else{
     content= "";
  }

  if (content != "") {
    if(sizeof(content)==16){
        mac=content;
    }
    else{
      int tam=16-content.length();
      String aux_mac;
      for(int a=0;a<tam;a++){
        aux_mac.concat(completado);        
      }
      aux_mac.concat(content);
      mac=aux_mac;
    }
  }
  else{
    Serial.println("Error en la lectura de mac - nodo sensor");
  }
}

void Dormir_Xbee(){
  delay(100000);
}

void Parpadeo_Led(){
   for(int a=0;a<5;a++){
      digitalWrite(estado,!digitalRead(estado)); 
      delay(100);
   }
}
 //sprintf(frame,"TEM%s,HUM%s,VOL%s,MAC%s","56.6","45.5","5.0","0013A200408B8BD3");   
 // Serial.println(frame);

