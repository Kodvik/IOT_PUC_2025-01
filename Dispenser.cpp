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
#include <WiFi.H>
#include <PubSebClient.h>
#include <HX711.h>
#include <Servo.h>
/* **********************************************************************************************************************
 * endregion */

/* region Struct/Typedef Section */
/* *******************************************         Struct/Typedef section        *******************************************/

//log-in section for MQTT / WIFI
const char* ssid = "NOME_WIFI";
const char* password = "SENHA_WIFI";
const char* mqtt_server = "mosquitto.test.com";
WiFiClient espClient;
PubSubClient client(espClient);

/*Hardware Definitions*/
//HX711
HX711 balancas[4];
const int DT_PINS[4] = {4,12,14,26};
const int SCK_PINS[4] = {5,13,27,25};

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

void callback(char* topic, byte* message, unisigned int length){
    String msg;
    for(int i=0; i< length, i++){
        msg += (char)message[i]
    }

    if(String(topic) == "dispenser/config"){
        //Exemplo de JSON: {"slot0, "dose":2, "horarios":["08:00","20:00"]}
        DynamicJsonDocument doc(256);
        deserializeJson(doc,msg);
        int slot = doc["slot"];
        remedios[slot].qtd_por_dose = doc["dose"]
        for(int i=0; i<doc["horarios"].size();i++){
            remedios[slot].horario[i]
        }
    }
}

bool deveLiberarDose(int slot){
    String agora = getHoraAtual(); //usando um RTC ou NTP
    for(int = 0; i<4; i++){
        if(remedios[slot].horario[i] == agora){
            return true;
        }
    }
    return false;
}

void liberarDose(int slot){
    servos[slot].write(90);
    millis(10000); // 10 segundos para retirada do medicamento
    servos[slot].write(0);
}
/* **********************************************************************************************************************
 * endregion */

/* region Main Code */
/* ********************************************         Main Loop          ********************************************/
void loop(){
    if(!client.connect()) reconnect();
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
    mililis(1000) // checagem a cada segundo
}
/* **********************************************************************************************************************
 * endregion */
