/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

// Load Wi-Fi library
#include <WiFi.h>
// needed classes for BME280 sensor
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define LIGHTSENSORPIN 34 //Valosensori pinnissä 34
byte movementSensorPin = 26;

Adafruit_BME280 bme;

// ESP-tukiaseman ssid ja salasana
const char* ssid     = "JukanAP";
const char* password = "12345678";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

void setup() {
  Serial.begin(115200);
  while(!Serial);
  pinMode(LIGHTSENSORPIN, INPUT);
  pinMode(movementSensorPin, INPUT);

  // checking that we find BME280 sensor
  // needed to modify starting to address 0x76
  if (!bme.begin(0x76)) {
    Serial.println(F("Could not find a valid BME280 sensor, check wiring!"));
    while (1) delay(10);
  }

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.softAP(ssid, password);

  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.softAPIP());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("</head>");
           
           // CSS to style

            // Web Page Heading
            client.println("<body>");
            client.println("<h1 style=\"text-align:center\">ESP32 Mittaukset</h1>");
            
            // Printing values
            // &#8451; is code for proper celsius symbol
            // using printf method to control value decimals
            client.println("<table style=\"margin-left:auto;margin-right:auto;\">");      
            client.println("<tr>");
            client.println("<td>Lämpötila</td>");
            client.printf("<td>%.1f &#8451;</td>", bme.readTemperature());
            client.println("</tr>");
            
            client.println("<tr>");
            client.println("<td>Kosteus</td>");
            client.printf("<td>%.0f %%</td>", bme.readHumidity());            
            client.println("</tr>");
            
            client.println("<tr>");
            client.println("<td>Paine</td>");
            client.printf("<td>%.0f mbar</td>", bme.readPressure()/100.0F);
            client.println("</tr>");
            
            client.println("<tr>");
            client.println("<td>Valoisuus</td>");
            client.printf("<td>%d luxia</td>", analogRead(LIGHTSENSORPIN));
            client.println("</tr>");
            
            client.println("<tr>");
            client.println("<td>Paikalla?</td>");
            client.printf("<td>%s</td>", getMovementState());
            client.println("</tr>");
            client.println("</table>");

            // Button for reload page with short javascript code.
            client.println("<button type=\"button\" onclick=\"window.location.reload();\" style=\"margin:0 auto;display:block;padding:1em 1em 1em 1em; margin-top:2em\">UPDATE</button>");
            
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

// function to report is motion detected (KYLLÄ) or not (EI)
char* getMovementState() {
  byte state = digitalRead(movementSensorPin);
  if(state == 1) return "KYLLÄ";
  else if(state == 0) return "EI";
}
