# 📝 Dossier Technique : Advanced Air Analyser (ESP32-C3)

Ce document récapitule l'architecture matérielle finale, optimisée pour la stabilité électrique et la précision des mesures environnementales.

---

## 1. Liste des Composants
* **Microcontrôleur :** ESP32-C3 Super Mini (USB-C natif).
* **Capteur CO2 :** Sensirion SCD40 (NDIR véritable).
* **Capteur Particules :** Plantower PMS 5003 (Laser scattering).
* **Capteur Environnement :** Bosch BME688 (Temp, Hum, Press, Gaz/COV).
* **Capteur Présence :** HLK-Tech LD2410C (Radar 24GHz).
* **Stabilisation :** 3x Condensateurs 100µF (Électrolytiques) + 1x 100nF (Céramique).

---

## 2. Schéma de Câblage Détaillé

### A. Gestion de l'Alimentation & Filtrage
Le filtrage par condensateurs est **obligatoire** pour absorber les pics de courant du ventilateur PMS et du Wi-Fi.

| Composant | Capacité | Emplacement | Connexion |
| :--- | :--- | :--- | :--- |
| **Chimique** | **3x 100µF** | Rail 5V (Entrée) | **Bande grise** sur GND / **Patte longue** sur 5V |
| **Céramique** | **1x 100nF** | Près de l'ESP32 | Entre **3V3** et **GND** (Pas de sens) |

### B. Brochage des Capteurs

| Composant | Pin Capteur | Pin ESP32-C3 | Tension | Fonction |
| :--- | :--- | :--- | :--- | :--- |
| **SCD40** | SDA / SCL | **GPIO 8 / GPIO 9** | 3.3V | Bus I2C (0x62) |
| **BME688** | SDA / SCL | **GPIO 8 / GPIO 9** | 3.3V | Bus I2C (0x76) |
| **PMS 5003** | VCC / GND | **5V / GND** | 5V | Puissance |
| | TXD | **GPIO 20** | 3.3V | Flux série (RX0) |
| | SET | **3.3V** | 3.3V | Activation Laser |
| **LD2410C** | VCC / GND | **5V / GND** | 5V | Puissance |
| | OUT | **GPIO 10** | 3.3V | Signal Présence |

---

## 3. Architecture Visuelle (Mermaid)

```mermaid
graph TD
    subgraph "Source d'Énergie"
        USB[USB-C 5V]
        VCC[Rail 5V / VBUS]
        GND[Rail Masse / GND]
        USB --> VCC
        USB --> GND
    end

    subgraph "Filtrage & Lissage"
        C1[3x 100µF en Parallèle] --- VCC
        C1 --- GND
        C2[1x 100nF] --- ESP_3V3
        C2 --- GND
    end

    subgraph "ESP32-C3 Super Mini"
        ESP_5V[Pin 5V]
        ESP_3V3[Pin 3V3]
        GND_ESP[Pin GND]
        I2C[I2C: SDA:8 / SCL:9]
        UART[UART RX:20]
        GPIO10[GPIO 10]
    end

    VCC --> ESP_5V
    GND --> GND_ESP

    subgraph "Capteurs I2C (3.3V)"
        SCD[SCD40 CO2]
        BME[BME688 Env]
        I2C --- SCD
        I2C --- BME
        SCD --- ESP_3V3
        BME --- ESP_3V3
    end

    subgraph "Capteurs Haute Puissance (5V)"
        PMS[PMS 5003]
        RAD[LD2410C Radar]
        PMS --"TXD"--> UART
        RAD --"OUT"--> GPIO10
        PMS --- VCC
        RAD --- VCC
        PMS --"SET"--> ESP_3V3
    end

    style C1 fill:#f96,stroke:#333
    style C2 fill:#ffeb3b,stroke:#333
    style ESP32-C3 fill:#2196f3,color:#fff