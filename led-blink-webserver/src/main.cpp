// Load Wi-Fi library
#include <WiFi.h>

const char *ssid = "szymon";
const char *password = "ezzm7395";
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String GREEN_LED_STATE = "off";

const int PIN_GREEN_LED = 25;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

void connectWiFi()
{
    // Connect to Wi-Fi
    Serial.print("Trying to connect ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print("Retrying ...\n");
    }

    return;
}

void startWebserver()
{
    Serial.println("WiFi connection estabilished");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    server.begin();

    return;
}

void setup()
{
    Serial.begin(115200);

    pinMode(PIN_GREEN_LED, OUTPUT);

    digitalWrite(PIN_GREEN_LED, LOW);

    connectWiFi();
    startWebserver();
}


void solveLEDs()
{
    if (header.indexOf("GET /25/on") >= 0)
    {   
        Serial.println("GPIO 25 on");
        GREEN_LED_STATE = "on";
        digitalWrite(PIN_GREEN_LED, HIGH);
    }
    else if (header.indexOf("GET /25/off") >= 0)
    {
        Serial.println("GPIO 25 off");
        GREEN_LED_STATE = "off";
        digitalWrite(PIN_GREEN_LED, LOW);
    }
    
    return;
}

void displayWebpage(WiFiClient client)
{
    // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
    // and a content-type so the client knows what's coming, then a blank line:
    String webpage = \
    "HTTP/1.1 200 OK\n\
    Content-type:text/html\n\
    Connection: close\n";

    client.println(webpage);
    webpage = "";

    // Create webpage
    webpage += \
    "<!DOCTYPE html><html>\n\
    <head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n\
    <link rel=\"icon\" href=\"data:,\">\n";

    // webpage style
    webpage += \
    "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center; color: white;}\n\
    .button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;\n\
    text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}\n\
    .buttonOff {background-color: #F2300D;}\n\
    body {background-image: url('https://kononopedia.pl/images/9/93/Zbigniew_Stonoga.jpeg');}</style></head>\n";

    // webpage title
    webpage += "<body><h1>ESP32 light switcher</h1>\n";

    webpage += "<p>Green LED " + GREEN_LED_STATE + "</p>\n";
    if (GREEN_LED_STATE == "off")
    {
        webpage += "<p><a href=\"/25/on\"><button class=\"button\">ON</button></a></p>\n";
    }
    else
    {
        webpage += "<p><a href=\"/25/off\"><button class=\"button buttonOff\">OFF</button></a></p>\n";
    }

    webpage += "</body></html>\n";

    client.println(webpage);
    return;
}

void loop()
{
    WiFiClient client = server.available(); // Listening

    if (client)
    {
        currentTime = millis();
        previousTime = currentTime;
        Serial.println("connected ...");
        String currentLine = "";

        while (client.connected() && currentTime - previousTime <= timeoutTime)
        {
            currentTime = millis();
            if (client.available())
            {
                // read bytes and print on serial monitor
                char c = client.read();
                Serial.write(c);
                header += c;
                if (c == '\n')
                {
                    // two newline characters = end of HTTP request => send webpage as response
                    if (currentLine.length() == 0)
                    {
                        solveLEDs();
                        displayWebpage(client);
                        break;
                    }
                    else
                    {
                        currentLine = "";
                    }
                }
                else if (c != '\r') // /r = carriage return
                {
                    currentLine += c;
                }
            }
        }
        header = "";

        client.stop();
        Serial.println("disconnecting ...\n");
    }
}
