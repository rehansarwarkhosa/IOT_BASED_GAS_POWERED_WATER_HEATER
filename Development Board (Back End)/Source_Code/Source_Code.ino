#include <EEPROM.h>

const byte SOLENOID_COIL_PIN = 16;
const byte SENSOR_WARNING_PIN = 0; //LED //BUZZER

const byte GAS_SENSING_PIN = 14;
byte re_sense_gas_delay = 0;
boolean closed_due_to_gas = false;

boolean force_on = false;
boolean is_start_by_schedule = false;

const String CSS = "<style> body { margin:0; } a { background: linear-gradient(to top, #ffffcc 0%, #ffff00 100%); border: none; color: black; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 22pt; margin: 4px 2px; cursor: pointer; font-family: 'Muli', sans-serif; width: 250px; } h1 { font-weight:bold; text-shadow: 0 -3px rgba(0,0,0,0.6); font-size: 42pt; background: linear-gradient(to bottom, #33ccff 0%, #000099 100%); height: 100px; border: 1px solid #fff; padding: 5px 15px; color: white; border-radius: 0 10px 0 10px; font-family: 'Muli', sans-serif; } .footer { font-weight:bold; text-shadow: 0 -3px rgba(0,0,0,0.6); font-size: 22pt; background: linear-gradient(to top, #33ccff 0%, #000099 100%); border: 1px solid #fff; padding: 5px 15px; color: white; border-radius: 0 10px 0 10px; font-family: 'Muli', sans-serif; position: fixed; left: 0; bottom: 0; width: 100%; margin-bottom: 0; } #info{ border: none; font-weight: bold; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 22pt; margin: 4px 2px; cursor: pointer; font-family: 'Muli', sans-serif; } #table-heading { background-color: #0000cc; color: white; font-size: 22pt; font-family: 'Muli', sans-serif; padding: 5px 15px; } #table-content{ background-color: #e7e7e7; color: black; font-size: 22pt; font-family: 'Muli', sans-serif; padding: 5px 15px; text-align: center; } #entry-form{ font-size: 25pt; font-family: 'Muli', sans-serif; padding: 5px 15px; margin: 10px; font-weight: bold; } </style>";
const String NAVIGATION_HOME = "<table width='100%'> <tr><td></td><td><a href=\"http://192.168.4.1/\" style='border: 2px solid black;'>Home</a> </td> <td> <a href=\"http://192.168.4.1/http_set_temperature\">Set Temperature</a> </td> <td> <a href=\"http://192.168.4.1/http_new_schedule\">New Schedule</a> </td> </tr> </table>";
const String NAVIGATION_SET_TEMPERATURE= "<table width='100%'><tr><td></td><td> <a href=\"http://192.168.4.1/\">Home</a></td><td><a style='border: 2px solid black;' href=\"http://192.168.4.1/http_set_temperature\">Set Temperature</a> </td> <td> <a href=\"http://192.168.4.1/http_new_schedule\">New Schedule</a> </td> </tr> </table>";
const String NAVIGATION_NEW_SCHEDULE = "<table width='100%'> <tr><td></td><td><a href=\"http://192.168.4.1/\">Home</a> </td><td><a href=\"http://192.168.4.1/http_set_temperature\">Set Temperature</a> </td> <td> <a style='border: 2px solid black;' href=\"http://192.168.4.1/http_new_schedule\">New Schedule</a> </td> </tr> </table>";
const String HORIZONTAL_LINE = "<hr>";
const String TITLE = "<title>Gas-Powered Water Heater</title>";
const String FOOTER = "<h2 class='footer'></h2>";
const String HTML_OPEN = "<html>";
const String HTML_CLOSE = "</html>";
const String BODY_OPEN = "<body>";
const String BODY_CLOSE = "</body>";
const String HEAD_OPEN = "<head>";
const String HEAD_CLOSE = "</head>";
const String TEMPERATURE_INPUT_FORM = "<div style=\"background-color:#DCDCDC; padding: 50px;\"> <form method=\"GET\" action=\"http://192.168.4.1/\"> <table> <tr> <input type=\"hidden\" name=\"m\" value=\"t\"> <td><p id=\"entry-form\">Temperature:</p></td> <td><input id=\"entry-form\" type=\"number\" name=\"temperature\"></td> </tr> <tr> <td></td> <td><input id=\"entry-form\" type=\"submit\" value=\"Set\"></td> </tr> </table> </form> </div>";
const String SCHEDULE_INPUT_FORM = "<div style=\"background-color:#DCDCDC; padding: 50px;\"> <form method=\"GET\" action=\"http://192.168.4.1/\"> <table> <tr> <input type=\"hidden\" name=\"m\" value=\"c\"> <td><p id=\"entry-form\">Start hour:</p></td> <td><input id=\"entry-form\" type=\"number\" name=\"sth\"></td> </tr> <tr> <td><p id=\"entry-form\">Start minute:</p></td> <td><input id=\"entry-form\" type=\"number\" name=\"stm\"></td> </tr> <tr> <td><p id=\"entry-form\">End hour:</p></td> <td><input id=\"entry-form\" type=\"number\" name=\"eth\"></td> </tr> <tr> <td><p id=\"entry-form\">Start minute:</p></td> <td><input id=\"entry-form\" type=\"number\" name=\"etm\"></td> </tr> <tr> <td><p id=\"entry-form\">Day (0-7):</p></td> <td><input id=\"entry-form\" type=\"number\" name=\"day\"></td> </tr> <tr> <td></td> <td><input id=\"entry-form\" type=\"submit\" value=\"Save\"></td> </tr> </table> </form> </div>";
const String HEADER = "<h1>Gas-Powered Water Heater</h1>";
const String FORCE_ON_OFF = "<table width='100%'> <tr> <td> <a style=\"width: 85%; background: linear-gradient(to bottom, #ff0000 0%, #ff99cc 100%);\" href=\"http://192.168.4.1?m=s&switch=0\"><b>OFF</b></a> </td> <td> <a style=\"width: 85%; background: linear-gradient(to bottom, #009900 0%, #00ff99 100%);\" href=\"http://192.168.4.1?m=s&switch=1\"><b>ON</b></a> </td> </tr> </table>";
//-----------------WiFi------------------------------------------------
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#ifndef APSSID
#define APSSID "Geyser"
#define APPSK  "12345678"
#endif

const char *ssid = APSSID;
const char *password = APPSK;

ESP8266WebServer server(80);
//------------------RTC Library and Variable Declartion----------------
#include "RTClib.h"
RTC_DS3231 rtc;

DateTime now;

int _year;
int _month;
int _day;

int _hour;
int _minute;
int _second;

String get_RTC_data;


const byte DAILY_SCHEDULE = 7;
//---------------------------------------------------------------------

//----------------------Temperature Sensor Libraries and variable Declartion----------------------------
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);


boolean isTemperatureReached = false;
byte current_temperature_from_sensor;
//--------------------------------------------------------------------

const int EEPROM_ADDRESS_FOR_TEMPERATURE = 0;
const int EEPROM_ADDRESS_FOR_SCHEDULE_COUNT = 1;
const int EEPROM_ADDRESS_FOR_SCHEDULE_START = 2;

char charTemporary[2]; // temporary char array for conversion
int indx = 0; // variable for loop iteration

struct Schedule {
  byte STH;
  byte STM;
  byte ETH;
  byte ETM;
  byte Day;
};

Schedule retrieve_schedule_from_EEPROM;


void remove_all_schedule() {
  EEPROM.write(EEPROM_ADDRESS_FOR_SCHEDULE_COUNT, 0);
  Schedule dummySchedule = {99, 99, 99, 99, 99};
  for (int eeprom_address = EEPROM_ADDRESS_FOR_SCHEDULE_START; eeprom_address < (EEPROM.length() - 6); eeprom_address += sizeof(Schedule))
  {
    EEPROM.put(eeprom_address, dummySchedule);
  }
}


void remove_single_schedule(Schedule rmSchedule) {
  Schedule searchedSchedule;
  Schedule dummySchedule = {99, 99, 99, 99, 99};
  for (int eeprom_address = EEPROM_ADDRESS_FOR_SCHEDULE_START; eeprom_address < (EEPROM.length() - 6); eeprom_address += sizeof(Schedule))
  {
    EEPROM.get(eeprom_address, searchedSchedule);
    if (searchedSchedule.STH != 99)
    {
      if (searchedSchedule.Day == rmSchedule.Day &&
          searchedSchedule.STH == rmSchedule.STH &&
          searchedSchedule.ETH == rmSchedule.ETH &&
          searchedSchedule.STM == rmSchedule.STM &&
          searchedSchedule.ETM == rmSchedule.ETM)
      {
        Serial.println("\n##############################");
        Serial.println("One Schedule removed from EEPROM!");
        Serial.println("##############################\n");
        EEPROM.put(eeprom_address, dummySchedule);
        EEPROM.write(EEPROM_ADDRESS_FOR_SCHEDULE_COUNT, EEPROM.read(EEPROM_ADDRESS_FOR_SCHEDULE_COUNT) - 1);
        return;
      }
    }
  }
}

boolean is_valid_schedule(Schedule newSchedule) {
  Schedule dummySchedule;
  for (int eeprom_address = EEPROM_ADDRESS_FOR_SCHEDULE_START; eeprom_address < (EEPROM.length() - 6); eeprom_address += sizeof(Schedule))
  {
    EEPROM.get(eeprom_address, dummySchedule);
    if (dummySchedule.STH != 99)
    {
      if (dummySchedule.Day == newSchedule.Day || newSchedule.Day == DAILY_SCHEDULE)
      {
        // If new schedule is in between existing and also check duplicate
        if (dummySchedule.STH <= newSchedule.STH && dummySchedule.ETH >= newSchedule.ETH)
        {
          Serial.println("ERROR 101");
          return false;
        }
        // If new schedule STH in between existing schedule but ETH outside the existing schedule
        else if (newSchedule.STH > dummySchedule.STH && newSchedule.STH < dummySchedule.ETH)
        {
          Serial.println("ERROR 102");
          return false;
        } else if ((newSchedule.STH == dummySchedule.STH && newSchedule.STM >= dummySchedule.STM) &&
                   (newSchedule.STH == dummySchedule.ETH && newSchedule.STM <= dummySchedule.ETM))
        {
          Serial.println("ERROR 103");
          return false;
        } else if (newSchedule.STH == dummySchedule.ETH && newSchedule.STM <= dummySchedule.ETM)
        {
          Serial.println("ERROR: 104");
          return false;
        }
        // If new schedule ETH in between existing schedule but STH outside the existing schedule
        else if (newSchedule.ETH > dummySchedule.STH && newSchedule.ETH < dummySchedule.ETH)
        {
          Serial.println("ERROR 105");
          return false;
        }
        else if ((newSchedule.ETH == dummySchedule.ETH && newSchedule.ETM <= dummySchedule.ETM) &&
                 (newSchedule.ETH == dummySchedule.STH && newSchedule.ETM >= dummySchedule.STM))
        {
          Serial.println("ERROR 106");
          return false;
        } else if (newSchedule.ETH == dummySchedule.STH && newSchedule.ETM >= dummySchedule.STM)
        {
          Serial.println("ERROR 107");
          return false;
        }
        // If existing schedules between new schedule
        else if (newSchedule.STH < dummySchedule.STH && newSchedule.ETH > dummySchedule.ETH)
        {
          Serial.println("ERROR 108");
          return false;
        }
      }
    }
  }
  return true;
}

void write_temperature(byte temperature) {
  EEPROM.write(EEPROM_ADDRESS_FOR_TEMPERATURE, temperature);
}

void write_schedule(Schedule newSchedule) {
  if (is_valid_schedule(newSchedule))
  {
    Schedule dummySchedule;
    for (int eeprom_address = EEPROM_ADDRESS_FOR_SCHEDULE_START; eeprom_address < (EEPROM.length() - 6); eeprom_address += sizeof(Schedule))
    {
      EEPROM.get(eeprom_address, dummySchedule);
      if (dummySchedule.STH == 99) {
        EEPROM.put(eeprom_address, newSchedule);
        EEPROM.write(EEPROM_ADDRESS_FOR_SCHEDULE_COUNT, EEPROM.read(EEPROM_ADDRESS_FOR_SCHEDULE_COUNT) + 1);
        return;
      }
    }
  } else
  {
    Serial.println("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    Serial.println("Inserted schedule is duplicate or invalid!");
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
  }

}

void display_temperature_from_EEPROM() {
  Serial.println("\n******************************");
  Serial.print("Temperature from EEPROM: ");
  Serial.println(EEPROM.read(0));
  Serial.println("******************************\n");
}

void display_all_schedule() {
  Schedule dummySchedule;
  for (int eeprom_address = EEPROM_ADDRESS_FOR_SCHEDULE_START; eeprom_address < (EEPROM.length() - 6); eeprom_address += sizeof(Schedule))
  {
    EEPROM.get(eeprom_address, dummySchedule);
    if (dummySchedule.STH != 99) {

      Serial.print("Start time: ");
      Serial.print(dummySchedule.STH);
      Serial.print(":");
      Serial.print(dummySchedule.STM);

      Serial.println();

      Serial.print("End time: ");
      Serial.print(dummySchedule.ETH);
      Serial.print(":");
      Serial.print(dummySchedule.ETM);

      Serial.println();

      Serial.print("Day: ");
      Serial.println(dummySchedule.Day);
      Serial.print("Total num of Schedule: ");
      Serial.println(EEPROM.read(EEPROM_ADDRESS_FOR_SCHEDULE_COUNT));
      Serial.println("------------------------------");
    }
  }
}

void set_RTC_time_and_date() {
  rtc.adjust(DateTime(_year, _month, _day, _hour, _minute, _second));
  // following line sets the RTC to the date & time this sketch was compiled
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // year day month hour minute second
  //rtc.adjust(DateTime(2020, 3, 2, 3, 55, 59));
  //Serial.println("\n\n\n\n\n\n");
  //Serial.println(_year);
  //Serial.println(_day);
  //Serial.println(_month);
  //Serial.println(_hour);
  //Serial.println(_minute);
  //Serial.println(_second);
  //Serial.println("\n\n\n\n\n\n");
}

void operation() {

  //Serial.println("******************************");
  //Serial.print("Temperature from sensor: ");
  //Serial.println(get_temperature_from_sensor());
  //Serial.println("******************************\n");

  //-------------------------Temperature Sensor Logic------------------------------------
  if (get_temperature_from_sensor() <= (get_temperature_from_EEPROM() - 3))
  {
    isTemperatureReached = false;
    if (force_on)
    {
      switch_on();
    }
  }

  if (get_temperature_from_sensor() >= get_temperature_from_EEPROM())
  {
    isTemperatureReached = true;
    switch_off();
  }
  //-----------------------------------------------------------------------------------
  if (!isTemperatureReached && !force_on)
  {
    //Serial.println("\n\nSchedule invoked!\n");
    //Serial.print("RTC_Hour: ");
    //Serial.println(get_hour_from_RTC());
    //Serial.print("RTC_Minute: ");
    //Serial.println(get_minute_from_RTC());
    //Serial.print("RTC_Day_Of_The_Week: ");
    //Serial.println(get_day_of_the_week_from_RTC());

    is_start_by_schedule = false;

    //Compare  EEPROM saved schedule with current time and date after temperature
    for (int eeprom_address = EEPROM_ADDRESS_FOR_SCHEDULE_START; eeprom_address < (EEPROM.length() - 6); eeprom_address += sizeof(Schedule))
    {
      EEPROM.get(eeprom_address, retrieve_schedule_from_EEPROM);
      if (retrieve_schedule_from_EEPROM.STH != 99)
      {
        if (get_day_of_the_week_from_RTC() == retrieve_schedule_from_EEPROM.Day || retrieve_schedule_from_EEPROM.Day == DAILY_SCHEDULE)
        {
          //Serial.println("Schedule: 1 Condition true!");
          //-------------------------------------ON LOGIC START---------------------------------------------------------------------------
          if ((get_hour_from_RTC() == retrieve_schedule_from_EEPROM.STH && get_minute_from_RTC() >= retrieve_schedule_from_EEPROM.STM) ||
              get_hour_from_RTC() > retrieve_schedule_from_EEPROM.STH)
          {
            //Serial.println("Schedule: 2 Condition true!");
            if (get_hour_from_RTC() == retrieve_schedule_from_EEPROM.ETH && get_minute_from_RTC() < retrieve_schedule_from_EEPROM.ETM)
            {
              //Serial.println("Schedule: 3 Condition true!");
              is_start_by_schedule = true;
              switch_on();
            } else if (get_hour_from_RTC() < retrieve_schedule_from_EEPROM.ETH)
            {
              //Serial.println("Schedule: 4 Condition true!");
              is_start_by_schedule = true;
              switch_on();
            }
          }
          //------------------------------------ON LOGIC END------------------------------------------------------------------------------------------
        }
      }
    }// End for loop
    if (!is_start_by_schedule) {
      switch_off();
    }
  }
}

byte get_total_num_of_schedule_from_EEPROM() {
  return EEPROM.read(EEPROM_ADDRESS_FOR_SCHEDULE_COUNT);
}

byte get_temperature_from_EEPROM() {
  return EEPROM.read(0);
}

byte get_temperature_from_sensor() {
  return current_temperature_from_sensor;
}

byte get_hour_from_RTC() {
  now = rtc.now(); // Get current date and time
  char buf1[] = "hh"; // Not declared outside the current scope. Only work as a local array not work with global
  get_RTC_data = now.toString(buf1);
  charTemporary[0] = get_RTC_data.charAt(0);
  charTemporary[1] = get_RTC_data.charAt(1);
  return atoi(charTemporary);
}

byte get_minute_from_RTC() {
  now = rtc.now(); // Get current date and time
  char buf1[] = "mm"; // Not declared outside the current scope. Only work as a local array not work with global
  get_RTC_data = now.toString(buf1);
  charTemporary[0] = get_RTC_data.charAt(0);
  charTemporary[1] = get_RTC_data.charAt(1);
  return atoi(charTemporary);
}

byte get_day_of_the_week_from_RTC() {
  return now.dayOfTheWeek(); // Day 0 to 6
}

void switch_on() {
  digitalWrite(SOLENOID_COIL_PIN, HIGH);
  delay(9000);
}

void switch_off() {
  digitalWrite(SOLENOID_COIL_PIN, LOW);
}

void sense_gas(){

  //boolean state_force_on = force_on;
  //boolean state_is_start_by_schedule = is_start_by_schedule;

if(!closed_due_to_gas){
  
  if(force_on == true || is_start_by_schedule == true)
  {
    if(digitalRead(GAS_SENSING_PIN) == HIGH)
    {
      switch_off();
      closed_due_to_gas = true;       
    }
  }
  
}


if(closed_due_to_gas){
  re_sense_gas_delay++;

  if(re_sense_gas_delay >=50)
  {
    if(force_on)
    {
      switch_on();
    }
    closed_due_to_gas = false;
    re_sense_gas_delay = 0;
  }
}
  
}// End Sense_Gas


DateTime get_current_date_and_time_from_RTC(){
  return rtc.now();
}

void print_current_date_and_time_from_RTC() {
  Serial.println("\n~~~~~~~~~~~~~Real Time Clock~~~~~~~~~~~~~~~");
  DateTime nw = rtc.now();

  Serial.print("Year: ");
  Serial.println(nw.year(), DEC);

  Serial.print("Month: ");
  Serial.println(nw.month(), DEC);

  Serial.print("Day: ");
  Serial.println(nw.day(), DEC);

  Serial.println();

  Serial.print(nw.hour(), DEC);
  Serial.print(':');
  Serial.print(nw.minute(), DEC);
  Serial.print(':');
  Serial.println(nw.second(), DEC);

  Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

void sensor_warning() {
  digitalWrite(SENSOR_WARNING_PIN, HIGH);
  delay(400);
  digitalWrite(SENSOR_WARNING_PIN, LOW);
  delay(400);
}

String http_dashboard(){
  
  String state = "OFF";
  if(digitalRead(SOLENOID_COIL_PIN) == HIGH){
    state = "ON";
  }

  DateTime currentDateTime = get_current_date_and_time_from_RTC();
  
  return "<table>" +
          String("<tr>") +
          "<td><p id=\"info\">Current State:</p></td>" +
          "<td><p id=\"info\">" +
          state +
          "</p></td>" +
          "</tr>" +
          "<tr>" + 
          "<td><p id=\"info\">Water Temperature:</p></td>" +
          "<td><p id=\"info\">" +
          get_temperature_from_sensor() +
          "</p></td>" +
          "</tr>" +
          "<tr>" +
          "<td><p id=\"info\">Fixed Temperature:</p></td>" +
          "<td><p id=\"info\">" +
          get_temperature_from_EEPROM() +
          "</p></td>" +
          "</tr>" +
          "<tr>" +
          "<td><p id=\"info\">Total number of schedule:</p></td>" +
          "<td><p id=\"info\">" +
          get_total_num_of_schedule_from_EEPROM() +
          "</tr>" +
          "<tr>" +
          "<td><p id=\"info\">Internal Clock:</p></td>" +
          "<td><p id=\"info\">" +
          currentDateTime.hour() + ":" + currentDateTime.minute() + " | " + currentDateTime.day() + "-" + currentDateTime.month() + "-" + currentDateTime.year() +
          "</p></td>" +
          "</tr>" +
          "</table>";
}

String http_schedule_list(){
  String content =  "<marquee><p style='color: Gray; font-size: 22pt;'><b>Day-0:</b> Monday, <b>Day-1:</b> Tuesday, <b>Day-2:</b> Wednesday, <b>Day-3:</b> Thursday, <b>Day-4:</b> Friday, <b>Day-5:</b> Saturday, <b>Day-6:</b> Sunday, <b>Day-7:</b> Everyday</p></marquee>" +
          String("<table width='100%'>") +
          String("<tr>") +
          "<td><p id=\"table-heading\">Start Hour</p></td>" +
          "<td><p id=\"table-heading\">Start Minute</p></td>" +
          "<td><p id=\"table-heading\">End Hour</p></td>" +
          "<td><p id=\"table-heading\">End Minute</p></td>" +
          "<td><p id=\"table-heading\">Day &nbsp;&nbsp;(0-7)</p></td>" +
          "<td></td>" +
          "</tr>";

  Schedule dummySchedule;
  for (int eeprom_address = EEPROM_ADDRESS_FOR_SCHEDULE_START; eeprom_address < (EEPROM.length() - 6); eeprom_address += sizeof(Schedule))
  {
    EEPROM.get(eeprom_address, dummySchedule);
    if (dummySchedule.STH != 99) {

      content += "<tr>";
      
      content += "<td><p id=\"table-content\">";
      content += dummySchedule.STH;
      content += "</p></td>";

      content += "<td><p id=\"table-content\">";
      content += dummySchedule.STM;
      content += "</p></td>";

      content += "<td><p id=\"table-content\">";
      content += dummySchedule.ETH;
      content += "</p></td>";

      content += "<td><p id=\"table-content\">";
      content += dummySchedule.ETM;
      content += "</p></td>";

      content += "<td><p id=\"table-content\">";
      content += dummySchedule.Day;
      content += "</p></td>";

      content += "<td><a style=\"background: linear-gradient(to bottom, #ff3300 0%, #ff5050 100%); color: white; width: 100px;\" href='http://192.168.4.1?m=r&"+ String("sth=") + dummySchedule.STH + "&stm=" + dummySchedule.STM + "&eth=" + dummySchedule.ETH + "&etm=" + dummySchedule.ETM + "&day=" + dummySchedule.Day + "'>Delete</a></td>";
      content += "</tr>";
    }
  }

  content += "</table>";
  
          

  return content;
}

void http_set_temperature(){
  server.send(200, "text/html", HTML_OPEN + HEAD_OPEN + TITLE + CSS + HEAD_CLOSE + BODY_OPEN + HEADER + HORIZONTAL_LINE + NAVIGATION_SET_TEMPERATURE + HORIZONTAL_LINE + FORCE_ON_OFF  + HORIZONTAL_LINE + http_dashboard() + HORIZONTAL_LINE + TEMPERATURE_INPUT_FORM + FOOTER + BODY_CLOSE + HTML_CLOSE);
}

void http_new_schedule(){
  server.send(200, "text/html", HTML_OPEN + HEAD_OPEN + TITLE + CSS + HEAD_CLOSE + BODY_OPEN + HEADER + HORIZONTAL_LINE + NAVIGATION_NEW_SCHEDULE + HORIZONTAL_LINE + FORCE_ON_OFF  + HORIZONTAL_LINE + http_dashboard() + HORIZONTAL_LINE + SCHEDULE_INPUT_FORM + FOOTER + BODY_CLOSE + HTML_CLOSE);
}

void handleRoot() {
  //for (uint8_t i = 0; i < server.args(); i++) {
  //message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  //}

  if (server.arg(0).equals("a")) {

    _year = server.arg(1).toInt();
    _month = server.arg(2).toInt();
    _day = server.arg(3).toInt();
    _hour = server.arg(4).toInt();
    _minute = server.arg(5).toInt();
    _second = server.arg(6).toInt();

    set_RTC_time_and_date();

  } else if (server.arg(0).equals("d")) {

    remove_all_schedule();
    EEPROM.commit();

  } else if (server.arg(0).equals("s")) {

    if (server.arg(1).toInt() == 1) {
      if(!closed_due_to_gas){
        force_on = true;
        switch_on();
      }
    } else {
      force_on = false;
      switch_off();
    }

  } else if (server.arg(0).equals("t")) {

    write_temperature(server.arg(1).toInt());
    EEPROM.commit();

  } else if (server.arg(0).equals("c")) {

    Schedule newSchedule;

    newSchedule.STH = server.arg(1).toInt();
    newSchedule.STM = server.arg(2).toInt();
    newSchedule.ETH = server.arg(3).toInt();
    newSchedule.ETM = server.arg(4).toInt();
    newSchedule.Day = server.arg(5).toInt();

    write_schedule(newSchedule);
    EEPROM.commit();

  } else if (server.arg(0).equals("r")) {

    Schedule rmSchedule;

    rmSchedule.STH = server.arg(1).toInt();
    rmSchedule.STM = server.arg(2).toInt();
    rmSchedule.ETH = server.arg(3).toInt();
    rmSchedule.ETM = server.arg(4).toInt();
    rmSchedule.Day = server.arg(5).toInt();

    remove_single_schedule(rmSchedule);
    EEPROM.commit();

  }



server.send(200, "text/html", HTML_OPEN + HEAD_OPEN + TITLE + CSS + HEAD_CLOSE + BODY_OPEN + HEADER + HORIZONTAL_LINE + NAVIGATION_HOME + HORIZONTAL_LINE + FORCE_ON_OFF  + HORIZONTAL_LINE + http_dashboard() + HORIZONTAL_LINE + http_schedule_list() + FOOTER + BODY_CLOSE + HTML_CLOSE);

}

void setup() {
  pinMode(SOLENOID_COIL_PIN, OUTPUT);
  digitalWrite(SOLENOID_COIL_PIN, LOW);
  
  Serial.begin(115200);
  delay(100);
  EEPROM.begin(1024);

  sensors.begin();  //Start to listen the temprature sensor
  delay(100);

  pinMode(GAS_SENSING_PIN, INPUT);

  pinMode(SENSOR_WARNING_PIN, OUTPUT);
  digitalWrite(SENSOR_WARNING_PIN, LOW);

  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  server.on("/", handleRoot);
  server.on("/http_set_temperature", http_set_temperature);
  server.on("/http_new_schedule", http_new_schedule);
  server.begin();

  delay(100);
}

void loop() {

  server.handleClient();


  while(!rtc.begin())
  {
    Serial.println("RTC sensor not working properly!");
    digitalWrite(SOLENOID_COIL_PIN, LOW);
    force_on = false;
    
    sensor_warning();
  }

   sensors.requestTemperatures(); // Send the command to get temperatures
   float tempC = sensors.getTempCByIndex(0);
   while(tempC == DEVICE_DISCONNECTED_C) 
  {
    Serial.println("Temperature sensor not working properly!");
    digitalWrite(SOLENOID_COIL_PIN, LOW);
    force_on = false;

    sensor_warning();
    sensors.requestTemperatures(); // Send the command to get temperatures
    tempC = sensors.getTempCByIndex(0);
  }
  current_temperature_from_sensor = (byte) tempC; 

  /*
    //Populate character array with serial data
    indx = 0;
    while (Serial.available())
    {
      charRead[indx] = (char) Serial.read();
      if (indx < MAX_INDEX) {
        ++indx;
      }
    }
  */

  //for(int i=0; i<MAX_INDEX; ++i){
  //Serial.print(charRead[i]);
  //}



  //display_all_schedule();
  //display_temperature_from_EEPROM();
  //print_current_date_and_time_from_RTC();

  //Serial.println("\n------Operation() start");
  delay(100);

   sense_gas();
  //Serial.println("------Operation() end\n");

  delay(100);

  if(!closed_due_to_gas){
    operation();
  }

  delay(100);
}
