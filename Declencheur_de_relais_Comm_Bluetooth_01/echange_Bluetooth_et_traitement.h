// ------------------------------------------------------------------------------- 
//            ÉCHANGE ET TRAITEMENT BLUETOOTH
// ------------------------------------------------------------------------------- 
//
//***** Renvoi des données du ESP32 vers le Client ****
// ****Trame de réponse : [ |DATA|#|#|#|#|#| ]  # est un nombre.
void envoieDataVersClient(){

      Serial.print(flVoltMesure);
      Serial.println(" flVoltMesure " );
      Serial.print(u32DelaiDeReaction);
      Serial.println(" delaiDeReaction " );
      Serial.print(u32TempsRelaisActif);
      Serial.println(" tempsRelaisActif " );
      Serial.print(u32DelaiAvantRetour);
      Serial.println(" delaiAvantRetour " );
      Serial.print(blUnitDlReactMicroScd);
      Serial.println(" Bool unité délai de réaction " );
      Serial.println();
      delay(0);
      int inPauseDeTransmission = 100;  // > 50 Temporisation pour éviter les engorgements 
      SerialBT.print("[ |");                 delay(inPauseDeTransmission);
      SerialBT.print("DATA|");               delay(inPauseDeTransmission);
      SerialBT.print(flVoltMesure);          delay(inPauseDeTransmission);
      SerialBT.print("|");                   delay(inPauseDeTransmission);
      SerialBT.print(u32DelaiDeReaction);    delay(inPauseDeTransmission);
      SerialBT.print("|");                   delay(inPauseDeTransmission);
      SerialBT.print(u32TempsRelaisActif);   delay(inPauseDeTransmission);
      SerialBT.print("|");                   delay(inPauseDeTransmission);
      SerialBT.print(u32DelaiAvantRetour);   delay(inPauseDeTransmission);
      SerialBT.print("|");                   delay(inPauseDeTransmission);
      SerialBT.print(blUnitDlReactMicroScd); delay(inPauseDeTransmission);
      SerialBT.print("| ]");
}


//*****  Réception et traitement des échanges sur BLUETOOTH
void do_Uart_Tick()
{
  char charUart_Data=0;
   
  if(SerialBT.available()) 
  {
    size_t len = SerialBT.available();  // size_t  integer non signé, 
                                        // len =  nombre de caractères à lire
    uint8_t u8SerialBuffer[len + 1];    // uint8_t integer non signé 8 bits,
                                        // équivaut à byte
    u8SerialBuffer[len] = 0x00;
    SerialBT.readBytes(u8SerialBuffer, len);
    memcpy(buffUART + buffUARTIndex, u8SerialBuffer, len);// s'assurer que le
                                                          // port série peut
                                                          // lire l'intégralité
                                                          // de la trame de données
    buffUARTIndex += len;
    preUARTTick = millis();
    if(buffUARTIndex >= MAX_PACKETSIZE - 1) 
    {
      buffUARTIndex = MAX_PACKETSIZE - 2;
      preUARTTick = preUARTTick - 200;
    }
  }
  if(buffUARTIndex > 0 && (millis() - preUARTTick >= 100))
  { //données prêtes
    buffUART[buffUARTIndex] = 0x00;
    if(buffUART[0]=='X')                    // Réserve
    {
    }

    //***********  Demande de déclenchement du relais ****
    else if (buffUART[0]=='C')   // CLIC,D           
    {
      Serial.print("Data brut du buffer de réception ->");
      Serial.println(buffUART);
      sscanf(buffUART,"CLIC,%s",&charUart_Data);
      Serial.print("Data après traitement ->"); 
      Serial.print(charUart_Data);
      Serial.println();
    }
    //***********  Démarrage, connexion Bluetooth réussie, envoie des données initiales ****
    //***********  Envoie des données du ESP32 vers le Client ****
    else if (buffUART[0]=='I')   // INIT,I           
    {
      Serial.print("Data brut du buffer de réception ->");
      Serial.println(buffUART);
      sscanf(buffUART,"INIT,%s",&charUart_Data);
      Serial.print("Data après traitement ->"); 
      Serial.print(charUart_Data);
      Serial.println();
    }
    else if (buffUART[0]=='M')   // Unité : délai de réaction = ms
    {
      charUart_Data='M';
      blUnitDlReactMicroScd = 0;
      Serial.print(blUnitDlReactMicroScd);
      Serial.println(" Milliseconde ");
      
    }
    //***** Réception des données venant de la TABLLETTE allant vers le ESP32  ****
    //      Nouveau DATA
    else if (buffUART[0]=='N') // Venant de l'action du bouton d'envoie
    {
      charUart_Data='N';
      Serial.print("Data brut du buffer de réception ->");
      Serial.println(buffUART);
      sscanf(buffUART,"NDATA,%i,%i,%i",&u32DelaiDeReaction,&u32TempsRelaisActif,&u32DelaiAvantRetour);
      Serial.print("Data après traitement ->");  
      Serial.print(u32DelaiDeReaction);
      Serial.print("/");
      Serial.print(u32TempsRelaisActif);
      Serial.print("/");
      Serial.println(u32DelaiAvantRetour);
      Serial.println();
    }
    else if (buffUART[0]=='U')   // Unité : délai de réaction = us
    {
      charUart_Data='U';
      blUnitDlReactMicroScd = 1;
      Serial.print(blUnitDlReactMicroScd);
      Serial.println(" Microseconde ");
    }
    else  charUart_Data=buffUART[0];
    buffUARTIndex = 0;
  }



  //****** EXÉCUTE LES DEMANDES ****
  //    
  switch (charUart_Data)    
    {    
      // ***** DÉCLENCHEMENT DU RELAIS ********
      case 'D':
      delaiDeReaction();
      declencheRelais();  // inclus le délai de retour
      break;
      
      // ***** Envoie des données du ESP32 vers le Client ********
      case 'I':
      envoieDataVersClient();
      break;
      
      case 'M':
      // Enregistrement des données dans le EEPROM
      memFlashDataUser();   
      //***** Renvoi des données du ESP32 vers le Client ****
      envoieDataVersClient();
      break;

      case 'N':
      // Enregistrement des données dans le EEPROM
      memFlashDataUser();  
      //***** Renvoi des données du ESP32 vers le Client ****
      envoieDataVersClient();
      break;

      case 'U':
      // Enregistrement des données dans le EEPROM
      memFlashDataUser();  
      //***** Renvoi des données du ESP32 vers le Client ****
      envoieDataVersClient();
      break;
      
      
      default:break;
    }
}




