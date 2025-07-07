/* region README */
/* **********************************************         READ ME          **********************************************
 * Code by: Edgard Melo (1005132@sga.pucminas.br) - Puc Minas - Engenharia da Computação
 * Date: 2025-06-11 (YYYY-MM-DD)
 * Version: 0.5
 * **********************************************************************************************************************
 * endregion */

/* region Patch Notes */
/* ********************************************         Patch Notes          ********************************************
 * Version 0.5:
 *
 * Patch notes:
 * -changed the weight cell to the model HX711 amplifier
 * -applied a few changes to the connection logic
 * Current development status: 
 * -still working on the changes for the
 * Next steps: 
 * -the scale still heavly imprecise, big 
 * **********************************************************************************************************************
 * endregion */

/* region Include Section */
/* *******************************************         Include section        *******************************************/
#include <WiFi.h>
#include <PubSubClient.h>
#include <HX711.h>
#include <Servo.h>
#include <ArduinoJson.h>
/* **********************************************************************************************************************
 * endregion */

/* region Struct/Typedef Section */
/* *******************************************         Struct/Typedef section        *******************************************/

//log-in section for MQTT / WIFI
const char* ssid = "Kodvik wifi hotspot";
const char* password = "abcd1234";

const char* mqtt_server = "test.mosquitto.org"; // Corrigido o endereço do broker MQTT
const int mqtt_port = 1883; // Porta do broker MQTT

/* Definicao dos topicos */
/* Alarmes */
const char* topico_slot1 = "dispenser/slot1";
const char* topico_slot2 = "dispenser/slot2";
const char* topico_slot3 = "dispenser/slot3";
const char* topico_slot4 = "dispenser/slot4";

const char* topico_slot1_pesoAtual = "dispenser/slot1/peso";
const char* topico_slot2_pesoAtual = "dispenser/slot2/peso";
const char* topico_slot3_pesoAtual = "dispenser/slot3/peso";
const char* topico_slot4_pesoAtual = "dispenser/slot4/peso";

const char* topico_slot1_pesoUnitario = "dispenser/slot1/pesoUnitario";
const char* topico_slot2_pesoUnitario = "dispenser/slot2/pesoUnitario";
const char* topico_slot3_pesoUnitario = "dispenser/slot3/pesoUnitario";
const char* topico_slot4_pesoUnitario = "dispenser/slot4/pesoUnitario";

const char* topico_slot1_horario = "dispenser/slot1/horario";
const char* topico_slot2_horario = "dispenser/slot2/horario";
const char* topico_slot3_horario = "dispenser/slot3/horario";
const char* topico_slot4_horario = "dispenser/slot4/horario";

const char* topico_slot1_dosesRestantes = "dispenser/slot1/dosesRestantes";
const char* topico_slot2_dosesRestantes = "dispenser/slot2/dosesRestantes";
const char* topico_slot3_dosesRestantes = "dispenser/slot3/dosesRestantes";
const char* topico_slot4_dosesRestantes = "dispenser/slot4/dosesRestantes";

/* Inicialização do WiFi */
void setup_wifi(){
  delay(100); // Atraso para estabilizar a conexão, sugerido pelo guia que segui
  Serial.println(); // Limpa o buffer serial
  Serial.print("Conectando-se ao WiFi "); // Mensagem de conexão
  Serial.print(ssid); // Nome da rede Wi-Fi
  WiFi.begin(ssid, password); // Inicia a conexão Wi-Fi

  while (WiFi.status() != WL_CONNECTED) { // Enquanto não estiver conectado
    delay(500); // Atraso de 500ms
    Serial.print("."); // Imprime ponto no console
  }
  Serial.println(); // Limpa o buffer serial
  Serial.println("WiFi conectado"); // Mensagem de conexão bem-sucedida
  Serial.print("Endereço IP: "); // Mensagem de endereço IP
  Serial.println(WiFi.localIP()); // Imprime o endereço IP local
}

/* caso perca a conexão de WiFi tenta reconectar */
void reconnect() {
  while (!client.connected()) { // Enquanto não estiver conectado
    Serial.print("Tentando conexão MQTT..."); // Mensagem de tentativa de conexão
    if (client.connect("TP2_e_TP3_IOT")) { // Nome do cliente MQTT atualizado
      Serial.println("conectado"); // Mensagem de conexão bem-sucedida
      client.subscribe("dispenser/config"); // Inscreve-se no tópico de configuração
    } else {
      Serial.print("Falha na conexão, rc="); // Mensagem de falha na conexão
      Serial.print(client.state()); // Imprime o estado da conexão
      Serial.println(" tentado novamente em 10 segundos"); // Mensagem de tentativa novamente
      delay(10000); // Atraso de 10 segundos antes de tentar novamente
    }
  }
}

WiFiClient espClient;
PubSubClient client(espClient);

/*Hardware Definitions*/
//HX711
HX711 balancas[4];
const int DT_PINS[4] = {4,12,14,26};
const int SCK_PINS[4] = {5,13,27,25};
const float CALIBRATION_FACTOR[4] = {-7050.0, -7050.0, -7050.0, -7050.0}; // Ajustar conforme necessário

//Servos
Servo servos[4];
const int SERVO_PINS[4] = {15,16,17,18};

//Sensor de presença
const int SENSOR_PRESENCA = 2;

/*Structs*/
struct Medicamento{
    int qtd_por_dose;
    String horario[4];
};
Medicamento remedios [4];
/* **********************************************************************************************************************
 * endregion */

/* region Function Section */
/* *******************************************         Function section        *******************************************/
float lerPeso(int slot){
    return balancas[slot].get_units(5);
}

void callback(char* topic, byte* message, unsigned int length){
    String msg;
    for(int i=0; i< length; i++){
        msg += (char)message[i];
    }

    if(String(topic) == "dispenser/config"){
        //Exemplo de JSON: {"slot":0, "dose":2, "horarios":["08:00","20:00"]}
        DynamicJsonDocument doc(256);
        deserializeJson(doc,msg);
        int slot = doc["slot"];
        remedios[slot].qtd_por_dose = doc["dose"];
        for(int i=0; i<doc["horarios"].size();i++){
            remedios[slot].horario[i] = doc["horarios"][i];
        }
    }
}

bool deveLiberarDose(int slot){
    String agora = getHoraAtual(); //usando um RTC ou NTP
    for(int i = 0; i<4; i++){
        if(remedios[slot].horario[i] == agora){
            return true;
        }
    }
    return false;
}

void liberarDose(int slot){
    servos[slot].write(90);
    delay(10000); // 10 segundos para retirada do medicamento
    servos[slot].write(0);
}

// Function to get current time (placeholder - implement with RTC or NTP)
String getHoraAtual() {
    // TODO: Implement with RTC DS3231 or NTP client
    // For now, return a test time
    return "08:00";
}
/* **********************************************************************************************************************
 * endregion */

/* region Setup Function */
/* *******************************************         Setup Function        *******************************************/
void setup() {
  Serial.begin(115200);
  
  // Initialize HX711 scales
  for(int i = 0; i < 4; i++) {
    balancas[i].begin(DT_PINS[i], SCK_PINS[i]);
    balancas[i].set_scale(CALIBRATION_FACTOR[i]);
    balancas[i].tare(); // Reset scale to 0
  }
  
  // Initialize servos
  for(int i = 0; i < 4; i++) {
    servos[i].attach(SERVO_PINS[i]);
    servos[i].write(0); // Initial position
  }
  
  // Initialize presence sensor
  pinMode(SENSOR_PRESENCA, INPUT);
  
  // Initialize WiFi and MQTT
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}
/* **********************************************************************************************************************
 * endregion */

/* region Main Code */
/* ********************************************         Main Loop          ********************************************/
void loop(){
    if(!client.connected()) reconnect();
    client.loop();

    //Exemplo: se hora == "08:00" e presença detectada, libera dose
    for(int i=0; i<4; i++){
        if(deveLiberarDose(i)){
            if(digitalRead(SENSOR_PRESENCA) == HIGH){
                liberarDose(i);
                client.publish("dispenser/logs","Dose liberada slot "+String(i));
            }
            else{
                client.publish("dispenser/logs","paciente ausente no slot " + String(i));
            }
        }
    }
    delay(1000); // checagem a cada segundo
}
/* **********************************************************************************************************************
 * endregion */
