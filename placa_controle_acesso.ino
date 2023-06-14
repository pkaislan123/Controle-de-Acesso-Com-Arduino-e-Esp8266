#include <Wiegand.h>
#include <Keypad.h>
//#include "LoRa_E220.h"
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <MemoryFree.h>


//#define E220_22
//#define FREQUENCY_915

#define pinoRele1 A2
#define pinoRele2 A3
#define pinoLedTx A0
#define pinoLedRx A1

#define rxPinLora 11
#define txPinLora 12
#define numero_serie "33322"
#define i_numero_serie 33322


#define rxPin 8
#define txPin 9
SoftwareSerial wifi(rxPin, txPin);  // RX, TX




LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);


char teclado[3][3] =  // Definir as teclas
{
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' }
};

byte rowPins[3] = { 5, 4, 10 };  // Pinos digitais relacionados as linhas.
byte colPins[3] = { 6, 7, 13 };  // Pinos digitais relacionados as colunas.

Keypad meuteclado = Keypad(makeKeymap(teclado), rowPins, colPins, 3, 3);


WIEGAND wg;


int tempo_intervalo_online = 15000;
long unsigned int tempo_ultima_msg_online = millis();


//LoRa_E220 ctrl_acesso(rxPinLora, txPinLora);


long tempo_acionamento_rele_remoto1 = 30000;
long unsigned int tempo_acionado_rele_remoto1 = millis();

long tempo_acionamento_rele_remoto2 = 30000;
long unsigned int tempo_acionado_rele_remoto2 = millis();

long unsigned int tempo = millis();


boolean rele_remoto1_ativo = false;
boolean rele_remoto2_ativo = false;


String linha1Global = "";
String linha1GlobalOffline = "";


int stringStart, stringStop = 0;
int scrollCursor = 16;


String dataHoraGlobal = "01/01/01 00:00";


int disparar_acesso_negado = 1;
int acionar_rele1_liberado = 1;
int acionar_rele2_liberado = 1;
int tempo_acionamento_liberado_rele1 = 5;
int multiplicador_tempo_rele1 = 3;
int tempo_acionamento_liberado_rele2 = 5;
int multiplicador_tempo_rele2 = 3;
int senha_master = 9876;


boolean menu_principal = true;
boolean menu_offline = false;
boolean menu_aguarde = true;

long unsigned int tempo_menu_offline = millis();
long unsigned int tempo_menu_aguarde = millis();

int tokens_globais[10] = { 0 };

boolean menu_senha = false;
String senha_global = "";

long unsigned int tempo_menu_senha = millis();


boolean acionou_rele_individual = false;

long unsigned int tUVAreleInd = millis();


long unsigned int tempo_restposta_wifi = millis();
boolean enviou_uid = false;

void setup() {
  //salvarSenhaMestra(1515);

  // ctrl_acesso.begin();
  delay(200);


  Serial.begin(115200);
  delay(200);


  wifi.begin(38400);
  delay(200);


  wg.begin();


  pinMode(pinoRele1, OUTPUT);
  pinMode(pinoRele2, OUTPUT);
  pinMode(pinoLedTx, OUTPUT);
  pinMode(pinoLedRx, OUTPUT);


  digitalWrite(pinoRele1, HIGH);
  digitalWrite(pinoRele2, HIGH);



  digitalWrite(pinoLedTx, HIGH);
  digitalWrite(pinoLedRx, HIGH);
  delay(1000);
  digitalWrite(pinoLedTx, LOW);
  digitalWrite(pinoLedRx, LOW);

  lcd.begin(16, 2);
  limparLinha(0);
  lcd.setCursor(0, 0);
  lcd.print(dataHoraGlobal);

  menu_principal = true;
  carregarEeprom();




  Serial.println("CTRL Iniciado");


  randomSeed(analogRead(0));
}

void loop() {

  //ler valores do arduino
  if (wifi.available()) {
    String texto_recebido = wifi.readString();
    lerNodemcu(texto_recebido);
  }

  if (wg.available()) {
    /*
    menu_principal = false;
    menu_offline = false;
    menu_aguarde = true;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Aguarde...");
    tempo_menu_aguarde = millis();

    */

    String uid = String(wg.getCode(), HEX);
    Serial.print("Hex = ");
    Serial.println(uid);
    String mensagem = "@";
    mensagem.concat(uid);
    mensagem.concat("$");
    wifi.println(mensagem);

    tempo_restposta_wifi = millis();
    enviou_uid = true;
  }


  /*
    changeMenu();

    if (wg.available()) {
      menu_principal = false;
      menu_offline = false;
      menu_aguarde = true;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Aguarde...");
      tempo_menu_aguarde = millis();



      String uid = String(wg.getCode(), HEX);
      Serial.print("Hex = ");
      Serial.println(uid);
      String mensagem = "@";
      mensagem.concat(uid);
      mensagem.concat("$");
      wifi.println(mensagem);

      tempo_restposta_wifi = millis();
      enviou_uid = true;
    }



    verificarEstadoReles();



    if (menu_principal) {


      imprimeScroll(dataHoraGlobal, linha1Global);
    }

    if (menu_offline) {
      if (millis() - tempo_menu_offline > 50000) {
        menu_offline = false;
        menu_aguarde = false;
        menu_senha = false;
        menu_principal = true;
        resetScroll();
      } else {


        imprimeScroll("SISTEMA OFFLINE", linha1GlobalOffline);
      }
    }

    if (menu_aguarde) {
      if (millis() - tempo_menu_aguarde > 10000) {
        menu_offline = false;
        menu_senha = false;
        menu_aguarde = false;
        menu_principal = true;
        resetScroll();
      }
    }


    if (menu_senha) {
      if (millis() - tempo_menu_senha > 15000) {
        senha_global = "";
        menu_senha = false;
        menu_offline = false;
        menu_aguarde = false;
        menu_principal = true;
        resetScroll();
      } else {
        if (senha_global.length() == 4) {
          int senha_int = senha_global.toInt();
          if (senha_int > 0) {

            boolean comparacaoTokens = compareSenha(senha_int);
            boolean comparacaoSenhaMestre = compareSenhaMestre(senha_int);
            if (comparacaoTokens || comparacaoSenhaMestre) {


              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Acesso Liberado!");

              /*
                          //enviar comando via lora para desarmar e rearmar a central
                          String texto_comando = "+33322";
                          texto_comando.concat("&700%|");
                          char mensagemArray[texto_comando.length() + 1];
                          texto_comando.toCharArray(mensagemArray, texto_comando.length() + 1);
                          ctrl_acesso.sendMessage(mensagemArray);


              if (acionar_rele1_liberado == 1) {
                digitalWrite(pinoRele1, LOW);
              }


              if (acionar_rele2_liberado == 1) {
                digitalWrite(pinoRele2, LOW);
              }

              if (acionar_rele1_liberado == 1 && acionar_rele2_liberado == 1) {
                if (tempo_acionamento_liberado_rele1 < tempo_acionamento_liberado_rele2) {
                  delay(tempo_acionamento_liberado_rele1);
                  digitalWrite(pinoRele1, HIGH);
                  delay(tempo_acionamento_liberado_rele2 - tempo_acionamento_liberado_rele1);
                  digitalWrite(pinoRele2, HIGH);
                } else {
                  delay(tempo_acionamento_liberado_rele2);
                  digitalWrite(pinoRele2, HIGH);
                  delay(tempo_acionamento_liberado_rele1 - tempo_acionamento_liberado_rele2);
                  digitalWrite(pinoRele1, HIGH);
                }
              } else if (acionar_rele1_liberado == 1 && acionar_rele2_liberado == 0) {
                delay(tempo_acionamento_liberado_rele1);
                digitalWrite(pinoRele1, HIGH);

              } else if (acionar_rele1_liberado == 0 && acionar_rele2_liberado == 1) {
                delay(tempo_acionamento_liberado_rele2);
                digitalWrite(pinoRele2, HIGH);
              }


              senha_global = "";
              menu_senha = false;
              menu_offline = false;
              menu_aguarde = false;
              menu_principal = true;
              resetScroll();
              delay(2000);


            } else {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Senha Incorreta");
              senha_global = "";
              menu_senha = false;
              menu_offline = false;
              menu_aguarde = false;
              menu_principal = true;
              resetScroll();
              delay(2000);
            }

          } else {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Senha Invalida");
            senha_global = "";
            menu_senha = false;
            menu_offline = false;
            menu_aguarde = false;
            menu_principal = true;
            resetScroll();
            delay(2000);
          }
        }
      }
    }
  */

  //avisarOnline();

  if (enviou_uid) {
    if (millis() - tempo_restposta_wifi > 20000) {
      //se nao houv respostas, esta offline, avisar
      enviou_uid = false;
      lerNodemcu("%99&");
    }
  }


}  //fim do loop




void lerNodemcu(String texto_recebido) {

  Serial.print("Requicao: ");
  Serial.println(texto_recebido);

  int arroba = texto_recebido.indexOf("@");
  int hifen = texto_recebido.indexOf("-");
  int asterisco = texto_recebido.indexOf("*");
  int e_com = texto_recebido.indexOf("&");
  int hashtag = texto_recebido.indexOf("#");
  int sifrao = texto_recebido.indexOf("$");
  int mais = texto_recebido.indexOf("+");
  int porcentagem = texto_recebido.indexOf("%");
  int fim_chave = texto_recebido.indexOf("}");
  int a = texto_recebido.indexOf("A");
  int b = texto_recebido.indexOf("B");

  int z = texto_recebido.indexOf("Z");
  int y = texto_recebido.indexOf("Y");

  int w = texto_recebido.indexOf("W");
  int k = texto_recebido.indexOf("K");

  /*
    if (w == 0 && a > 0 && k > 0 && sifrao > 0) {
      //Requisicao para salvar tokens
      String tipo_requisicao = texto_recebido.substring(w + 1, a);
      String id_requisicao = texto_recebido.substring(k + 1, sifrao);

      int Itipo_requisicao = tipo_requisicao.toInt();
      long Iid_requisicao = id_requisicao.toInt();


      if (Itipo_requisicao > 0 && Iid_requisicao > 0) {
        if (Itipo_requisicao == 70) {
          //dividir tokens
          int b = texto_recebido.indexOf("B");
          int c = texto_recebido.indexOf("C");
          int d = texto_recebido.indexOf("D");
          int e = texto_recebido.indexOf("E");
          int f = texto_recebido.indexOf("F");
          int g = texto_recebido.indexOf("G");
          int h = texto_recebido.indexOf("H");
          int i = texto_recebido.indexOf("I");
          int j = texto_recebido.indexOf("J");

          int tokens_locais[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

          tokens_locais[0] = texto_recebido.substring(a + 1, b).toInt();
          tokens_locais[1] = texto_recebido.substring(b + 1, c).toInt();
          tokens_locais[2] = texto_recebido.substring(c + 1, d).toInt();
          tokens_locais[3] = texto_recebido.substring(d + 1, e).toInt();
          tokens_locais[4] = texto_recebido.substring(e + 1, f).toInt();
          tokens_locais[5] = texto_recebido.substring(f + 1, g).toInt();
          tokens_locais[6] = texto_recebido.substring(g + 1, h).toInt();
          tokens_locais[7] = texto_recebido.substring(h + 1, i).toInt();
          tokens_locais[8] = texto_recebido.substring(i + 1, j).toInt();
          tokens_locais[9] = texto_recebido.substring(j + 1, k).toInt();

          for (int i = 0; i <= 9; i++) {
            Serial.print("Token na posicao ");
            Serial.print(i);
            Serial.print(":");
            Serial.println(tokens_locais[i]);
          }

          //salvar os tokens
          salvarToken(tokens_locais);
          delay(1000);
          lerTokens();

          String mensagem_responder_requisicao = "@";

          mensagem_responder_requisicao.concat("71");
          mensagem_responder_requisicao.concat("&");
          mensagem_responder_requisicao.concat(id_requisicao);
          mensagem_responder_requisicao.concat("#");

          wifi.println(mensagem_responder_requisicao);
        }
      }
    }
  */
  //controle de acesso
  if (arroba == 0 && hashtag > 0 && sifrao > 0) {

    Serial.println("Requisicao ctrl acesso");

    Serial.print("indice @: ");
    Serial.println(arroba);

    Serial.print("indice #: ");
    Serial.println(hashtag);

    Serial.print("indice $: ");
    Serial.println(sifrao);

    String tipo_requisicao = texto_recebido.substring(arroba + 1, hashtag);
    String id_requisicao = texto_recebido.substring(hashtag + 1, sifrao);

    int Itipo_requisicao = tipo_requisicao.toInt();
    long Iid_requisicao = id_requisicao.toInt();


    if (Itipo_requisicao > 0 && Iid_requisicao > 0) {
      if (Itipo_requisicao == 17) {
        //acesso negado
        Serial.println("Acesso Negado!");
        enviou_uid = false;


        String mensagem_responder_requisicao = "@";

        mensagem_responder_requisicao.concat("19");
        mensagem_responder_requisicao.concat("&");
        mensagem_responder_requisicao.concat(id_requisicao);
        mensagem_responder_requisicao.concat("#");

        wifi.println(mensagem_responder_requisicao);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Acesso Negado!");

        /*
          if (disparar_acesso_negado) {
          String texto_live = "*";
          texto_live.concat(numero_serie);
          texto_live.concat("-1#|");

          char mensagemArray[texto_live.length() + 1];
          texto_live.toCharArray(mensagemArray, texto_live.length() + 1);
          ctrl_acesso.sendMessage(mensagemArray);
          }
        */

        delay(3000);

        menu_principal = true;

      } else if (Itipo_requisicao == 16) {
        //acesso liberado
        Serial.println("Acesso Liberado!");
        enviou_uid = false;

        String mensagem_responder_requisicao = "@";

        mensagem_responder_requisicao.concat("18");
        mensagem_responder_requisicao.concat("&");
        mensagem_responder_requisicao.concat(id_requisicao);
        mensagem_responder_requisicao.concat("#");

        wifi.println(mensagem_responder_requisicao);


        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Acesso Liberado!");

        if (acionar_rele1_liberado == 1) {
          digitalWrite(pinoRele1, LOW);
        }


        if (acionar_rele2_liberado == 1) {
          digitalWrite(pinoRele2, LOW);
        }

        if (acionar_rele1_liberado == 1 && acionar_rele2_liberado == 1) {
          if (tempo_acionamento_liberado_rele1 < tempo_acionamento_liberado_rele2) {
            delay(tempo_acionamento_liberado_rele1);
            digitalWrite(pinoRele1, HIGH);
            delay(tempo_acionamento_liberado_rele2 - tempo_acionamento_liberado_rele1);
            digitalWrite(pinoRele2, HIGH);
          } else {
            delay(tempo_acionamento_liberado_rele2);
            digitalWrite(pinoRele2, HIGH);
            delay(tempo_acionamento_liberado_rele1 - tempo_acionamento_liberado_rele2);
            digitalWrite(pinoRele1, HIGH);
          }
        } else if (acionar_rele1_liberado == 1 && acionar_rele2_liberado == 0) {
          delay(tempo_acionamento_liberado_rele1);
          digitalWrite(pinoRele1, HIGH);

        } else if (acionar_rele1_liberado == 0 && acionar_rele2_liberado == 1) {
          delay(tempo_acionamento_liberado_rele2);
          digitalWrite(pinoRele2, HIGH);
        }

        delay(4000);
        senha_global = "";
        menu_senha = false;
        menu_offline = false;
        menu_aguarde = false;
        menu_principal = true;
        resetScroll();
      }
    }

  }  //LIgar algum dos reles
  //&33+2#45445$    &tipoR+rele+tempo_em-seg#id_re$
  if (e_com == 0 && mais > 0 && hashtag > 0 && sifrao > 0) {
    String tipo_requisicao = texto_recebido.substring(e_com + 1, mais);
    String numero_rele = texto_recebido.substring(mais + 1, hashtag);
    String id_requisicao = texto_recebido.substring(hashtag + 1, sifrao);

    int Itipo_requisicao = tipo_requisicao.toInt();
    int INumero_rele = numero_rele.toInt();
    long Iid_requisicao = id_requisicao.toInt();


    if (Itipo_requisicao > 0 && INumero_rele > 0 && Iid_requisicao > 0) {
      if (Itipo_requisicao == 33) {
        //acionar algum rele
        if (INumero_rele == 1) {


          digitalWrite(pinoLedRx, HIGH);
          delay(500);
          digitalWrite(pinoLedRx, LOW);

          if (millis() - tUVAreleInd > 10000) {
            Serial.print("tempo acionamento rele 1: ");
            Serial.println(tempo_acionamento_liberado_rele1);

            digitalWrite(pinoRele1, LOW);
            delay(tempo_acionamento_liberado_rele1);
            digitalWrite(pinoRele1, HIGH);

            Serial.print("desligou ");

            acionou_rele_individual = true;
            tUVAreleInd = millis();
          }

          String mensagem_responder_requisicao = "@";

          mensagem_responder_requisicao.concat("34");
          mensagem_responder_requisicao.concat("&");
          mensagem_responder_requisicao.concat(id_requisicao);
          mensagem_responder_requisicao.concat("#");

          wifi.println(mensagem_responder_requisicao);

        } else if (INumero_rele == 2) {
          digitalWrite(pinoLedRx, HIGH);
          delay(500);
          digitalWrite(pinoLedRx, LOW);

          if (millis() - tUVAreleInd > 10000) {
            Serial.print("tempo acionamento rele 2: ");
            Serial.println(tempo_acionamento_liberado_rele2);


            digitalWrite(pinoRele2, LOW);
            delay(tempo_acionamento_liberado_rele2);
            digitalWrite(pinoRele2, HIGH);

            Serial.print("desligou ");
            acionou_rele_individual = true;
            tUVAreleInd = millis();
          }

          String mensagem_responder_requisicao = "@";

          mensagem_responder_requisicao.concat("34");
          mensagem_responder_requisicao.concat("&");
          mensagem_responder_requisicao.concat(id_requisicao);
          mensagem_responder_requisicao.concat("#");

          wifi.println(mensagem_responder_requisicao);
        }
      }
    }
  }

  //acesso negado offline
  if (porcentagem == 0 && e_com > 0) {
    String id_requisicao = texto_recebido.substring(porcentagem + 1, e_com);

    long Iid_requsicao = id_requisicao.toInt();
    if (Iid_requsicao > 0) {
      if (Iid_requsicao == 99) {
        enviou_uid = false;
        resetScroll();

        menu_principal = false;
        menu_aguarde = false;
        menu_offline = true;
        tempo_menu_offline = millis();
      }
    }
  }




  if (asterisco == 0 && fim_chave > 0) {
    String data_hora = texto_recebido.substring(asterisco + 1, fim_chave);
    dataHoraGlobal = "";
    dataHoraGlobal.concat(data_hora);
  }


  if (a == 0 && arroba > 0 && hifen > 0 && asterisco > 0 && hashtag > 0 && y > 0 && mais > 0 && z > 0 && porcentagem > 0 && sifrao > 0) {

    String tipo_requisicao = texto_recebido.substring(a + 1, arroba);
    String disparar = texto_recebido.substring(arroba + 1, hifen);
    String a_r_1 = texto_recebido.substring(hifen + 1, asterisco);
    String t_a_r_1 = texto_recebido.substring(asterisco + 1, hashtag);
    String m_r_1 = texto_recebido.substring(hashtag + 1, y);
    String a_r_2 = texto_recebido.substring(y + 1, mais);
    String t_a_r_2 = texto_recebido.substring(mais + 1, z);
    String m_r_2 = texto_recebido.substring(z + 1, porcentagem);
    String id_requisicao = texto_recebido.substring(porcentagem + 1, sifrao);

    int Itipo_requisicao = tipo_requisicao.toInt();
    int Idisparar = disparar.toInt();
    int Ia_r_1 = a_r_1.toInt();
    int It_a_r_1 = t_a_r_1.toInt();
    int Im_r_1 = m_r_1.toInt();
    int Ia_r_2 = a_r_2.toInt();
    int It_a_r_2 = t_a_r_2.toInt();
    int Im_r_2 = m_r_2.toInt();
    long Iid_requisicao = id_requisicao.toInt();

    if (Itipo_requisicao > 0 && Idisparar >= 0 && Ia_r_1 >= 0 && It_a_r_1 >= 0 && Im_r_1 >= 0 && Ia_r_2 >= 0 && It_a_r_2 >= 0 && Im_r_2 >= 0) {
      if (Itipo_requisicao == 40) {
        //  salvarConfiguracoes(int disparar, int acionar_r_1, int acionar_r_2, int t_a_l_r_1, int multiplicador1, int t_a_l_r_2, int multiplicador2)
        salvarConfiguracoes(Idisparar, Ia_r_1, Ia_r_2, It_a_r_1, Im_r_1, It_a_r_2, Im_r_2);
        carregarEeprom();

        String mensagem_responder_requisicao = "@";

        mensagem_responder_requisicao.concat("17");
        mensagem_responder_requisicao.concat("&");
        mensagem_responder_requisicao.concat(id_requisicao);
        mensagem_responder_requisicao.concat("#");

        wifi.println(mensagem_responder_requisicao);
      }
    }
  }


  //mudar frase online
  if (k == 0 && a > 0 && b > 0 && sifrao > 0) {

    String tipo_requisicao = texto_recebido.substring(k + 1, a);
    String mensagem = texto_recebido.substring(a + 1, b);
    String id_requisicao = texto_recebido.substring(b + 1, sifrao);

    int Itipo_requisicao = tipo_requisicao.toInt();
    long Iid_requisicao = id_requisicao.toInt();

    if (Itipo_requisicao > 0) {
      if (Itipo_requisicao == 80) {
        // Mudar frase online
        salvarInformacao(mensagem, 0);
        carregarEeprom();

        String mensagem_responder_requisicao = "@";

        mensagem_responder_requisicao.concat("17");
        mensagem_responder_requisicao.concat("&");
        mensagem_responder_requisicao.concat(id_requisicao);
        mensagem_responder_requisicao.concat("#");

        wifi.println(mensagem_responder_requisicao);

      } else if (Itipo_requisicao == 81) {
        // Mudar frase online
        salvarInformacao(mensagem, 200);
        carregarEeprom();

        String mensagem_responder_requisicao = "@";

        mensagem_responder_requisicao.concat("17");
        mensagem_responder_requisicao.concat("&");
        mensagem_responder_requisicao.concat(id_requisicao);
        mensagem_responder_requisicao.concat("#");

        wifi.println(mensagem_responder_requisicao);
      } else if (Itipo_requisicao == 82) {
        //mudar senha mestre

        int senha = mensagem.toInt();

        if (senha > 0) {
          salvarSenhaMestra(senha);
          carregarEeprom();
        }

        String mensagem_responder_requisicao = "@";

        mensagem_responder_requisicao.concat("17");
        mensagem_responder_requisicao.concat("&");
        mensagem_responder_requisicao.concat(id_requisicao);
        mensagem_responder_requisicao.concat("#");

        wifi.println(mensagem_responder_requisicao);
      }
    }
  }
}




void changeMenu()  // Modifica o menu atual
{
  char keypressed = meuteclado.getKey();
  if (keypressed != NO_KEY) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Senha: ");

    senha_global.concat(keypressed);

    lcd.setCursor(0, 1);
    lcd.print(senha_global);


    menu_principal = false;
    menu_aguarde = false;
    menu_offline = false;
    menu_senha = true;


    tempo_menu_senha = millis();
  }
}

/*
  void avisarOnline() {
  if (millis() - tempo_ultima_msg_online > tempo_intervalo_online) {
    int freeMemAntes = freeMemory();


    Serial.println("Avisando online");
    String texto_live = "*";
    texto_live.concat(numero_serie);
    texto_live.concat("-live#|");

    char mensagemArray[texto_live.length() + 1];

    texto_live.toCharArray(mensagemArray, texto_live.length() + 1);

    ctrl_acesso.sendMessage(mensagemArray);

    int freeMemDepois = freeMemory();
    int memConsumida = freeMemAntes - freeMemDepois;

    Serial.print("Método consumiu: ");
    Serial.print(memConsumida);
    Serial.println(" bytes");

    tempo_ultima_msg_online = millis();
    digitalWrite(pinoLedTx, HIGH);
    delay(500);
    digitalWrite(pinoLedTx, LOW);
    variarTempoIntervalo();
  }
  }
*/

void variarTempoIntervalo() {
  tempo_intervalo_online = (random(31) * 500) + 15000;
}


void verificarEstadoReles() {
  if (rele_remoto1_ativo) {
    if (millis() - tempo_acionamento_rele_remoto1 > tempo_acionado_rele_remoto1) {
      rele_remoto1_ativo = false;
      digitalWrite(pinoRele1, HIGH);
    }
  }
  if (rele_remoto2_ativo) {
    if (millis() - tempo_acionamento_rele_remoto2 > tempo_acionado_rele_remoto2) {
      rele_remoto2_ativo = false;
      digitalWrite(pinoRele2, HIGH);
    }
  }
}

void salvarInformacao(String texto, int posicao_inicial) {

  EEPROM.write(posicao_inicial, texto.length());

  for (int i = posicao_inicial + 1; i < posicao_inicial + 1 + texto.length(); i++) {
    EEPROM.write(i, texto.charAt(i - posicao_inicial - 1));
  }
}

String carregarInformacao(int posicao_inicial) {

  String conteudo = "";

  int tamanho = EEPROM.read(posicao_inicial);  //tamanho da string

  for (int i = posicao_inicial + 1; i < posicao_inicial + 1 + tamanho; i++) {
    conteudo.concat(char(EEPROM.read(i)));
  }

  return conteudo;
}


void limparLinha(int linha) {
  for (int i = 0; i < 16; i++) {
    lcd.setCursor(i, linha);
    lcd.print(" ");
  }
}



void imprimeScroll(String linha0, String linha1) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(linha0);

  lcd.setCursor(scrollCursor, 1);
  lcd.print(linha1.substring(stringStart, stringStop));


  //Quanto menor o valor do delay, mais rapido o scroll
  delay(250);

  scroll_sup(linha1);  //Chama a rotina que executa o scroll

  if (stringStart == linha1.length()) {
    stringStart = 0;
    stringStop = 0;
  }
}



void scroll_sup(String msg) {

  if (stringStart == 0 && scrollCursor > 0) {
    scrollCursor--;
    stringStop++;
  } else if (stringStart == stringStop) {
    stringStart = stringStop = 0;
    scrollCursor = 16;
  } else if (stringStop == msg.length() && scrollCursor == 0) {
    stringStart++;
  } else {
    stringStart++;
    stringStop++;
  }
}




void resetScroll() {
  stringStart, stringStop = 0;
  scrollCursor = 0;
}

/*
  void salvarTextos(String principal, String offline0, String offline1) {

  salvarInformacao(principal, 0);
  salvarInformacao(offline0, 100);
  salvarInformacao(offline1, 200);

  }
*/

void salvarConfiguracoes(int disparar, int acionar_r_1, int acionar_r_2, int t_a_l_r_1, int multiplicador1, int t_a_l_r_2, int multiplicador2, int senha) {

  //disparar ao obter acesso_negado
  EEPROM.write(300, disparar);

  //acionar rele 1 ao liberar
  EEPROM.write(301, acionar_r_1);

  //acionar rele 2 ao liberar
  EEPROM.write(302, acionar_r_2);

  //tempo acionamento do rele 1
  EEPROM.write(303, t_a_l_r_1);

  //multiplifacdor tempo acionamento do rele 1
  EEPROM.write(304, multiplicador1);

  //tempo acionamento do rele 2
  EEPROM.write(305, t_a_l_r_2);

  //multiplifacdor tempo acionamento do rele 2
  EEPROM.write(306, multiplicador2);


  EEPROM.write(307, senha / 256);
  EEPROM.write(308, senha % 256);
}


void salvarConfiguracoes(int disparar, int acionar_r_1, int acionar_r_2, int t_a_l_r_1, int multiplicador1, int t_a_l_r_2, int multiplicador2) {

  //disparar ao obter acesso_negado
  EEPROM.write(300, disparar);

  //acionar rele 1 ao liberar
  EEPROM.write(301, acionar_r_1);

  //acionar rele 2 ao liberar
  EEPROM.write(302, acionar_r_2);

  //tempo acionamento do rele 1
  EEPROM.write(303, t_a_l_r_1);

  //multiplifacdor tempo acionamento do rele 1
  EEPROM.write(304, multiplicador1);

  //tempo acionamento do rele 2
  EEPROM.write(305, t_a_l_r_2);


  //multiplifacdor tempo acionamento do rele 2
  EEPROM.write(306, multiplicador2);
}


void salvarToken(int tokens[]) {


  int posicao_inicial = 400;

  for (int i = 0; i <= 9; i++) {
    EEPROM.write(posicao_inicial, tokens[i] / 256);
    EEPROM.write(posicao_inicial + 1, tokens[i] % 256);
    posicao_inicial += 2;
  }
}



void carregarEeprom() {
  linha1Global = carregarInformacao(0);
  linha1GlobalOffline = carregarInformacao(200);

  disparar_acesso_negado = EEPROM.read(300);
  acionar_rele1_liberado = EEPROM.read(301);
  acionar_rele2_liberado = EEPROM.read(302);

  tempo_acionamento_liberado_rele1 = EEPROM.read(303);
  multiplicador_tempo_rele1 = EEPROM.read(304);
  tempo_acionamento_liberado_rele2 = EEPROM.read(305);
  multiplicador_tempo_rele2 = EEPROM.read(306);

  tempo_acionamento_liberado_rele1 = tempo_acionamento_liberado_rele1 * multiplicador_tempo_rele1;
  tempo_acionamento_liberado_rele2 = tempo_acionamento_liberado_rele2 * multiplicador_tempo_rele2;


  //senhamaster
  int parte1 = EEPROM.read(307);
  int parte2 = EEPROM.read(308);
  int senha = (parte1 * 256) + parte2;
  senha_master = senha;

  lerTokens();
}

void salvarSenhaMestra(int senha) {
  //senhamaster

  EEPROM.write(307, senha / 256);
  EEPROM.write(308, senha % 256);
  senha_master = senha;
}



void lerTokens() {
  int posicao_inicial = 400;

  for (int i = 0; i < 10; i++) {
    int parte1 = EEPROM.read(posicao_inicial);
    int parte2 = EEPROM.read(posicao_inicial + 1);
    int token = (parte1 * 256) + parte2;
    tokens_globais[i] = token;
    posicao_inicial += 2;
  }
}


boolean compareSenha(int senha) {

  for (int i = 0; i < 10; i++) {
    if (tokens_globais[i] == senha) {
      //apagar senha
      //i = 5
      EEPROM.write((i * 2) + 400, 0);
      EEPROM.write((i * 2) + 400 + 1, 0);

      delay(100);
      lerTokens();

      return true;
    }
  }
  return false;
}


boolean compareSenhaMestre(int senha) {
  if (senha == senha_master) {
    return true;
  }
  return false;
}
