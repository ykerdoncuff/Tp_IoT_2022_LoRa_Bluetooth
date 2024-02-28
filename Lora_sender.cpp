#include <WiFi.h> // Inclut la bibliothèque pour se connecter au WiFi
#include <PubSubClient.h> // Gère la connexion et les communications avec un serveur MQTT
#include <LoRa.h> // Bibliothèque pour communiquer via la technologie LoRa
#include <SPI.h> // Bibliothèque pour la communication SPI, nécessaire pour LoRa

// Définition des broches pour le module LoRa
#define SCK 5    // GPIO5 -- SX127x's SCK
#define MISO 19  // GPIO19 -- SX127x's MISO
#define MOSI 27  // GPIO27 -- SX127x's MOSI
#define SS 18    // GPIO18 -- SX127x's CS (Chip Select)
#define RST 14   // GPIO14 -- SX127x's RESET
#define DI0 26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

// Configuration du client WiFi et MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Informations de connexion WiFi
const char *ssid = "Raspi-web";     // Nom du réseau WiFi
const char *password = "azertyui";  // Mot de passe du réseau WiFi

// Configuration du serveur MQTT
const char *mqtt_broker = "test.mosquitto.org"; // Adresse du broker MQTT
const char *topic = "srt5/GEFY"; // Topic MQTT sur lequel les messages seront publiés
const char *mqtt_username = "";
const char *mqtt_password = ""; 
const int mqtt_port = 1883; // Port pour la connexion MQTT

// Variables pour stocker les paramètres et les données LoRa
int frequence = 0;
int sf1 = 0;
int sb1 = 0;
float d1, d2;
int i = 0;

// Structure pour manipuler les paquets de données LoRa
union pack {
  uint8_t frame[16]; // Trames avec octets
  float data[4]; // 4 valeurs en virgule flottante
} sdp; // Paquet d’émission LoRa

void setup() {
  Serial.begin(115200); // Initialise la communication série à 115200 bauds
  WiFi.begin(ssid, password); // Connexion au réseau WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  
  pinMode(DI0, INPUT); // Configure DI0 en tant qu'entrée pour les interruptions LoRa
  SPI.begin(SCK, MISO, MOSI, SS); // Initialise la communication SPI avec les broches spécifiées
  LoRa.setPins(SS, RST, DI0); // Configure les broches LoRa
  
  client.setServer(mqtt_broker, mqtt_port); // Définit le serveur MQTT
  client.setCallback(callback);
  
  // Tente de se connecter au broker MQTT avec un identifiant client unique
  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public emqx mqtt broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void loop() {
  Serial.println("Sending message to MQTT topic..");
  // Publie des paramètres de configuration LoRa sur le topic MQTT 870Mhz, sp 12 et BW 125
  client.publish("srt5/GEFY", "870000000;12;125000");
  client.loop(); // Gère les tâches MQTT

  // Prépare et envoie un paquet LoRa avec les données d1 et d2
  Serial.println("Sending message to LoRa");
  float d1 = 12.0, d2 = 321.54;
  LoRa.beginPacket(); // Commence la construction du paquet
  sdp.data[0] = d1; // Assignation des données à envoyer
  sdp.data[1] = d2;
  LoRa.write(sdp.frame, 16); // Envoie le paquet
  LoRa.endPacket(); // Termine et envoie le paquet
  d1++; d2 += 2; // Incrémente les valeurs pour le prochain envoi
  delay(5000); // Attend 5 secondes avant le prochain envoi
}
