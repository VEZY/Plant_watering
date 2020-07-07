//Including the Telegram library for communication: 
#include "CTBot.h"

// ------- Telegram config --------
String ssid  = "xxxxx"      ; // Wifi name
String pass  = "xxxxx"; // Wifi Password
String token = "xxxxxx:xxxxxxxxxxxxxxxxxxxxxxx"   ; // Telegram bot token
int chatID = -000000000; // This the ID of our group chat for the plants

// Instantiate a bot object:
CTBot myBot;
CTBotInlineKeyboard myKbd;  // custom inline keyboard object helper

#define IS_SYSTEM_ON_CALLBACK  "isSystemON"   // callback data sent when "Is The system ON?" button is pressed
#define SOIL_MOISTURE_CALLBACK "soilMoisture" // callback data sent when "Show soil moisture" button is pressed
#define SYSTEM_OFF_CALLBACK "systemOFF"       // callback data sent when "Shut down the system" button is pressed
#define SYSTEM_ON_CALLBACK "systemON"         // callback data sent when "Turn on the system" button is pressed
#define PUMP_ON_CALLBACK "pumpON"             // callback data sent when "Turn on the water pump" button is pressed

#define Bot_mtbs 1000 //mean time between scan messages
#define Bot_mtbs_2 86400000 //mean time between messages for "no plant wattering since X days"
long Bot_lasttime;   //last time messages' scan has been done

#define interval 500 // Interval between readings

// Define pins: 
#define relay 13
#define soil_moisture  A0
#define tank_floater  16

// State of the whole system:
bool system_on = true;

// State of the pump in manual mode: 
bool pump_manual_state = false;
int manual_watering_duration = 30000; // Manual watering duration

// Floater in the water tank: 
int water_level = 2;
bool is_level_change = false; // Keep trace of a change in tank level
int prev_water_level = 2;     // To keep trace of the water level from the previous loop execution

// Soil Moisture sensor: 
int moisture = 0;             // Initializing moisture to 0
int moisture_perc = 0;        // Moisture in %
const int AirValue = 750;    // Driest value -> air value
const int WaterValue = 280;  // Wettest value -> water value

// Timer to avoid wattering for too long: 
int watering_max_time = 600000;       // Maximal time for one wattering event (10 minutes)
long watering_interval_min = 7200000L; // Minimal time to wait before two waterring events (2 hours here)
long last_watering_start = 0.0 ;      // time since the waterring event as been started
long last_watering_end = -watering_interval_min; // time since the last waterring event as been ended
// NB: initialized at -watering_interval_min to be able to start wattering at startup.
long max_wo_water = 345600000L; // Maximum days without wattering until notification to say there's probably an issue with the system


#define min_soil_moisture 40         // Min soil moisture (trigger when below):
bool unexpected_watering_end = false;// Flag unexpected end of a watering event


void setup() {
  // Set up the connexion to see the values from the moisture sensor:
  Serial.begin(9600);
  // Set the pin modes: 
  pinMode(relay, OUTPUT);
  pinMode(tank_floater, INPUT_PULLUP);
  
  Serial.println("Starting TelegramBot...");
  // myBot.setMaxConnectionRetries(15); // try 15 times to connect to the specified SSID (mySSID)
  // connect the ESP8266 to the desired access point
  myBot.wifiConnect(ssid, pass);

  // set the telegram bot token
  myBot.setTelegramToken(token);
  
  // check if all things are ok
  if (myBot.testConnection()){
    Serial.println("\nTelegram connection test: OK");
  } else {
    Serial.println("\nTelegram connection test: problem");
  }

  // Inline keyboard for Telegram (set the buttons):
  // Add a query button to the first row of the inline keyboard to know if the system is ON:
  myKbd.addButton("Is The system ON?", IS_SYSTEM_ON_CALLBACK, CTBotKeyboardButtonQuery);
  // Add another query button to the first row of the inline keyboard to get the soil moisture:
  myKbd.addButton("Show soil moisture", SOIL_MOISTURE_CALLBACK, CTBotKeyboardButtonQuery);
  // add a new empty button row
  myKbd.addRow();
  // add a query button to the second row of the inline keyboard for setting the system OFF:
  myKbd.addButton("Shut down the system", SYSTEM_OFF_CALLBACK, CTBotKeyboardButtonQuery);
  // add another query button to the second row of the inline keyboard for setting the system ON:
  myKbd.addButton("Turn on the system", SYSTEM_ON_CALLBACK, CTBotKeyboardButtonQuery);
  // add a new empty button row
  myKbd.addRow();
  // add another query button to the third row of the inline keyboard for setting the water pump ON:
  myKbd.addButton((String)"Turn on the water pump for "+(manual_watering_duration/1000)+" seconds", PUMP_ON_CALLBACK, CTBotKeyboardButtonQuery);
  // add a new empty button row
  myKbd.addRow();
  // add a URL button to the fourth row of the inline keyboard for documentation:
  myKbd.addButton("see docs", "https://github.com/shurillu/CTBot", CTBotKeyboardButtonURL);
}

void loop() {
  // Read the input soil moisture on analog pin A0:
  moisture = analogRead(soil_moisture);
  moisture_perc = map(moisture,WaterValue,AirValue,100,0);
  
  // Read the input from the float sensor in the water tank:
  water_level= digitalRead(tank_floater);
  TBMessage msg; // Store telegram message

  // Check if there is an incoming message from Telegram:
  if (myBot.getNewMessage(msg)) {
    // check what kind of message was received
    if (msg.messageType == CTBotMessageText) {
      // received a text message
      if (msg.text.equalsIgnoreCase("Show keyboard")) {
        // the user is asking to show the inline keyboard --> show it
        myBot.sendMessage(msg.sender.id, "Inline Keyboard", myKbd);
      } else {
        // the user write anithing else --> show a hint message
        myBot.sendMessage(msg.sender.id, "Try 'Show keyboard' to get the keyboard options.");
      }
    } else if (msg.messageType == CTBotMessageQuery) {
      // received a callback query message
      if (msg.callbackQueryData.equals(IS_SYSTEM_ON_CALLBACK)) {
        // Say if the system is running: 
        myBot.endQuery(msg.callbackQueryID, "The system is up and running!", true);
      } else if (msg.callbackQueryData.equals(SOIL_MOISTURE_CALLBACK)) {
        // Return the soil moisture as a popup message
        myBot.endQuery(msg.callbackQueryID, (String)"Soil Moisture : "+moisture_perc+"%"+"("+moisture+")",true);
      } else if (msg.callbackQueryData.equals(SYSTEM_OFF_CALLBACK)) {
        // Turn off the system.
        system_on = false;
        myBot.endQuery(msg.callbackQueryID, "Shutting down the system");
      } else if (msg.callbackQueryData.equals(SYSTEM_ON_CALLBACK)) {
        // Return the soil moisture as a popup message
        system_on = true;
        myBot.endQuery(msg.callbackQueryID, "Re-starting the system");
      } else if (msg.callbackQueryData.equals(PUMP_ON_CALLBACK)) {
        // Turn on the pump manually.
        pump_manual_state = true;
        myBot.endQuery(msg.callbackQueryID, (String)"Starting manual watering for "+(manual_watering_duration/1000)+" second");
      }
    }
  }

  // Start of the actual program for the system (only if system_on is ON): 
  if (system_on){
    
    if (water_level == prev_water_level) {
      is_level_change = false;
    } else {
      is_level_change = true;
    }
  
    if (water_level == HIGH) {
      // If there is water in the tank...
      // Print the moisture to the console: 
      Serial.print((String)"Soil Moisture : "+moisture_perc+"%"+"("+moisture+")"+"\n");
  
      // If tank was just refilled: 
      if (is_level_change) {
        // Sending a message via Telegram (only if it wasn't said already, and it didn't change just before)
        if ((millis() > Bot_lasttime + Bot_mtbs))  {
          myBot.sendMessage(chatID, "Thank you for refilling the water tank !");
          Bot_lasttime = millis();
        }
      }

      Serial.print((String)"last_watering_end  : "+last_watering_end +", watering_interval_min : "+watering_interval_min+"\n");

      if ((((long(millis()) > last_watering_end + watering_interval_min)||unexpected_watering_end) && (moisture_perc <= 30)) || pump_manual_state) {

        // /!\ START WATERING /!\
        // NB: watering can only start after a minimum given period of time (watering_interval_min), or if it terminated unexpectedly just before;
        // or if the user request a manual activation (pump_manual_state).
        
        last_watering_start= millis(); // Record the time of watering start

        // Resetting the flag for unexpected end of watering to false when entering the watering (if stopped because the tank was empty):
        unexpected_watering_end = false;
        
        myBot.sendMessage(chatID, "Starting plant wattering !");

        Serial.print((String)"Entering watering loop. Moisture : "+moisture_perc+", min_soil_moisture : "+min_soil_moisture+"\n");
        
        while ((moisture_perc <= min_soil_moisture) || pump_manual_state) {

          // Check if there is water in the tank:
          water_level= digitalRead(tank_floater);
          
          Serial.print((String)"water_level : "+water_level+"\n");

          if(water_level == LOW){
            // If there is no more water in the tank during a watering event, stop the pump, and stop the event.
            digitalWrite(relay, LOW);
            myBot.sendMessage(chatID, "Hu-ho, I emptied the water tank during plant watering, please re-fill!");
            // NB: not using `last_watering_end= millis();` here because we want to continue the plant watering whenever the tank is re-filled

            // Flag the waterring as not finished so we can re-enter the loop whenever the tank is re-filled: 
            unexpected_watering_end = true;
            break;
          }          

          Serial.print((String)"last_watering_start : "+last_watering_start+", watering_max_time : "+watering_max_time+"\n");

          if(long(millis()) > last_watering_start + watering_max_time){
            // Make sure the system is not watering for more than watering_max_time, and if so, stop watering and make sure the pump is shut off.
            last_watering_end = millis();
            digitalWrite(relay, LOW);
            myBot.sendMessage(chatID, (String)"Plants were wattered during "+((millis()-last_watering_start)/1000)+" seconds but they are still thirsty. Try to set watering_max_time to a higher value");
            break;
          }

          if(pump_manual_state && (millis() > last_watering_start + manual_watering_duration)){
            // If the user request a manual waterring, water until manual_watering_duration
            last_watering_end = millis();
            pump_manual_state = false;
            digitalWrite(relay, LOW);
            myBot.sendMessage(chatID, (String)"Plants were wattered during "+((millis()-last_watering_start)/1000)+" seconds after your request.");
            break;
          }

          Serial.print((String)"Start waterring !\n");

          // If everything is OK, start watering:
          digitalWrite(relay, HIGH);
          moisture = analogRead(soil_moisture);
          moisture_perc = map(moisture,WaterValue,AirValue,100,0);
          
          last_watering_end= millis();  
          delay(500);                // 500ms delay
        }

        // Shut down the pump when wattering is finished (happens here only when soil_moisture > min_soil_moisture)
        digitalWrite(relay, LOW);
        myBot.sendMessage(chatID, (String)"Wattered the plants during "+((last_watering_end-last_watering_start)/1000)+" seconds");
      }
  
      if ((last_watering_end > (long(millis()) + max_wo_water)) && (millis() > Bot_lasttime + Bot_mtbs_2)){
        // NB: using long(millis()) because at first last_watering_end is negative and millis() is an unsigned long.
        myBot.sendMessage(chatID, (String)"Plants were not wattered since "+(max_wo_water/86400000)+" days. Please check the system.");
        Bot_lasttime = millis();
      }
    } else {
      // If there is no water in the tank, problem !!
      // Powering down the pump in case it was running:
      digitalWrite(relay, LOW);
      Serial.print((String)"Water tank empty. Please refill with water !"+"(soil Moisture : "+moisture_perc+"%)"+"\n");
  
      // Sending a message via Telegram (only if it wasn't said already, and it didn't change just before)
      if ((millis() > Bot_lasttime + Bot_mtbs) && is_level_change)  {
        myBot.sendMessage(chatID, "The water tank is empty, please refill before all plants die !");
        Bot_lasttime = millis();
      }
    }
  }
  
  // Always make sure the water pump is OFF at the end of the loop just in case:
  digitalWrite(relay, LOW);
    
  prev_water_level= water_level;
  delay(interval);    
}
