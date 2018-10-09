/****************************************************************************
'*  Arquivo : robo_SI.ino                                                   *
'*  Name    : Controlador de robo luta estilo curso SI                      *
'*  Author  : Mauricio Alves Oliveira Neto                                  *
'*  Notice  : Copyright (c) 2018 - Mauricio Alves Oliveira Neto             *
'*  E-mail  : mauricioaonet@hotmail.com                                     *
'*          : All Rights Reserved                                           *
'*  Date    : 30/08/2018                                                    *
'*  Version : 1.0 Versão para Arduino 30/08/2018                            *
'*  Notes   :                                                               *
'*          :                                                               *
'***************************************************************************/
#include <AFMotor.h>
#include <Servo.h>

//Motores de movimento do robo
AF_DCMotor motor1(1);
AF_DCMotor motor2(2);

//AF_DCMotor motor_ARMA_A(3);
//AF_DCMotor motor_ARMA_B(4);

Servo SERVO_ARMA_DIREITA;
Servo SERVO_ARMA_ESQUERDA;

//Habilita modo de teste para motores e driver de motor DC
const bool MODO_TESTE = false;

//Ativar arma do robo (segunda fase do projeto)
const bool ATIVAR_ARMA = true;

//CPU Load > para piscar em trabalho de processamento! (para analize do codigo)
const bool CPU_LOAD_ATIVAR = true;

//CPU Load PIN (porta padrão do LED no arduino)
const byte CPULOAD_PIN = 13;

//Implementar arma no robo
const byte ARMA_SENSOR = A0;

//const int TEMPO_ATIVAR_ARMA    = 1000;
//const int TEMPO_DESATIVAR_ARMA = 1000;
//const int TEMPO_CORRIGIR_ARMA  = 100;

//Variaveis para a Arma
bool armaAtiva = false;
int tempoArma = 0;

//Variaveis tratamento do debug
byte lastMenssagem = 0;

const byte VAZIO    = 0;
const byte FRENTE   = 1;
const byte REVERSO  = 2;
const byte ESQUERDA = 3;
const byte DIREITA  = 4;
const byte PARAR    = 5;

const byte COMANDO_NAO_ENCONTRADO = 6;
const byte ARMA_ATIVADA    = 7;
const byte ARMA_DESATIVADA = 8;
const byte SENSOR_ATIVADO  = 9;


void setup() {
  
  pinMode(ARMA_SENSOR,INPUT_PULLUP);
  pinMode(CPULOAD_PIN, OUTPUT);
   
  SERVO_ARMA_DIREITA.attach(9);
  SERVO_ARMA_ESQUERDA.attach(10);

  //Configuração da porta Serial para Bluetooth
  Serial.begin(9600);

  //Colocar motores na velocidade maxima
  motor1.setSpeed(200);
  motor2.setSpeed(200);
  
//  motor_ARMA_A.setSpeed(200);
//  motor_ARMA_B.setSpeed(200);

  //Colocar os motores em estado parados
  motor1.run(RELEASE);
  motor2.run(RELEASE);

//  motor_ARMA_A.run(RELEASE);
//  motor_ARMA_B.run(RELEASE);

  desativaArma();
  delay(200); 

  //ROBO iniciado
  Serial.println("ROBO SI INICIADO:");

  if (ATIVAR_ARMA == true){
    Serial.println("MODE DE COMBATE INICIADO, ARMA ATIVADA!");
  }

  if(MODO_TESTE == true)
  {
    bool rev = true;
    byte load = 1;
    Serial.println("MODO DE TESTE DE MOTORES INICIADO!");
    
    while(1)
    {
      if (load == 1){
        Serial.println("");
        load = 30;
        
        if (rev == true){
          moverRobo('w');
          rev = false;
        }else{
          moverRobo('s');
          rev = true;     
        }
      }
      
      if (ATIVAR_ARMA == true){
        if (digitalRead(ARMA_SENSOR) == LOW){ 
          enviarMenssagem(SENSOR_ATIVADO);
          ativaArma();
          
          while(digitalRead(ARMA_SENSOR) == LOW){
             
          }
          desativaArma();
        }
      }
      
      Serial.print(".");
      load--;
      cpuLoad();
      
      delay(100);
    } 
  }
}

void loop() {
  delay(100);
  
  if(Serial.available()>0)
  {    
    char data = Serial.read();
    moverRobo(data);
  }
  else
  {
    //moverRobo('q');
  }

  
  if (ATIVAR_ARMA == true){
    if (digitalRead(ARMA_SENSOR) == LOW){ 
      if (armaAtiva == false){
        armaAtiva = true;
        tempoArma = 150;
      }
    }
    
    if (armaAtiva == true){
      switch(tempoArma){
        case 150:
          cpuLoad();
          ativaArma();
          break;
          
//        case 250:
//          cpuLoad();
//          ativaArma(TEMPO_CORRIGIR_ARMA);
//          break;
//          
//        case 200:
//          cpuLoad();
//          ativaArma(TEMPO_CORRIGIR_ARMA);
//          break;
//          
//        case 150:
//          cpuLoad();
//          ativaArma(TEMPO_CORRIGIR_ARMA);
//          break;
      }
      //subtrair 100ms do tempo
      tempoArma--;
  
      //verificar flag do tempo para setar condições
      if(tempoArma == 0){
        armaAtiva = false;
        desativaArma();
      }
    }
  }
  cpuLoad(); 
}


//Nova função de movimentação!
void moverRobo(char dir)
{
  switch(dir)
  {
    case 'a': 
      enviarMenssagem(ESQUERDA);
      motor1.run(RELEASE);
      motor2.run(FORWARD);
      break; 
      
    case 'w': 
      enviarMenssagem(FRENTE);
      motor1.run(FORWARD);
      motor2.run(FORWARD);
      break; 
      
    case 's': 
      enviarMenssagem(REVERSO);
      motor1.run(BACKWARD);
      motor2.run(BACKWARD);
      break; 
      
    case 'd': 
      enviarMenssagem(DIREITA);
      motor1.run(FORWARD);
      motor2.run(RELEASE);
      break; 
      
    case 'q': 
      //enviarMenssagem(PARAR);
      motor1.run(RELEASE);
      motor2.run(RELEASE);
      break; 
      
    default : 
      enviarMenssagem(COMANDO_NAO_ENCONTRADO);
      motor1.run(RELEASE);
      motor2.run(RELEASE);
      break;
  }
}


void ativaArma(){
  enviarMenssagem(ARMA_ATIVADA);
 
  SERVO_ARMA_DIREITA.write(180);
  SERVO_ARMA_ESQUERDA.write(1);
  delay(20); 
  
//  motor_ARMA_A.run(FORWARD);
//  motor_ARMA_B.run(FORWARD);
//  delay(tempo);
//  pararArma(1);
}

void desativaArma(){
  enviarMenssagem(ARMA_DESATIVADA);

  SERVO_ARMA_DIREITA.write(1);
  SERVO_ARMA_ESQUERDA.write(180);
  delay(20); 
  
//  motor_ARMA_A.run(BACKWARD);
//  motor_ARMA_B.run(BACKWARD);
//  delay(tempo);
}

//void pararArma(int tempo){
//  enviarMenssagem(ARMA_DESATIVADA);
//  motor_ARMA_A.run(RELEASE);
//  motor_ARMA_B.run(RELEASE);
//  delay(tempo);
//}

void enviarMenssagem(byte menssagem){
  if (menssagem != lastMenssagem){
    switch(menssagem)
    {
      case VAZIO:
        Serial.println("");
        break;
        
      case FRENTE:
        Serial.println("FRENTE");
        break;
        
      case REVERSO:
        Serial.println("REVERSO");
        break;
        
      case ESQUERDA:
        Serial.println("ESQUERDA");
        break;
        
      case DIREITA:
        Serial.println("DIREITA");
        break;
        
      case PARAR:
        Serial.println("PARAR");
        break;
        
      case COMANDO_NAO_ENCONTRADO:
        Serial.println("COMANDO NAO ENCONTRADO");
        break;
        
      case ARMA_ATIVADA:
        Serial.println("ARMA ATIVADA");
        break;
        
      case ARMA_DESATIVADA:
        Serial.println("ARMA DASATIVADA");
        break;

      case SENSOR_ATIVADO:
        Serial.print("SENSOR ATIVADO...");
        break;
    }
  }
  else{
    lastMenssagem = menssagem;
  }
}

void cpuLoad(){
  if (CPU_LOAD_ATIVAR == true){
    digitalWrite(CPULOAD_PIN, !digitalRead(CPULOAD_PIN));
  }
}


