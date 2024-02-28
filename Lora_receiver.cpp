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
const char *topic = "srt5/GEFY"; // Topic MQTT sur lequel les messages seront publiés/souscrits
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 1883; // Port pour la connexion MQTT

// Variables pour stocker les paramètres et les données LoRa
int frequence = 0;
int sf1 = 0;
int sb1 = 0;
float d1, d2;
int i = 0, rssi = 0;

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
  client.setCallback(callback); // Définit la fonction de callback pour les messages MQTT
  
  // Tente de se connecter au broker MQTT avec un identifiant client unique
  while (!client.connected()) {
    String client_id = "esp32-Florian-";
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
  client.subscribe(topic); // Souscrit au topic MQTT spécifié
}

bool doCallback = true; // Contrôle si le callback doit être exécuté

// Fonction de callback appelée à la réception d'un message MQTT
void callback(char *topic, byte *payload, unsigned int length) {
  if (!doCallback) return;
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  
  char message[length + 1];  // Tableau pour stocker le message reçu
  for (unsigned int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0'; // Ajoute le caractère de fin de chaîne
  
  // Extrait les paramètres de configuration LoRa du message
  sscanf(message, "%d;%d;%d", &frequence, &sf1, &sb1);
  
  Serial.println(frequence);
  Serial.println(sf1);
  Serial.println(sb1);
  
  lora_init(); // Initialise le module LoRa avec les paramètres reçus
  
  doCallback = false; // Empêche l'exécution répétée du callback
}

// Initialise la communication LoRa avec les paramètres spécifiés
void lora_init() {
  if (!LoRa.begin(frequence)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("Starting LoRa good!");
  LoRa.setSpreadingFactor(sf1); // Définit le facteur d'étalement
  LoRa.setSignalBandwidth(sb1); // Définit la bande passante du signal
  
  Serial.println("LoRa initialized!");
}

void loop() {
  client.loop(); // Gère les tâches MQTT
  int packetLen = LoRa.parsePacket();
  
  if (packetLen == 16) { // Vérifie si un paquet LoRa de 16 octets a été reçu
    i = 0;
    while (LoRa.available()) { // Lit le paquet LoRa octet par octet
      sdp.frame[i] = LoRa.read();
      i++;
    }
    d1 = sdp.data[0]; // Extrait les données reçues
    d2 = sdp.data[1];
    rssi = LoRa.packetRssi(); // Lit la force du signal reçu
    
    Serial.println("Reception LoRa : ");
    Serial.println(d1); // Affiche les données reçues
    Serial.println(d2);
    Serial.print(rssi);
    Serial.println(" dBm"); // Affiche la force du signal en dBm
  }
}
