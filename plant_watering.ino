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

#define LIGHT_ON_CALLBACK  "IsSystemON"  // callback data sent when "LIGHT ON" button is pressed
#define LIGHT_OFF_CALLBACK "lightOFF" // callback data sent when "LIGHT OFF" button is pressed

int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done

#define interval 1000 // Interval between readings

// Define pins: 
#define relay 13
#define soil_moisture  A0
#define tank_floater  16

// Floater in the water tank: 
int water_level = 2;
bool is_level_change= false; // Keep trace of a change in tank level
int prev_water_level= 2;     // To keep trace of the water level from the previous loop execution

// Soil Moisture sensor: 
int moisture= 0;             // Initializing moisture to 0
int moisture_perc= 0;        // Moisture in %
const int AirValue = 780;    // Driest value -> air value
const int WaterValue = 280;  // Wettest value -> water value

// Timer to avoid wattering for too long: 
int watering_max_time = 300000;       // Maximal time for one wattering event (5 minutes)
int watering_interval_min = 10800000; // Minimal time to wait before two waterring events (3 hours here)
long last_watering_start = 0.0 ;       // time since the waterring event as been started
long last_watering_end = -watering_interval_min; // time since the last waterring event as been ended
// NB: initialized at -watering_interval_min to be able to start wattering at startup.
long max_wo_water = 259200000; // Maximum days without wattering until notification to say there's probably an issue with th system



void setup() {
  // Set up the connexion to see the values from the moisture sensor:
  Serial.begin(9600);
  // Set the pin modes: 
  pinMode(relay, OUTPUT);
  pinMode(tank_floater, INPUT_PULLUP);
  
  Serial.println("Starting TelegramBot...");
  myBot.setMaxConnectionRetries(15); // try 15 times to connect to the specified SSID (mySSID)
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

  
  // inline keyboard customization for Telegram
  // add a query button to the first row of the inline keyboard
  myKbd.addButton("Is The system ON?", LIGHT_ON_CALLBACK, CTBotKeyboardButtonQuery);
  // add another query button to the first row of the inline keyboard
  myKbd.addButton("Show soil moisture", LIGHT_OFF_CALLBACK, CTBotKeyboardButtonQuery);
  // add a new empty button row
  myKbd.addRow();
  // add a URL button to the second row of the inline keyboard
  myKbd.addButton("see docs", "https://github.com/shurillu/CTBot", CTBotKeyboardButtonURL);
}

void loop() {
  // Read the input soil moisture on analog pin A0:
  moisture = analogRead(soil_moisture);
  moisture_perc = map(moisture,WaterValue,AirValue,100,0);
  
  // Read the input from the float sensor in the water tank:
  water_level= digitalRead(tank_floater);
  TBMessage msg; // Store telegram message

  // if there is an incoming message from Telegram:
  if (myBot.getNewMessage(msg)) {
    // check what kind of message was received
    if (msg.messageType == CTBotMessageText) {
      // received a text message
      if (msg.text.equalsIgnoreCase("show keyboard")) {
        // the user is asking to show the inline keyboard --> show it
        myBot.sendMessage(msg.sender.id, "Inline Keyboard", myKbd);
      } else {
        // the user write anithing else --> show a hint message
        myBot.sendMessage(msg.sender.id, "Try 'show keyboard' to get the keyboard options.");
      }
    } else if (msg.messageType == CTBotMessageQuery) {
      // received a callback query message
      if (msg.callbackQueryData.equals(LIGHT_ON_CALLBACK)) {
        // Say if the system is running: 
        myBot.endQuery(msg.callbackQueryID, "The system is up and running!", true);
      } else if (msg.callbackQueryData.equals(LIGHT_OFF_CALLBACK)) {
        // Return the soil moisture as a popup message
        myBot.endQuery(msg.callbackQueryID, (String)"Soil Moisture : "+moisture_perc+"%"+"("+moisture+")");
      }
    }
  }



  
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

    if ((millis() > last_watering_end + watering_interval_min) && (moisture_perc <= 30)) {
      // Waterring can only start after a minimum given period of time (watering_interval_min)
      last_watering_start= millis(); // starting wattering

      myBot.sendMessage(chatID, "Starting plant wattering !");
      
      while (moisture_perc <= 30) {
        digitalWrite(relay, HIGH);
        moisture = analogRead(soil_moisture);
        moisture_perc = map(moisture,WaterValue,AirValue,100,0);
        
        if(millis() > last_watering_start + watering_max_time){
          // If the system is wattering for more than watering_max_time, stop wattering
          myBot.sendMessage(chatID, (String)"Plants were wattered during "+((millis()-last_watering_start)/1000)+" seconds but they are still thirsty. Try to set watering_max_time to a higher value");
          break;
        }
        delay(500);                // 500ms delay
      } 
      
      last_watering_end= millis();  
      myBot.sendMessage(chatID, (String)"Wattered the plants during "+((last_watering_end-last_watering_start)/1000)+" seconds");
    }

    if (last_watering_end > (millis()+max_wo_water)){
      myBot.sendMessage(chatID, (String)"Plants were not wattered since"+(max_wo_water/86400000)+"days. Please check the system.");
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

  // Always shut off the water pump in any other case:
  digitalWrite(relay, LOW);
    
  prev_water_level= water_level;
  delay(interval);    
}
