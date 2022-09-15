
////////////////////////////////////////COURANTOMETRE || V 2.5 /////////////////////////////
///////////////////////////////////////////////////////////Développé par Méaban KOECHLIN ET BIBI/////
//FOnctionne avec PEBA WIFI V5
#include <arduino.h>
#include <TimerOne.h>
#include <SPI.h>
#include <SD.h>
#include <GSMSim.h>
//https://github.com/erdemarslan/GSMSim
#include <AVR_RTC.h>

/////////////////////////////////////////////////////////////////////////////////////////////
#define DEBUG                     1                             //Activation du debug sur le port série vers le PC (mettre //devant la ligne si desactivation)

#define COMPTEUR_1                1                             //Activation Capteur1 (mettre //devant la ligne si desactivation)
#define CAPTEUR_TOR               1                             //Activation Capteur Tout ou RIEN (mettre //devant la ligne si desactivation)

#define CTR_1_PAS                 1                             //Nombre de pas du compte tours num 1
#define COEFF_CAPT1               1/1                           //Coefficient de transmission entre le compte tours num 1 et l'objet a mesurer

#define NUMERO_1                  "+33770281556"                //assistance technique ---> "+33770281556" 
#define NB_SEC_PAR_MINUTE         60
#define NB_MIN_PAR_HEURE          60

//////////////////////////////////////////////////////////////////////////////////////////////

#define CTR_1_PIN                 5                             /*Definition des numeros de pin*/
#define SD_LED                    2
#define PULSE_LED                 6                             /*(NE PAS TOUCHER : )          */
#define TOR_PIN                   4
#define RX 7
#define TX 8
#define BAUD 9600


static int tor_state =0;
static int pinCS = 53;
static int C1=0;
static int E1=0;
static long int Ctr1=0;
volatile double tr1=0.00;
volatile double TR1=0.00;
volatile float val=0;
volatile int sec=0;
volatile int minutes = 0;
volatile int jours = 0;
volatile long int heures = 0;
volatile float moy=0;
//String historique ="";
char* number;
//char* historique_24h = "";
char* historique_24h[24];
char* historique_24h_1msg = "";
//char* message_capteur = ""; //
GSMSim gsm;
File myFile;
boolean flag_sms = false;
boolean flag_gestion_histo = false;
int index_historique =0;

void SEC();
  

void setup() 
{
  /*********************************/
  number = NUMERO_1;
  /*********************************/
  Serial.begin(115200);       //initialisation communication série vers PC
  delay (100);
  Serial.println("...");
  delay (10000);              //tempo pour attente allumage module GSM

  Serial.println("Start GSM...");
  gsm.start(); // baud default 9600
  
  Serial.println("Changement en mode texte...");
  if(!gsm.smsTextMode(true)){ // TEXT or PDU mode. TEXT is readable :)
       Serial.println("echec");
  }
  Serial.println("Envoi SMS de test --->");
  Serial.println(gsm.smsSend(number, "test sms courantometre peba")); // if success it returns true (1) else false (0)
  delay(2000);
  
  pinMode(CTR_1_PIN, INPUT);       //CAPT 1
  pinMode(LED_BUILTIN, OUTPUT);         //FILE_STATUS LED
  pinMode(PULSE_LED, OUTPUT);      //INT_PULSE LED
  pinMode(TOR_PIN, INPUT);



  // SD Card Initialization///////////////////////////////////////
  if (SD.begin())
  {
      #ifdef DEBUG
      Serial.println("La carte SD est prete a l'emploi");
      #endif
      digitalWrite(SD_LED, HIGH);
  } else
  {          
    #ifdef DEBUG
    Serial.println("l'initialisation de la carte sd a echouee");
    #endif
      return;
  }
        
  myFile = SD.open("test.txt", FILE_WRITE);
          
  // if the file opened okay, write to it:                                                  //TODO WRITE THE CONF
  if (myFile) {
      #ifdef DEBUG
      Serial.println("Ecriture dans le fichier test.txt...");
      #endif
      
      // Write to file
      /*myFile.println("MESURES / PROTO TURBINE");
      myFile.println("MESURES A IMPORTER DANS EXCEL AVEC ; COMME SEPARATION");*/
      myFile.close(); // close the file   
      digitalWrite(SD_LED, LOW);
  }
  // if the file didn't open, print an error:
  else {
      #ifdef DEBUG
      Serial.println("Erreur d'ouverture du fichier test.txt");
      #endif
      digitalWrite(SD_LED, LOW);
  }

  Timer1.initialize(1000000); // Periode = 1 minute
  Timer1.attachInterrupt(SEC); //Fonction comptages des secondes pour cadencement envoi resultat
}
 
////////////////////////////////MAIN///////////////////////////////



void loop() 
{
 ////////////CAPTEUR1/////////////////////////// 
 #ifdef COMPTEUR_1
   C1 = digitalRead(5); 
  
  if ((C1 == 0)&&(E1 == 1))
       {
          E1 = 0;
       }
  
  if ((C1 == 1)&&(E1 == 0))
        {
            Ctr1 = Ctr1+1;
            E1 = 1;
        }
  if (Ctr1>= CTR_1_PAS)     
      {
          Ctr1 = 0;
          tr1 = tr1 + 1;
          TR1 = tr1*1;
     }
 #endif


    if(flag_gestion_histo){
         Serial.println("debug flag histo...");
         flag_gestion_histo = false;
         //Gestion historique 24h
         char message_1h[7];
         index_historique = heures;
         Serial.println("debug index histo...");
         Serial.println(index_historique);
              /*dtostrf(TR1, 3, 2, message_24h);  
              //historique_24h[index_historique-1] = message_24h;
              strcpy(historique_24h[index_historique-1], message_24h);*/

            /*  #ifdef CAPTEUR_TOR
          char message_capteur[9];
          char etat_tor[2];
          String str;
          #else
          char message_capteur[7];
          #endif
          dtostrf(TR1, 3, 2, message_capteur);             //insertion de la valeur capteur dans la chaine de caractère "message_capteur"
          #ifdef CAPTEUR_TOR
          str = String(tor_state);
          str.toCharArray(etat_tor,2);
          strcat(message_capteur,";");
          strcat(message_capteur, etat_tor);
          strcat(message_capteur,";");
          #endif
          Serial.println(message_capteur);
          //Serial.println(tor_state);*/

        TR1 = 0;

        if(flag_sms){
          Serial.println("debug flag sms...");
          //flag_sms = false;
          heures = 0;
          char message_capteur[9];
          Serial.println("Envoi sms...");
          ////////
          //gsm.smsSend(number,message_capteur);
          ////////
          //TODOOO
          if(!gsm.smsSend(number, message_capteur)){
            Serial.println("Echec envoi sms...");
            flag_sms = false;
          }else{
            flag_sms = false;
          }
          //delay(2000);
          //Serial.println(gsm.smsSend(number, message_capteur)); // envoi par sms de message_capteur
          //todo essai
          //Serial.println(gsm.smsSend(number, print(TR1))); // envoi par sms de message_capteur
          //RAZ des variables...
          
      }
    }

}   

///////////////////////////////////FIN MAIN/////////////////////////////




//////////////////////////////////FONCTION HORLOGE ecriture carte sd///////////////////////////
void SEC()
{
  //TODO if flag == false //pour attendre la fin du sms avant de recompter
  if(flag_sms == false){
    #ifdef CAPTEUR_TOR
		tor_state = digitalRead(TOR_PIN);
    #endif
		sec = sec +1;
		
		if(sec == NB_SEC_PAR_MINUTE)
		{
		  minutes = minutes + 1;
		  sec = 0;
	  
		   if(minutes == NB_MIN_PAR_HEURE)
			{
				minutes = 0;
				heures = heures +1;  

				////////ECRITURE CARTE SD TT LES HEURES///////////////////////
				moy = moy/3600;  //calcul final de la moyenne (division par 60sec)
        Serial.println("");
        Serial.println("");
				Serial.println(heures);
				Serial.println("Mesure...");
				Serial.print(TR1);
        Serial.println(";");
				Serial.println("TOUR");
        Serial.println(";");
        Serial.print(tor_state);
        Serial.println(";");
        Serial.println("TOR");
        

        //digitalWrite(LED_BUILTIN, HIGH);                     //todo test si fichier error et allumer led
				myFile = SD.open("test.txt", FILE_WRITE);
				myFile.println();
				myFile.print(heures);
				myFile.print("H");
				myFile.print(minutes);
				myFile.print("min");
				myFile.print(sec);
				myFile.print("s");
				myFile.print(";");
				#ifdef COMPTEUR_1
				myFile.print("compteur1;");
				myFile.print(TR1);                            // changer tr1 pour TR1 pour appliquer le coefficient de reduction pour obtenir le nb tour de roue
				myFile.print(";");
				#endif
        #ifdef CAPTEUR_TOR
				myFile.print("TOR;");
				myFile.print(tor_state);                            // afficher etat logique de la lecture TOR
				myFile.print(";");
        #endif
				myFile.close(); // close the file
        //digitalWrite(LED_BUILTIN, LOW);
				
				flag_gestion_histo = true;
				//RAZ des compteurs pour la prochaine heure.     
        //TR1 = 0;                                  
				tr1 = 0;
				moy = 0;
	  
				if(heures == 24)
				{
					index_historique = 0;
					jours = jours +1;     
          flag_sms = true;       
				}
				
			}
	  
		}
	}else
	{
		Serial.println("attente fin envoi sms avant mesure...");
	}
}
