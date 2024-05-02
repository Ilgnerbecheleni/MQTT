#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Ilgner";
const char* password = "0123456789";
const char* mqtt_server = "192.168.0.115"; // Endereço IP completo do servidor MQTT
const char* mqtt_topic = "command";
const int outputPin = 23;


const int trigPin = 21;
const int echoPin = 19;

#define VEL_SOM 0.034

long duracao ;
float distancia = 0 ;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
bool outputActive = false;

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(outputPin, OUTPUT);
  digitalWrite(outputPin, LOW);
  pinMode(trigPin,OUTPUT);
pinMode(echoPin,INPUT);
}

void setup_wifi(){
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Tentando reconectar ao MQTT...");
    if (client.connect("ESP32client")) {
      Serial.println("Conectado ao MQTT");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("Falha na conexão ao MQTT, código de retorno=");
      Serial.print(client.state());
      Serial.println(", tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Mensagem: ");
  Serial.println(message);

  if (strcmp(topic, mqtt_topic) == 0) {
    if (message.equals("1")) {
      // Ativa a saída por 1 segundo
      digitalWrite(outputPin, HIGH);
      Serial.println("Comando '1' recebido. ");
      
    }

      if (message.equals("0")) {
      // Ativa a saída por 1 segundo
      digitalWrite(outputPin, LOW);
      Serial.println("Comando '0' recebido. ");
      
    }
  }
}

void pegardistancia(){
  digitalWrite(trigPin,0);
delayMicroseconds(2);

digitalWrite(trigPin,1);
delayMicroseconds(10);
digitalWrite(trigPin,0);

duracao = pulseIn(echoPin, HIGH);

distancia = duracao * VEL_SOM / 2;

Serial.print("Distancia: ");
Serial.print(distancia);
Serial.println(" cm ");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    pegardistancia();
    char msg[10];
    snprintf(msg,10,"%.2f",distancia);
    
    // Envia mensagem de confirmação ao servidor MQTT
    client.publish("distancia", msg);
  }
}
