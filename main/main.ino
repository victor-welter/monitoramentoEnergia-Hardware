// INCLUSÃO DA BIBLIOTECA NECESSÁRIA
#include <Arduino.h>

// Bibliotecas Sensor de Amperagem
#include "ACS712.h"

// Bibliotecas Shild
#include <SPI.h>
#include <Ethernet.h>

String urlInsert = "GET /salvaMonitoramento?codigo=3";

// Definicoes de IP, mascara de rede e gateway
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

IPAddress server(10, 0, 0, 2); //Notebook
IPAddress ip(10, 0, 0, 66);
IPAddress myDns(10, 0, 0, 3); //Placa de rede

// Inicializa o servidor web na porta 8080
EthernetClient client;

// Variables to measure the speed
unsigned long beginMicros;

int sensorValue_aux = 0;
float valorSensor = 0;
float voltsporUnidade = 0.004887586;// 5%1023
float sensibilidade = 0.076;

float amperagem = 0;
int voltagem = 220;
float resistencia = 0;

void setup()
{
    pinMode(A0, INPUT);

    // start the Ethernet connection:
    Serial.println("Inicializar Ethernet com DHCP:");
    if (Ethernet.begin(mac) == 0)
    {
        Serial.println("Falha ao configurar Ethernet usando DHCP");
        // Check for Ethernet hardware present
        if (Ethernet.hardwareStatus() == EthernetNoHardware)
        {
            Serial.println("O escudo Ethernet não foi encontrado. Desculpe, não pode ser executado sem hardware. :(");
            while (true)
            {
                delay(1); // do nothing, no point running without Ethernet hardware
            }
        }
        if (Ethernet.linkStatus() == LinkOFF)
        {
            Serial.println("O cabo Ethernet não está conectado.");
        }
        // try to congifure using IP address instead of DHCP:
        Ethernet.begin(mac, ip, myDns);
    }
    else
    {
        Serial.print(" DHCP atribuída IP ");
        Serial.println(Ethernet.localIP());
    }

    // give the Ethernet shield a second to initialize:
    delay(1000);
    Serial.print("conectando à ");
    Serial.print(server);
    Serial.println("...");

    // Velocidade de transmissão da porta serial
    Serial.begin(9600);
}

void loop()
{
    for (int i = 10000; i > 0; i--)
    {
        // le o sensor na pino analogico A0 e ajusta o valor lido ja que a saída do sensor é (1023)vcc/2 para corrente =0
        sensorValue_aux = (analogRead(A0) - 512);

        // somam os quadrados das leituras.
        valorSensor += pow(sensorValue_aux, 2);
    }

    // finaliza o calculo da média quadratica e ajusta o valor lido para volts
    valorSensor = (sqrt(valorSensor / 10000)) * voltsporUnidade;

    // calcula a corrente considerando a sensibilidade do sernsor (185 mV por amper)
    amperagem = (valorSensor / sensibilidade);

     if (amperagem <= 0.150)
    {
        amperagem = 0;
    }

    valorSensor = 0;

    // Mostra o valor da corrente
    Serial.print("\nCorrente : ");
    Serial.print(amperagem, 3);
    Serial.print(" A \n");

    // Calcula e mostra o valor da potencia
    Serial.print("Potencia (Consumo) : ");
    Serial.print(amperagem * voltagem);
    Serial.println(" Watts ");

    // Calcula o valor da resistencia
    resistencia = (voltagem / amperagem);

    // Mostra o valor da resistencia
    Serial.print("Resistencia : ");
    Serial.print(resistencia);
    Serial.println(" Ohms ");
    
    if(amperagem != 0) {
    String request = urlInsert + "&voltagem=" + voltagem + "&amperagem=" + amperagem + "&resistencia=" + resistencia;

    Serial.println(request);

    // Envia a requisição para o servidor
    enviaRequisicao(request);
    }

    delay(300000);
}

void enviaRequisicao(String mensagem)
{
    if (client.connect(server, 8080))
    {
        Serial.print("conectado a ");
        Serial.println(client.remoteIP());
        client.println(mensagem);
    }
    else
    {
        Serial.println("conexão falhou");
    }

    beginMicros = micros();
}