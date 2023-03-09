#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>
#include <string.h>
#include <Wire.h>

// Inicializa o display no endereco
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Definindo as Portas das I/Os
#define VS 2   // Output, Valvula Superior
#define VI 4   // Output, Valvula Inferior
#define AQ 15  // Output, Aquecedor
#define L 34   // Input, Botao Liga
#define ST 32  // Input, Sensor de Temperatura
#define SP 35  // Input, Sensor da Porta
#define buzzer 14

int t1, t2;

String topicomqtt = "Estado";

const char *broker = "broker.hivemq.com";
const int port = 1883;

WiFiClient wifiClient;
PubSubClient cliente;

// Variavel que armazanam o tempo de espera dos diferentes esta2000s o aquecimento da Agua
// const int Tcl = 1200000; // um milhão e dozentos mil milissegundos que dariam 20 minutos
int Tcl = 5000;  // um minutos para teste
// const int Tsa = 300000;  // trezentos mil milissegundos que dariam 5 minutos
int const Tsa = 10000;
// const int Tdl = 120000;  // cento e vinte mil milissegundos que dariam 2 minutos
int Tdl = 5000;
// const int Tel = 120000;  // cento e vinte mil milissegundos que dariam 2 minutos
int Tel = 5000;

int FlagEstado = 0;

// Definindo os Est,
enum Estado {
  A,
  B,
  C,
  D,
  E,
  F
};

Estado estado;

const char *ssid = "Giovana";
const char *password = "ekls9466";

WiFiServer server(80);

String msg;

void callback(const char *topic, byte *payload, unsigned int length) {
  //Serial.println("callback");
  if ((String)topic == "Tcl") {

    String valor = "";
    valor = msg;

    for (int i = 0; i < length; i++) {
      msg += (char)payload[i];
    }

    Tcl = msg.toInt();
    Tcl = Tcl * 1000;
    Serial.println(Tcl);
  } else {
    if ((String)topic == "Tdl") {

      String valor = "";
      valor = msg;

      for (int i = 0; i < length; i++) {
        msg += (char)payload[i];
      }

      Tdl = msg.toInt();
      Tdl = Tdl * 1000;
      Serial.println(Tdl);
    } else {
      if ((String)topic == "Tel") {

        String valor = "";
        valor = msg;

        for (int i = 0; i < length; i++) {
          msg += (char)payload[i];
        }

        Tel = msg.toInt();
        Tel = Tel * 1000;
        Serial.println(Tel);
      }
    }
  }

  //delay(750);
}

int TempoS(int segundos) {
  int TempoSecundos;
  TempoSecundos = segundos * 10000;

  return TempoSecundos;
}

void setup() {

  // pinMode(LED, OUTPUT);
  Serial.begin(115200);
  delay(10);

  t1 = millis();              // iniciando t1 com o tempo de agora
  lcd.begin();           // Iniciando a variavel que controlara o Display
  pinMode(L, INPUT);          // Definindo Input Botão ON OFF
  pinMode(ST, INPUT_PULLUP);  // Definindo Input Sensor de Temperatura
  pinMode(SP, INPUT_PULLUP);  // Definindo Input Sensor de Porta
  pinMode(VS, OUTPUT);        // Definindo Output Valvula de Agua Superior
  pinMode(VI, OUTPUT);        // Definindo Output Valvula de Agua Inferior
  pinMode(AQ, OUTPUT);        // Definindo Output Dispositivo Aquecedor
  pinMode(buzzer, OUTPUT);

  t2 = millis();

  // Maquina Inicia no Estado Desligado
  estado = A;
  //lcd.init();

  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");

  // Conectar ao HiveMQ
  cliente.setClient(wifiClient);
  cliente.setServer("broker.hivemq.com", 1883);
  cliente.connect("Esp32", "MLLMQTT", "122110");
  cliente.subscribe("Estados");
  cliente.subscribe("Tcl");
  cliente.subscribe("Tdl");
  cliente.subscribe("Tel");
  cliente.setCallback(callback);
}

void loop() {

  if (!cliente.connected()) {
    cliente.connect("Esp32", "MLLMQTT", "122110");
    cliente.subscribe("Tcl");
    cliente.subscribe("Tdl");
    cliente.subscribe("Tel");
    cliente.setCallback(callback);
  }

  delay(200);
  msg = "";
  cliente.loop();
  //delay(1);

  t2 = millis();  // Atualizando t2

  if (estado == A) {
    if (FlagEstado == 0) {
      cliente.publish("Estados", "Desligado");
      msg = "";
      msg = ("Desligado");
      FlagEstado = 1;
    }
    digitalWrite(VS, LOW);  // Definindo a Valvula de Agua Superior
    digitalWrite(VI, LOW);
    digitalWrite(AQ, LOW);
    digitalWrite(buzzer, LOW);
    lcd.clear();
    lcd.setBacklight(HIGH);
    lcd.setCursor(0, 0);
    lcd.print("Desligado");
    Serial.println("Desligado");


    if (digitalRead(L) == 1 and digitalRead(SP) == 1) {
      // Fechando a Porta e Acionando o Botão ON OFF para Alto
      estado = B;
      FlagEstado = 0;
    } else {
      estado = A;
    }
  }

  // Segundo Estado B = Aquecendo
  if (estado == B) {

    if (FlagEstado == 0) {
      cliente.publish("Estados", "Aquecendo");
      msg = "";
      msg = ("Aquecendo");
      FlagEstado = 1;
    }

    digitalWrite(VI, LOW);
    digitalWrite(VS, LOW);
    digitalWrite(AQ, HIGH);
    digitalWrite(buzzer, LOW);
    lcd.clear();
    lcd.setBacklight(HIGH);
    lcd.setCursor(0, 0);
    lcd.print("Aquecendo");
    Serial.println("Aquecendo");

    if (digitalRead(ST) == 1) {
      estado = C;
      FlagEstado = 0;
      t1 = millis();
    }

    // Condição caso o usuario abra a porta a maquina apertando o Botão ON OFF em baixo
    if (digitalRead(SP) == 0) {
      estado = A;
      cliente.publish("Estados", "Desligado");
      digitalWrite(buzzer, HIGH);
      lcd.clear();
      lcd.setBacklight(HIGH);
      lcd.setCursor(0, 0);
      lcd.print("Desligado");
      delay(1000);
    }
  }
  // Terceiro Estado C = Ciclo de Lavagem
  if (estado == C) {
    if (FlagEstado == 0) {
      cliente.publish("Estados", "Ciclo de Lavagem");
      msg = "";
      msg = ("Ciclo de Lavagem");
      FlagEstado = 1;
    }
    digitalWrite(VS, HIGH);
    digitalWrite(VI, HIGH);
    digitalWrite(AQ, LOW);
    digitalWrite(buzzer, LOW);

    lcd.clear();
    lcd.setBacklight(HIGH);
    lcd.setCursor(0, 0);
    lcd.print("Ciclo de Lavagem");
    Serial.println("Ciclo de Lavagem");
    Serial.println(Tcl);

    // transformar milisegundos em segundo
    int Temporizador = TempoS(Tcl);
    Temporizador = Tcl / 1000;

    // imprimir tempo do ciclo de lavagem no lcd
    if ((t2 - t1) <= Tcl) {
      do {
        Temporizador--;
        lcd.setCursor(0, 1);
        lcd.print("Tempo: ");
        lcd.print("                ");
        lcd.setCursor(7, 1);
        lcd.print(Temporizador);
        delay(1000);
      } while (digitalRead(SP) == 1 && Temporizador);
      estado = D;
      t1 = millis();  // Atualizando t1 apos o encerramento da Saida de Agua para ultilizar o parametro Tdl - Tempo de Dispersão do Liquido
      FlagEstado = 0;
    }

    // Condição caso o usuario abra a porta a maquina apertando o Botão ON OFF em baixo
    if (digitalRead(SP) == 0) {
      cliente.publish("Estados", "Desligado");
      digitalWrite(buzzer, HIGH);
      estado = A;
      lcd.clear();
      lcd.setBacklight(HIGH);
      lcd.setCursor(0, 0);
      lcd.print("Desligado");
      delay(1000);
    }
  }

  // Quarto Estado D = Saida de Agua
  if (estado == D) {

    if (FlagEstado == 0) {
      cliente.publish("Estados", "Saida de Agua");
      msg = "";
      msg = ("Saida de Agua");
      FlagEstado = 1;
    }
    digitalWrite(VS, LOW);
    digitalWrite(VI, LOW);
    digitalWrite(AQ, LOW);
    digitalWrite(buzzer, LOW);

    lcd.clear();
    lcd.setBacklight(HIGH);
    lcd.setCursor(0, 0);
    lcd.print("Saida de Agua");
    Serial.println("Saida de Agua");

    // transformar milisegundos em segundo
    int seg = Tsa / 1000;

    // imprimir tempo do ciclo de lavagem no lcd
    if ((t2 - t1) <= Tsa) {
      do {
        seg--;
        lcd.setCursor(0, 1);
        lcd.print("Tempo: ");
        lcd.print("                ");
        lcd.setCursor(7, 1);
        lcd.print(seg);
        delay(1000);
      } while (digitalRead(SP) == 1 && seg);
      estado = E;
      t1 = millis();  // Atualizando t1 apos o encerramento da Saida de Agua para ultilizar o parametro Tdl - Tempo de Dispersão do Liquido
      FlagEstado = 0;
    }

    // Condição caso o usuario abra a porta a maquina apertando o Botão ON OFF em baixo
    if (digitalRead(SP) == 0) {
      cliente.publish("Estados", "Desligado");
      //delay(1000);
      digitalWrite(buzzer, LOW);
      estado = A;
      lcd.clear();
      lcd.setBacklight(HIGH);
      lcd.setCursor(0, 0);
      lcd.print("Desligado");
      delay(1000);
    }
  }

  // Quinto Estado E = Dispersão do Liquido Secante
  if (estado == E) {

    if (FlagEstado == 0) {
      cliente.publish("Estados", "Dispersao Liquido Secante");
      msg = "";
      msg = ("Dispersao Liquido Secante");
      FlagEstado = 1;
    }
    digitalWrite(VS, LOW);
    digitalWrite(VI, LOW);
    digitalWrite(AQ, LOW);
    digitalWrite(buzzer, LOW);
    lcd.clear();
    lcd.setBacklight(HIGH);
    lcd.setCursor(0, 0);
    lcd.print("Dispersao Liquido");
    Serial.println("Dispersao Liquido Secante");
    Serial.println(Tdl);

    // transformar milisegundos em segundo
    int seg = Tdl / 1000;

    // imprimir tempo do ciclo de lavagem no lcd
    if ((t2 - t1) <= Tdl) {
      do {
        seg--;
        lcd.setCursor(0, 1);
        lcd.print("Tempo: ");
        lcd.print("                ");
        lcd.setCursor(7, 1);
        lcd.print(seg);
        delay(1000);
      } while (digitalRead(SP) == 1 && seg);
      estado = F;
      t1 = millis();  // Atualizando t1 apos o encerramento da dispersão do Liquido e inicio do escoamento do mesmo pelo Tempo de Escoamento do Liquido.
      FlagEstado = 0;
    }

    // Condição caso o usuario abra a porta a maquina apertando o Botão ON OFF em baixo
    if (digitalRead(SP) == 0) {

      cliente.publish("Estados", "Desligado");
      //delay(1000);
      digitalWrite(buzzer, HIGH);
      estado = A;
      lcd.clear();
      lcd.setBacklight(HIGH);
      lcd.setCursor(0, 0);
      lcd.print("Desligado");
      delay(1000);
    }
  }

  if (estado == F) {

    if (FlagEstado == 0) {
      cliente.publish("Estados", "Escoamento do Liquido Secante");
      msg = "";
      msg = ("Escoamento do Liquido Secante");
      FlagEstado = 1;
    }
    digitalWrite(VS, LOW);
    digitalWrite(VI, LOW);
    digitalWrite(AQ, LOW);
    digitalWrite(buzzer, LOW);

    lcd.clear();
    lcd.setBacklight(HIGH);
    lcd.setCursor(0, 0);
    lcd.print("Escoamento do Liquido Secante");
    Serial.println("Escoamento do Liquido Secante");
    Serial.println(Tel);
    // transformar milisegundos em segundo
    int seg = Tel / 1000;

    // imprimir tempo do ciclo de lavagem no lcd
    if ((t2 - t1) <= Tel) {
      do {
        seg--;
        lcd.setCursor(0, 1);
        lcd.print("Tempo: ");
        lcd.print("                ");
        lcd.setCursor(7, 1);
        lcd.print(seg);
        delay(1000);
      } while (digitalRead(SP) == 1 && seg);
      estado = A;
      t1 = millis();  // Atualizando t1 apos o encerramento da dispersão do Liquido e inicio do escoamento do mesmo pelo Tempo de Escoamento do Liquido.
      FlagEstado = 0;
    }
    // Condição caso o usuario abra a porta a maquina apertando o Botão ON OFF em baixo
    if (digitalRead(SP) == 0) {
      digitalWrite(buzzer, HIGH);
      cliente.publish("Estados", "Desligado");
      //delay(1000);
      estado = A;
      lcd.clear();
      lcd.setBacklight(HIGH);
      lcd.setCursor(0, 0);
      lcd.print("Desligado");
      delay(1000);
    }
  }
}
