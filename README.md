# Tp_IoT_2022_LoRa

Auteurs : Yann KERDONCUFF et Florian DURAND
## Sujet du TP :

Dans ce projet nous allons réaliser une communication IoT utilisant la technologie LoRa pour l'échange de données entre un émetteur et un récepteur. Il implique la configuration des dispositifs pour se connecter à un réseau WiFi, l'utilisation du protocole MQTT pour récupérer ou publier des paramètres de configuration LoRa sur un broker MQTT. Voici un diagramme de séquence des échanges qui seront réalisés.

![mermaid-diagram-2024-02-28-144220](https://github.com/ykerdoncuff/Tp_IoT_2022_LoRa_Bluetooth/assets/57559729/0f7973e9-8dfe-447a-95c8-a9c9d423ace3)

## 1. Receiver :

Le récepteur effectue les opérations suivantes :

- **Connexion à un point d'accès WiFi**
  ```
  WiFi.begin(ssid, password); // Connexion au réseau WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  ```
- **Récupération d'un paquet MQTT** sur `test.mosquitto.org` du topic `srt5/GEFY`. et lecture du topic MQTT pour obtenir les valeurs nécessaires à la communication LoRa (fréquence, facteur d'étalement, bande passante).
  ```
  void callback(char *topic, byte *payload, unsigned int length) {
  if (!doCallback) return;
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  
  char message[length + 1];  // Tableau pour stocker le message reçu
  for (unsigned int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
      }
  message[length] = "\0"; // Ajoute le caractère de fin de chaîne
   }
 
  // Extrait les paramètres de configuration LoRa du message
  sscanf(message, "%d;%d;%d", &frequence, &sf1, &sb1);
  
  Serial.println(frequence);
  Serial.println(sf1);
  Serial.println(sb1);
  
  lora_init(); // Initialise le module LoRa avec les paramètres reçus
  
  doCallback = false; // Empêche l'exécution répétée du callback
  }
  ```
- **Écoute des données LoRa** transmises par l'émetteur, en utilisant les paramètres récupérés via MQTT.

### Fonctionnalités clés :

- Connexion au réseau WiFi spécifié.
- Abonnement au topic MQTT et récupération des paramètres LoRa.
- Configuration du module LoRa avec ces paramètres.
- Réception et affichage des données LoRa.

## 2. Sender :

L'émetteur effectue les opérations suivantes :

- **Connexion à un point d'accès WiFi** et publication d'un message MQTT sur `test.mosquitto.org` au topic `srt5/GEFY`, avec les valeurs nécessaires à LoRa (fréquence, facteur d'étalement, bande passante).
- **Envoi des données LoRa** en utilisant les paramètres spécifiés.

### Fonctionnalités clés :

- Connexion au réseau WiFi spécifié.
- Publication des paramètres de configuration LoRa sur le topic MQTT.
- Envoi périodique de paquets LoRa avec des données d'exemple.

### Remarques :

Pour chacune des deux parties (récepteur et émetteur), il est important de configurer correctement les broches pour le module LoRa et d'assurer une connexion stable au réseau WiFi ainsi qu'au broker MQTT. Les paramètres LoRa (fréquence, facteur d'étalement, bande passante) sont cruciaux pour une communication réussie et doivent être cohérents entre l'émetteur et le récepteur.



