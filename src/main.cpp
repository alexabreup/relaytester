#include "EmonLib.h"   // Biblioteca para monitoramento de energia
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>  // Biblioteca para fazer requisições HTTP

EnergyMonitor emon;
#define vCalibration 106.8
#define currCalibration 0.52

// Credenciais Wi-Fi
const char* ssid = "z3";
const char* password = "g1g1g3g3$$$";


// URL do servidor Express.js
const char* serverUrl = "http://192.168.x.x:3000/update"; // Substitua pelo IP do servidor e porta

// Pinos para os LEDs
#define LED_RED_PIN 2   // Exemplo de pino GPIO para o LED vermelho
#define LED_BLUE_PIN 4  // Exemplo de pino GPIO para o LED azul

float kWh = 0;
unsigned long lastmillis = millis();

void sendData(float voltage, float current, float power, float energy) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");

        // JSON para enviar os dados
        String jsonData = String("{\"voltage\":") + voltage + ",\"current\":" + current + ",\"power\":" + power + ",\"energy\":" + energy + "}";
        
        int httpResponseCode = http.POST(jsonData);
        if (httpResponseCode > 0) {
            Serial.print("Resposta do servidor: ");
            Serial.println(httpResponseCode);
        } else {
            Serial.print("Erro na conexão: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    } else {
        Serial.println("Erro na conexão Wi-Fi");
    }
}

void connectToWiFi() {
    WiFi.begin(ssid, password);
    Serial.print("Conectando ao Wi-Fi...");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        digitalWrite(LED_BLUE_PIN, LOW);  // Mantenha o LED azul desligado enquanto tenta conectar
        delay(500);
    }
    Serial.println("\nConectado ao Wi-Fi!");
    digitalWrite(LED_BLUE_PIN, HIGH);  // Liga o LED azul ao conectar
}

void setup() {
    Serial.begin(9600);

    // Configura os pinos dos LEDs como saídas
    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(LED_BLUE_PIN, OUTPUT);

    // Conecta ao Wi-Fi
    connectToWiFi();

    // Configuração dos sensores
    emon.voltage(35, vCalibration, 1.7); // Tensão: pino de entrada, calibração, phase_shift
    emon.current(34, currCalibration);   // Corrente: pino de entrada, calibração.
}

void loop() {
    // Verifica se o Wi-Fi está conectado e pisca os LEDs intermitentemente
    if (WiFi.status() == WL_CONNECTED) {
        // Pisca os LEDs alternadamente para simular uma luz estroboscópica
        digitalWrite(LED_RED_PIN, HIGH);
        delay(100); // Tempo ligado para o LED vermelho
        digitalWrite(LED_RED_PIN, LOW);
        delay(100); // Tempo desligado para o LED vermelho
    } else {
        // Se o Wi-Fi não estiver conectado, ambos os LEDs permanecem apagados
        digitalWrite(LED_RED_PIN, LOW);
        digitalWrite(LED_BLUE_PIN, LOW);
        
        // Tenta reconectar ao Wi-Fi
        connectToWiFi();
    }

    // Realiza as leituras de tensão, corrente e potência
    emon.calcVI(20, 2000); // Calcula Vrms, Irms e potência aparente
    
    float voltage = emon.Vrms;
    float current = emon.Irms;
    float power = emon.apparentPower;
    
    // Calcula energia em kWh
    kWh += power * (millis() - lastmillis) / 3600000000.0;
    lastmillis = millis();

    // Exibe no Serial Monitor de forma organizada
    Serial.println("=============== LEITURAS ===============");
    Serial.print("Tensão (V): ");
    Serial.print(voltage, 2);  // Limita a duas casas decimais
    Serial.println(" V");

    Serial.print("Corrente (A): ");
    Serial.print(current, 4);  // Limita a quatro casas decimais
    Serial.println(" A");

    Serial.print("Potência (W): ");
    Serial.print(power, 4);    // Limita a quatro casas decimais
    Serial.println(" W");

    Serial.print("Energia (kWh): ");
    Serial.print(kWh, 4);      // Limita a quatro casas decimais
    Serial.println(" kWh");
    Serial.println("========================================");

    // Envia os dados ao servidor
    sendData(voltage, current, power, kWh);

    delay(5000); // Intervalo de 5 segundos para a próxima leitura
}
