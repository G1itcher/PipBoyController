/*
 *  This sketch demonstrates how to set up a simple HTTP-like server.
 *  The server will set a GPIO pin depending on the request
 *    http://server_ip/gpio/0 will set the GPIO2 low,
 *    http://server_ip/gpio/1 will set the GPIO2 high
 *  server_ip is the IP address of the ESP8266 module, will be 
 *  printed to Serial when the module is connected.
 */

#include <ESP8266WiFi.h>
#include <string>

const char* ssid = "ASUS";
const char* password = "h0m3n3tw0rk";

const float AUTO_OFF_DELAY = 5000;

const int OFF_VALUE = 1;
const int ON_VALUE = 0;
const int PIN = 14;

const int MAX_FAILS_BEFORE_RESTART = 5;

unsigned long offAt = 0;                   // How long since the last on signal?
bool isOn = false;
int failCount = 0;

std::string host = "autoremotejoaomgcd.appspot.com";
std::string getParams = "/sendmessage?key=APA91bH0vRbYArMB3L1D8hv1S6Ptc8YvSxl2yorpitycZUiTiilNraalTHHyrNLsaPqHH4AEjbGHP2DrNUFAgyjhxifVD7axPevTzbuC5KmwgHdDIwe2sggyTbdhT7fLwx0CXAo4wvdg&message=";
std::string onCommand = "on";
std::string offCommand = "of";

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

// To connect to the on/off server


bool setScreenState(const std::string state)
{
    WiFiClient httpClient;
    Serial.println("Making Request with state:");
    Serial.println(state.c_str());

    bool success = false;
    if (httpClient.connect(host.c_str(), 80)) {
        Serial.println("connected to server");
        Serial.println("Sending:");
        Serial.println(std::string("GET " + getParams + state + " HTTP/1.1").c_str());
        Serial.println(std::string("Host: " + host).c_str());
        Serial.println(std::string("Connection: close").c_str());
        // Make a HTTP request:
        httpClient.println(std::string("GET " + getParams + state + " HTTP/1.1").c_str());
        httpClient.println(std::string("Host: " + host).c_str());
        httpClient.println(std::string("Connection: close").c_str());
        httpClient.println();
        success = httpClient.readStringUntil('\r').indexOf("200") > -1;
        httpClient.stop();
    }
    Serial.println("Request Success: ");
    Serial.println(success);
    return success;
}

void setup() {
    ESP.wdtDisable();
    Serial.begin(115200);
    delay(10);

    // prepare GPIO2
    pinMode(PIN, OUTPUT);
    digitalWrite(PIN, 1);

    // Connect to WiFi network
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
    Serial.println("WiFi connected");

    // Start the server
    server.begin();
    Serial.println("Server started");

    //Reset the screen state
    while(!setScreenState(offCommand))
    {
      if(++failCount > MAX_FAILS_BEFORE_RESTART)
      {
        ESP.restart();
      }
    }

    // Print the IP address
    Serial.println(WiFi.localIP());
    //Activate 8 second watchdog
    ESP.wdtEnable(8000);
}

void loop() {
    if(WiFi.status() != WL_CONNECTED || failCount > MAX_FAILS_BEFORE_RESTART) //Check if we're still connected to the WiFi
    {
      Serial.println("Shutting Down in 9 seconds");
      ESP.restart();
      Serial.println("You should never see this");
    }
    ESP.wdtFeed();
    unsigned long currentMillis = millis();
    if(currentMillis >= offAt && isOn)
    {
        isOn = false;
        digitalWrite(PIN, OFF_VALUE);
        setScreenState(offCommand);
    }
    // Check if a client has connected
    WiFiClient client = server.available();
    if (!client) {
        return;
    }

    // Wait until the client sends some data
    Serial.println("new client");
    while(!client.available()){
        delay(1);
    }

    // Read the first line of the request
    String req = client.readStringUntil('\r');
    Serial.println(req);
    client.flush();
    String success = "success";
    std::string command;
    // Match the request
    bool retry = true;
    while(retry)
    {
        int val;
        if (req.indexOf("/on") != -1)
        {
            offAt = currentMillis + AUTO_OFF_DELAY;
            if(isOn)
            {
                success = "error";
                val = ON_VALUE;
            }
            else
            {
                isOn = true;
                val = ON_VALUE;
                command = (std::string)onCommand;
            }
        }
        else if (req.indexOf("/off") != -1)
        {
            if(!isOn)
            {
                success = "error";
                val = OFF_VALUE;
            }
            else
            {
                isOn = false;
                val = OFF_VALUE;
                command = offCommand;
            }
        }
        else {
            Serial.println("invalid request");
            success = "error";
        }

        if(success == "success")
        {
            // Set GPIO2 according to the request
            digitalWrite(PIN, val);
            bool success = setScreenState(command);
            retry = !success;
            failCount += success
        }
    }

    client.flush();

    String stringValue = val == ON_VALUE? "on" : "off";
    // Prepare the response
    String s = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"request\":\"";
    //ing s =    "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nGPIO is now ";
    s += success;
    s += "\",\"status\":\"";
    s += stringValue;
    s += "\"}";

    // Send the response to the client
    client.print(s);
    delay(1);
    Serial.println("Client disonnected");

    // The client will actually be disconnected 
    // when the function returns and 'client' object is detroyed
}

