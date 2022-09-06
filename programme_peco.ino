// framboise314 - Motorisation plaque tournante PECO pour train HO - Août2022
// Programme en CC-BY-NC-SA
// Utilisation gratuite pour application non commerciale
// Modification, diffusion, copie... libres
// Programme Arduino Nano pour enregistrer les positions des voies HO dans l'EEPROM
// Au démarrage, le moteur est positionné à 0 grâce à la fourchette optique
// Si le switch est positionné en mode programmation, l'écran affiche PROG D/G
// L'inverseur D/G permet de déplacer doucement le moteur
// Le bouton poussoir 1/2T accélère le déplacement du moteur pour faciliter l'approche
// La position actuelle est affichée sur l'écran (en nombre de micro pas)
// L'enregistrement se fait en appuyant au moins 0,5 seconde sur un des boutons de voies
// Chaque valeur est codée sur 16 bits - 2 octets
//    VOIE1    adresse mémoire 0
//    VOIE2    adresse mémoire 2
//    VOIE3    adresse mémoire 4
//    VOIE4    adresse mémoire 6

// Gestion de l'écran avec la bibliothèque Adafruit
// ===================================================================================
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Bibliothèque pour gérer l'EEPROM
#include <EEPROM.h>
// Adresse de la data dans l'EEPROM
int adresse = 0;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     4
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// ===================================================================================

// Définition des Entrées/Sorties
const int INT_OPTO     = 2;    // Fourchette optique => interruption
const int PAP_DIR      = 3;    // Direction vers driver A4988
const int PAP_STEP     = 4;    // Pas vers driver A4988
const int MODE_PROGRAM = 5;    // DIP Switch 4 => Si on : mode programmation
const int LED_LOCK     = 6;    // LED LOCK : indique que le plateau est verrouillé
// Clignote quand l'interruption est déclenchée lors de la RAZ du moteur
const int LED_1_2_T    = 7;    // LED du bouton poussoir 1/2T s'allume quand le bouton est appuyé et pendant le 1/2T du plateau
const int LED_VOIE1    = 8;    // LED du bouton poussoir VOIE1 s'allume quand le bouton est appuyé et jusqu'à l'arrivée sur VOIE1
const int LED_VOIE2    = 9;    // LED du bouton poussoir VOIE2 s'allume quand le bouton est appuyé et jusqu'à l'arrivée sur VOIE2
const int LED_VOIE3    = 10;   // LED du bouton poussoir VOIE3 s'allume quand le bouton est appuyé et jusqu'à l'arrivée sur VOIE3
const int LED_VOIE4    = 11;   // LED du bouton poussoir VOIE4 s'allume quand le bouton est appuyé et jusqu'à l'arrivée sur VOIE4
const int BP_GAUCHE    = 12;   // Entrée de l'inverseur D/G => tourner vers la gauche - CCW - sens anti-horaire
const int BP_DROIT     = 15;   // D15 Entrée de l'inverseur D/G => tourner vers la droite - CW - sens horaire
const int BUZZER       = 14;   // Buzzer actif - Un 1 déclenche le bruit
#define   BP_VOIE      = A7;   // Entrée analogique des boutons poussoirs (BP). Les BP envoient une tension convertie en N° de poussoir

// Variable pour ranger la valeur du bouton appuyé
int BP;

// Connexions du driver de moteur Pas à pas et nombre de pas par tour
const int dirPin       = 3;    // Commande de la direction de rotation G/D  CW/CCW  horaire/anti-horaire
const int stepPin      = 4;    // Envoyer un signal sur cette patte pour faire un pas 0=>1>0
const int pasParTour   = 1600; // nombre de (micro)pas par tour - Le A4988 est programmé en 1/8 de pas => 200x8 = 1600 pas/tour

// Variables globales pour récupérer l'état de l'inverseur
int gauche = HIGH;
int droite = HIGH;
// Variable globale pour stoker la valeur lue sur A7
int val = 0;

// Déclaration des messages
String message          = "                     ";
String msg_normal1      = "Appuyer    BP";
String msg_test         = "  TEST";
String msg_raz          = " R.A.Z.";
String msg_raz1         = "Remise     a zero";
String msg_program      = "   MODE     PROGRAM";
String msg_amener       = "  Amener   sur voie";
String msg_DG           = " Bouton      D/G";
String msg_valid        = "Validation  BP voie";
String msg_pos          = "Position   ";
String msg_record       = "Enregistre ";
String msg_demitour     = " Demi       tour";
String msg_erreur       = "ERREUR";
String msg_dej_pos      = "Deja en   position";
String msg_rotation_cw  = "Rotation  CW";
String msg_rotation_ccw = "Rotation  CCW";
String msg_aller       = "ALLER";

char msg_voie[][7] = {
  "VOIE 1",
  "VOIE 2",
  "VOIE 3",
  "VOIE 4"
};

String msg_relit        = "Relecture ";

// Emplacement pour ranger les positions enregistrées en EEPROM
int position_voie[4];


// Position actuelle du moteur, Depart = point de depart, destination = destination
int position;
int depart;
int destination;

// Nombre de pas dont il faut tourner
int nbre_pas;

// Sens de rotation du moteur PAP
String sens_rotation = "CCW";

// LOGO accueil
// =========================================================
const unsigned char logo_train [] PROGMEM = {
  // 'image_logo, 128x32px
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xf0, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xe0, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x1b, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xc0, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x1b, 0x7f, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xc0, 0x00, 0x3e, 0x07, 0xff, 0xff, 0xff, 0xfe, 0xdf, 0x7f, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xc7, 0xfc, 0x7c, 0x03, 0xff, 0xff, 0xff, 0xfe, 0xdb, 0x62, 0x39, 0xc3, 0x1f, 0xff, 0xff, 0xff,
  0xc7, 0xfc, 0x7e, 0x07, 0xff, 0xff, 0xff, 0xfe, 0x1b, 0x69, 0x34, 0xd3, 0x4f, 0xff, 0xff, 0xff,
  0xc7, 0xfc, 0x7e, 0x0f, 0xff, 0xff, 0xff, 0xfe, 0x1b, 0x69, 0x7e, 0xd3, 0x4f, 0xff, 0xff, 0xff,
  0xc7, 0xfc, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfb, 0x69, 0x70, 0xd3, 0x0f, 0xff, 0xff, 0xff,
  0xc7, 0xfc, 0x7e, 0x07, 0xff, 0xff, 0xff, 0xfe, 0xfb, 0x69, 0x76, 0xd3, 0x7f, 0xff, 0xff, 0xff,
  0xc7, 0xfc, 0x7e, 0x07, 0xff, 0xff, 0xff, 0xfe, 0xfb, 0x29, 0x34, 0xd3, 0x4f, 0xff, 0xff, 0xff,
  0xc7, 0xfc, 0x7e, 0x07, 0xff, 0xff, 0xff, 0xfe, 0xfb, 0x23, 0x30, 0xc3, 0x1f, 0xff, 0xff, 0xff,
  0xc0, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf3, 0xff, 0xff, 0xff, 0xff,
  0xc0, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd3, 0xff, 0xff, 0xff, 0xff,
  0xc0, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe7, 0xff, 0xff, 0xff, 0xff,
  0xc0, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xc0, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xc0, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xc0, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xc0, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xfe, 0x18, 0x67, 0x8f, 0xff, 0xff, 0xff, 0xff,
  0xc0, 0x3e, 0x01, 0xf0, 0x1f, 0xff, 0xff, 0xff, 0xfe, 0x18, 0x43, 0x0f, 0xff, 0xff, 0xff, 0xff,
  0xc0, 0x7f, 0x03, 0xf8, 0x1f, 0xff, 0xff, 0xff, 0xfe, 0xdb, 0xdb, 0x67, 0xff, 0xff, 0xff, 0xff,
  0xc0, 0xc0, 0x00, 0x18, 0x1f, 0xff, 0xff, 0xff, 0xfe, 0xdb, 0xdf, 0x67, 0xff, 0xff, 0xff, 0xff,
  0xe0, 0xc0, 0x00, 0x0c, 0x0f, 0xff, 0xff, 0xff, 0xfe, 0x18, 0xdf, 0x67, 0xff, 0xff, 0xff, 0xff,
  0xe0, 0xc0, 0x00, 0x0c, 0x3f, 0xff, 0xff, 0xff, 0xfe, 0x18, 0x5f, 0x67, 0xff, 0xff, 0xff, 0xff,
  0xf8, 0xc1, 0xfe, 0x2c, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfb, 0xdf, 0x67, 0xff, 0xff, 0xff, 0xff,
  0xf8, 0xe3, 0x07, 0x19, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfb, 0xdb, 0x67, 0xff, 0xff, 0xff, 0xff,
  0xfc, 0x7f, 0x03, 0xf9, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xf8, 0x43, 0x0f, 0xff, 0xff, 0xff, 0xff,
  0xfc, 0x1c, 0x70, 0xe3, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xf8, 0x67, 0x8f, 0xff, 0xff, 0xff, 0xff,
  0xfe, 0x00, 0xf0, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0x81, 0xfc, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};


// Définit les E/S utilisées par le driver A4988 du moteur pas à pas NEMA17
// Le driver est réglé sur le 1/8 de pas, ce qui fait 1600 pas pour 1 tour
#define dirPin 3
#define stepPin 4
#define stepsPerRevolution 1600

// Paramètres de vitesse du moteur en microsecondes
// Plus le nombre est grand plus la rotation est lente
int tres_vite  = 200;
int vite       = 500;
int lent       = 1000;
int tres_lent  = 10000;
int vitesse    = vite;

// Déclaration du flag en volatile pour utilisation dans la macro d'interruption
volatile boolean flag = false;

// Variable dans laquelle on range la valeur de la tension lue sur A7
int valeur = 1005;


// SETUP - Configuration exécutée une seule fois au démarrage
// ==========================================================
void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);


  // Déclaration des LED et extinction au démarrage
  pinMode(INT_OPTO, INPUT_PULLUP);     // Paramétrage de D2 en entrée en pullup
  attachInterrupt(digitalPinToInterrupt(2), prog_int, RISING); //Paramétrage d'une interruption sur la pin 2 / Front montant
  pinMode(PAP_DIR, OUTPUT);
  digitalWrite(PAP_DIR, LOW);          // Paramétrage de D3 en sortie et mise à 0
  pinMode(PAP_STEP, OUTPUT);
  digitalWrite(PAP_STEP, LOW);         // Paramétrage de D4 en sortie et mise à 0
  pinMode(MODE_PROGRAM, INPUT);        // Paramétrage de D5 en entrée en pullup switch PROGRAM
  pinMode(LED_LOCK, OUTPUT);
  analogWrite(LED_LOCK, 0);            // Paramétrage de D6 en sortie et mise à 0 - Extinction de LED LOCK
  pinMode(LED_1_2_T, OUTPUT);
  digitalWrite(LED_1_2_T, LOW);        // Paramétrage de D7 en sortie et mise à 0 - Extinction de LED 1/2T
  pinMode(LED_VOIE1, OUTPUT);
  digitalWrite(LED_VOIE1, LOW);        // Paramétrage de D8 en sortie et mise à 0 - Extinction de LED VOIE1
  pinMode(LED_VOIE2, OUTPUT);
  digitalWrite(LED_VOIE2, LOW);        // Paramétrage de D9 en sortie et mise à 0 - Extinction de LED VOIE2
  pinMode(LED_VOIE3, OUTPUT);
  digitalWrite(LED_VOIE3, LOW);        // Paramétrage de D10 en sortie et mise à 0 - Extinction de LED VOIE3
  pinMode(LED_VOIE4, OUTPUT);
  digitalWrite(LED_VOIE4, LOW);        // Paramétrage de D11 en sortie et mise à 0 - Extinction de LED VOIE4
  pinMode(BP_GAUCHE, INPUT_PULLUP);
  pinMode(BP_DROIT, INPUT_PULLUP);     // Déclaration de l'inverseur G/D
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, HIGH);          // Paramétrage de D14/A0 en sortie Arrêt buzzer

  logo();                              // Affiche le logo
  ledsOn(200);                         // Allumer toutes les LED un instant pour les vérifier
  bip(60);                             // Emettre un bip court
  clearScreen();                       // Effacer l'écran

}




// Boucle principale du programme
// ==============================
void loop() {
  // Mise à zéro du moteur pas à pas
  zero_pap();
  // Mettre le compteur de pas "position" à zéro
  position = 0;

  // Lit le switch MODE_PROGRAM / Passe en mode PROGRAMME si le switch est sur ON
  // Lu une seule fois au démarrage il faut redémarrer l'Arduino pour passer en utilisation normale
  if ( digitalRead(MODE_PROGRAM) == 1) {
    msg_txt(msg_program);
    delay(2000);
    msg_txt(msg_amener);
    delay(1500);
    msg_txt(msg_DG);
    delay(2000);
    msg_txt(msg_valid);
    delay(2000);


    // Boucle infinie de programmation
    //================================
    // Afficher position
    msg_txt_val(msg_pos, position);

    while (true) {

      // Est ce que l'inverseur G/D est actionné ?
      // Lire les entrées G et D
      gauche = digitalRead(BP_GAUCHE);
      droite = digitalRead(BP_DROIT);
      // On positionne grossièrement le moteur avec l'inverseur D/G
      // Quand la position est proche, appuyer sur le BP 1/2T pour avoir une vitesse réduite
      while (digitalRead(BP_GAUCHE) == LOW || digitalRead(BP_DROIT) == LOW) {
        // Ne pas aller à gauche si on est proche de la fourchette, décrémenter la position
        if (digitalRead(BP_GAUCHE) == LOW && position > 20) {
          digitalWrite(LED_1_2_T, HIGH);
          // Si le BP 1/2T est appuyé, ralentir la rotation
          if (bp() == 1) {
            delay(200);
          }
          unPas(vite, "CCW");
        }
        // Aller à droite jusque près de la fourchette / incrémenter la position
        if (digitalRead(BP_DROIT) == LOW && position < 1580) {
          digitalWrite(LED_VOIE1, HIGH);
          if (bp() == 1) {
            delay(200);
          }
          unPas(vite, "CW");
        }
      }
      digitalWrite(LED_1_2_T, LOW);
      digitalWrite(LED_VOIE1, LOW);
      affiche_position();


      // Si on appuie sur un des BP VOIE1 à VOIE4 on mémorise la position correspondante
      // La fonction BP retourne 9 si aucun bouton appuyé, sinon elle retourne le N° du bouton
      if (bp() != 9) {
        switch (bp()) {
          case 2:
            digitalWrite(LED_VOIE1, HIGH);
            mem_voie(1);
            digitalWrite(LED_VOIE1, LOW);
            break;
          case 3:
            digitalWrite(LED_VOIE2, HIGH);
            mem_voie(2);
            digitalWrite(LED_VOIE2, LOW);
            break;
          case 4:
            digitalWrite(LED_VOIE3, HIGH);
            mem_voie(3);
            digitalWrite(LED_VOIE3, LOW);
            break;
          case 5:
            digitalWrite(LED_VOIE4, HIGH);
            mem_voie(4);
            digitalWrite(LED_VOIE4, LOW);
            break;
        }
      } // Fin de la boucle while
    }
  }   // Fin du if mode program

  // Si le switch MODE_PROGRAM est sur OFF : Passe en mode NORMAL
  // ============================================================
  else {
    // Relire contenu EEPROM et le ramener en mémoire
    relire_EEPROM();
    aller_voie1();

    // Boucle infinie fonctionnement normal
    // ====================================
    while (true) {

      // Affichage texte + valeur
      msg_txt_val(msg_pos, position);
      // affiche_position();
      delay(1000);
      msg_txt(msg_normal1);
      delay(1000);
      // Si on appuie sur le BP 1/2 toutr, le plateau fait un demi tour
      // Si on appuie sur un des BP VOIE1 à VOIE4 on déplace le moteur sur la position mémorisée
      // La fonction BP retourne 9 si aucun bouton appuyé, sinon elle retourne le N° du bouton
      BP = bp();
      if (BP != 9) {
        switch (BP) {
          // BP 1/2T  Faire un demi tour
          // ===========================
          case 1:
            allume_LED_BP(BP);
            msg_txt(msg_demitour);
            delay(500);
            // Le départ est la position actuelle
            depart = position;
            // La destination est 800 pas plus loin
            destination = position + 800;
            // Si Dest  < 1600 on tourne dans le sens des aiguilles d'une montre CW
            if (destination < 1600) {
              sens_rotation = "CW";
              msg_txt(msg_rotation_cw);
              nbre_pas = 800;
            }
            // Si Dest  >= 1600 on tourne dans le sens des aiguilles d'une montre CW
            if (destination >= 1600) {
              sens_rotation = "CCW";
              msg_txt(msg_rotation_ccw);
              nbre_pas = 800;
            }
            // Si Dest  = 1600 ou Zero on diminue le nombre de pas pour ne pas provoquer d'erreur
            if (destination == 1600 || destination == 0) {
              nbre_pas = 790;
            }

            for (int i = 0; i < nbre_pas ; i++) {
              unPas(lent, sens_rotation);
              // Si on coupe le faisceau : erreur
              if   (flag == true) {
                // Le remettre à 0
                flag = false;
                // Allumer la LED LOCK
                analogWrite(LED_LOCK, 30);
                // ARRETER LE PROGRAMME
                // ====================
                msg_gros_txt(msg_erreur);
                while (1) {}
              }
            }
            eteint_LED_BP(BP);
            msg_txt_val(msg_pos, position);
            delay(750);
            break;

          // Pour tous les autres BP traitement identique
          // seul le numéro de voie change
          // BP 2345
          // ============================================
          case 2:
          case 3:
          case 4:
          case 5:
            // Allumer la LED du BP
            allume_LED_BP(BP);
            // Si c'est un des BP de positionnement on récupère la destination et le départ
            // Le départ est la position actuelle
            depart = position;
            // La destination est la valeur indiquée par le BP qui a été appuyé
            destination = position_voie[BP - 2];
            // Si Dest - Dep > 0 on tourne dans le sens des aiguilles d'une montre CW
            if (destination - depart > 0) {
              sens_rotation = "CW";
              msg_txt(msg_rotation_cw);
              delay(500);
              nbre_pas = destination - depart;
            }
            // Si Dest - Dep < 0 on tourne dans le sens inverse des aiguilles d'une montre CCW
            // D'une valeur égale à Dep - Dest
            if (destination - depart < 0) {
              sens_rotation = "CCW";
              msg_txt(msg_rotation_ccw);
              delay(500);
              nbre_pas = depart - destination;
            }
            // Si on appuie sur le bouton ou on est déjà
            if (destination == position) {
              nbre_pas = 0;
              msg_txt(msg_dej_pos);
              delay(500);
            }
            for (int i = 0; i < nbre_pas ; i++) {
              unPas(lent, sens_rotation);
              // Si on coupe le faisceau : erreur
              if   (flag == true) {
                // Le remettre à 0
                flag = false;
                // Allumer la LED LOCK
                analogWrite(LED_LOCK, 30);
                // ARRETER LE PROGRAMME
                // ====================
                msg_gros_txt(msg_erreur);
                while (1) {}
              }
            }
            eteint_LED_BP(BP);
            msg_txt_val(msg_pos, position);
            delay(750);
            BP = 9;
            break;

            // Fin du switch
        }
        // Fin du if BP
      }
      // Fin du while
      // =======================
    }
    // Fin du else
    // ====================
  }
  // Fin du LOOP
}

// =========================================================
// ==                                                     ==
// ==      FONCTIONS UTILISEES DANS LE PROGRAMME          ==
// ==                                                     ==
// =========================================================


// Traitement de l'interruption opto - Positionne le flag à 1
// ==========================================================
// Si l'interruption a été traitée, le flag a été remis à zéro
// Le passage par ce programme d'interruption le met à 1
void prog_int(void) {
  flag = !flag; //Inversion du flag
}

// Emettre un bip - Temps du bip passé en paramètre / Millisecondes
// ================================================================
void bip(int temps) {
  digitalWrite(BUZZER, 0);    // Mettre le Buzzer en route
  delay(temps);                  // Durée de fonctionnement
  digitalWrite(BUZZER, 1);    // Arrêter le Buzzer
}

// On allume toutes les LED pendant un temps en millisecondes
// ==========================================================
void ledsOn(int temps) {
  analogWrite(LED_LOCK, 30);           // Allume la LED LOCK en PWM à 30%
  digitalWrite(LED_1_2_T, HIGH);
  digitalWrite(LED_VOIE1, HIGH);
  digitalWrite(LED_VOIE2, HIGH);
  digitalWrite(LED_VOIE3, HIGH);
  digitalWrite(LED_VOIE4, HIGH);
  delay(temps);
  analogWrite(LED_LOCK, 0);
  digitalWrite(LED_1_2_T, LOW);
  digitalWrite(LED_VOIE1, LOW);
  digitalWrite(LED_VOIE2, LOW);
  digitalWrite(LED_VOIE3, LOW);
  digitalWrite(LED_VOIE4, LOW);
}

// Flash LED LOCK
// ==============
void flash_LED_LOCK(void) {
  analogWrite(LED_LOCK, 30);           // Allume la LED LOCK en PWM à 30%
  delay(100);
  analogWrite(LED_LOCK, 0);
}

// Affichage texte passé en paramètre
// ========================
void  msg_txt(String chaine) {

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(chaine);
  display.display();
}

// Affichage texte + valeur
// ========================
// Affiche un texte suivi d'un entier
void  msg_txt_val(String chaine, int valeur) {
  String espace = "   ";
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(chaine);
  display.setCursor(0, 15);
  display.println(valeur);

  display.display();
  delay(500);
}


// Effacer l'écran
// ===============
void clearScreen(void) {
  display.clearDisplay();
  display.display();
  delay(200);
}

// Affichage GROS texte
// ====================
// Affiche un texte en grand
void  msg_gros_txt(String chaine) {

  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println(chaine);
  display.display();
  delay(200);
}

// Afficher le logo d'accueil
// ==========================
void logo(void) {
  display.clearDisplay();
  display.drawBitmap(0, 0, logo_train, 128, 32, WHITE);
  display.display();
  delay(3000);
}


// Mise à zéro du moteur pas à pas
// ===============================
void zero_pap(void) {
  // Indique qu'on met à zero
  msg_txt(msg_raz1);
  // Ramener le moteur rapidement sur la fourchette opto
  // ===================================================
  // Rotation anti-horaire :
  digitalWrite(dirPin, LOW);

  // Faire tourner le moteur rapidement jusqu'à la fourchette opto
  while (!flag) {
    // These four lines result in 1 step:
    unPas(vite, "CCW");
  }
  // Faire clignoter LED LOCK et Remettre le flag à false
  analogWrite(LED_LOCK, 50);
  delay(100);
  analogWrite(LED_LOCK, 00);
  flag = false;

  // On s'arrête 0,5 seconde
  delay(500);
  // On écarte un peu le moteur à vitesse rapide
  // ===========================================
  // Rotation horaire :
  digitalWrite(dirPin, HIGH);
  // Fait tourner le moteur lentement
  // Destination
  for (int i = 0; i < 50 ; i++) {
    // These four lines result in 1 step:
    unPas(lent, "CW");
  }
  delay(500);

  // On ramène doucement le moteur sur l'opto
  // ========================================
  // Rotation anti-horaire :
  // digitalWrite(dirPin, LOW);

  // Faire tourner le moteur rapidement jusqu'à la fourchette opto
  while (!flag) {
    // These four lines result in 1 step:
    unPas(tres_lent, "CCW");
  }
  // Faire clignoter LED LOCK et Remettre le flag à false
  analogWrite(LED_LOCK, 50);
  delay(100);
  analogWrite(LED_LOCK, 00);
  flag = false;

  // Ecrire le message RAZ  sur l'écran
  msg_gros_txt(msg_raz);
  delay(500);
  position = 0;
  clearScreen();
}

// Fonction pour faire avancer le moteur pas à pas d'un pas
// ========================================================
int unPas(int v, String direction) {
  if (direction == "CW") {
    digitalWrite(dirPin, HIGH);
    position += 1;
  }
  if (direction == "CCW") {
    digitalWrite(dirPin, LOW);
    position -= 1;
  }
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(v);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(v);

}

// Afficher la position du moteur
// ==============================
void affiche_position(void) {
  // Afficher position
  msg_txt_val(msg_pos, position);
}

// Retourne le bouton poussoir appuyé
// ==================================
// Si la valeur lue est > 900 aucun bouton appuyé
// Sinon on divise et arrondit la valeur mesurée sur A7 pour obtenir le N° du BP
// 1 = BP 1/2T
// 2 = BP VOIE1
// 3 = BP VOIE2
// 4 = BP VOIE3
// 5 = BP VOIE 4
// 9 = Pas de BP appuyé
int bp(void) {
  valeur = analogRead(A7); // on lit les données du pin A7
  if (valeur < 900) {
    return (round(valeur / 170));
  }
  else {
    return 9;
  }
}

// Enregistrer la position d'une voie en mémoire
// =============================================
// Enregistre en mémoire, affciche l'enrgistrement puis relit la valeur depuis EEPROM
void mem_voie(int voie) {
  int valeur_relue;
  adresse = voie * 2;
  EEPROM.put(adresse, position);
  // Afficher sur l'écran
  message = msg_record + msg_voie[voie - 1];
  msg_txt(message);
  delay(1000);
  msg_txt_val(msg_record, position);
  delay(1000);
  EEPROM.get(adresse, valeur_relue);
  msg_txt_val(msg_relit, valeur_relue);
  flash_LED_LOCK();
  bip(50);
}

// Relecture de l'EEPROM en mode normal et affichage des 4 valeurs
// ===============================================================
void relire_EEPROM(void) {
  int valeur_relue;
  for (int i = 0; i < 4; i++) {
    // Lire valeur de l'EEPROM
    EEPROM.get((i + 1) * 2, valeur_relue);
    //msg_txt_val(msg_voie[i], valeur_relue);
    // Copier valeur relue dans la mémoire programme
    position_voie[i] = valeur_relue;
    msg_txt_val(msg_voie[i], position_voie[i]);
    delay(1000);
  }
}

// Allume LED BP
//==============
// Allume la LED correspondant à un BP appuyé
void allume_LED_BP(int bp) {
  if (bp == 1) {
    digitalWrite(LED_1_2_T, HIGH);
  }
  if (bp == 2) {
    digitalWrite(LED_VOIE1, HIGH);
  }  if (bp == 3) {
    digitalWrite(LED_VOIE2, HIGH);
  }  if (bp == 4) {
    digitalWrite(LED_VOIE3, HIGH);
  }  if (bp == 5) {
    digitalWrite(LED_VOIE4, HIGH);
  }
}

// Eteindre LED BP
// ===============
// Eteint a LED correspondant à un BP relaché
void eteint_LED_BP(int bp) {
  if (bp == 1) {
    digitalWrite(LED_1_2_T, LOW);
  }
  if (bp == 2) {
    digitalWrite(LED_VOIE1, LOW);
  }  if (bp == 3) {
    digitalWrite(LED_VOIE2, LOW);
  }  if (bp == 4) {
    digitalWrite(LED_VOIE3, LOW);
  }  if (bp == 5) {
    digitalWrite(LED_VOIE4, LOW);
  }
}

// Envoyer le plateau tournant sur la voie 1
// =========================================
void aller_voie1(void) {
  // On récupère la position de la VOIE1 et on envoie le plateau
  depart = position;
  destination = position_voie[0];
  Serial.print("Destination debut : ");
  Serial.println(destination);
  Serial.print("Depart : ");
  Serial.println(depart);
  if (destination - depart > 0) {
    sens_rotation = "CW";
    msg_txt(msg_aller);
    delay(500);
    msg_txt(msg_voie[0]);
    delay(500);
    nbre_pas = destination - depart;
  }
  // Si ce n'est pas le cas arrêter le programme
  // car il y a une erreur
  else {
    // Allumer la LED LOCK
    analogWrite(LED_LOCK, 30);
    // ARRETER LE PROGRAMME
    // ====================
    msg_gros_txt(msg_erreur);
    while (1) {}
  }
  // Aller sur la voie 1
  for (int i = 0; i < nbre_pas ; i++) {
    unPas(lent, sens_rotation);
    // Si on coupe le faisceau : erreur
    if   (flag == true) {
      // Le remettre à 0
      flag = false;
      // Allumer la LED LOCK
      analogWrite(LED_LOCK, 30);
      // ARRETER LE PROGRAMME
      // ====================
      msg_gros_txt(msg_erreur);
      while (1) {}
    }
  }


}
