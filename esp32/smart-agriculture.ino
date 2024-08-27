/** @file smart-agriculture.ino
 *
 *  @repository https://codeberg.org/firefly-cpp/t-call-esp32-data-collection
 *  @bug No known bugs.
 *
 */

#define TINY_GSM_MODEM_SIM800

#define SerialMon Serial
#define SerialAT Serial1

#define TINY_GSM_RX_BUFFER 128  // Increased buffer size
#define TINY_GSM_DEBUG SerialMon
#define MODEM_RST            5
#define MODEM_PWKEY          23
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26

// Sim card data | check with your provider
const char apn[]  = "";
const char gprsUser[] = "";
const char gprsPass[] = "";

// Your URL or IP of your web server / cloud
const char server[] = "YOUR_URL_OR_IP";

// Setup port
const int port = 8080;

// Include essential libraries
// You can download them directly from Arduino IDE
#include <TinyGsmClient.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <Wire.h>
#include <BH1750.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

TinyGsmClient client(modem);

// Define the DHT sensor type and the pin
#define DHTPIN 14  // check your wiring!
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// Initialize the BH1750 sensor
BH1750 lightMeter;

// Define the moisture sensor pin
#define MOISTURE_PIN 34

unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 10L * 1000L; // Increased interval to 10 seconds

// Define a struct to store sensor data
struct SensorData {
    float temperature;
    float humidity;
    float lightLevel;
    int moisture;
};

SensorData sensorData;

void setup() {
    SerialMon.begin(115200);
    delay(10);

    pinMode(MODEM_PWKEY, OUTPUT);
    pinMode(MODEM_RST, OUTPUT);
    pinMode(MODEM_POWER_ON, OUTPUT);
    digitalWrite(MODEM_PWKEY, LOW);
    digitalWrite(MODEM_RST, HIGH);
    digitalWrite(MODEM_POWER_ON, HIGH);

    SerialMon.println("Wait...");

    // Set baud rate for GSM module
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
    SerialMon.println("Initializing modem...");
    modem.init();

    connectGPRS();

    // Init the DHT sensor
    SerialMon.println("Initializing DHT sensor...");
    dht.begin();

    // Init the BH1750 sensor
    SerialMon.println("Initializing BH1750 sensor...");
    Wire.begin();
    if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
        SerialMon.println(F("BH1750 initialized"));
    } else {
        SerialMon.println(F("Error initializing BH1750"));
    }

    // Initialize the moisture sensor pin
    pinMode(MOISTURE_PIN, INPUT);
}

void loop() {
    // Check if it's time to update
    if (millis() - lastUpdateTime >= updateInterval) {
        lastUpdateTime = millis();
        if (!modem.isNetworkConnected()) {
            SerialMon.println("Network disconnected, attempting to reconnect...");
            reconnectGPRS();
        }
        if (modem.isNetworkConnected()) {
            readSensors();
            httpPostRequest();
        } else {
            SerialMon.println("Failed to reconnect to the network!");
        }
    }
}

void readSensors() {
    SerialMon.println("Reading sensors...");

    delay(1000);

    // Read temperature and humidity | DHT22 sensor
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Read light level
    float lightLevel = lightMeter.readLightLevel();

    // Print light level for debugging
    //SerialMon.print("Light: ");
    //SerialMon.println(lightLevel);

    // Read moisture level
    int moisture = analogRead(MOISTURE_PIN);

    // Print moisture level
    //SerialMon.print("Moisture: ");
    //SerialMon.println(moisture);

    // Check if any reads failed and exit early (to try again).
    if (isnan(temperature) || isnan(humidity) || isnan(lightLevel)) {
        SerialMon.println("Failed to read from sensors!");
        return;
    }

    // Store the sensor values in global variables
    sensorData.temperature = temperature;
    sensorData.humidity = humidity;
    sensorData.lightLevel = lightLevel;
    sensorData.moisture = moisture;

    // Check free heap memory for debugging
    SerialMon.print("Free heap memory: ");
    SerialMon.println(ESP.getFreeHeap());
}

void httpPostRequest() {
    // Make the HTTP POST request
    SerialMon.println("Performing HTTP POST request...");

    if (client.connect(server, port)) {
        // Create the JSON data string using the stored sensor values
        char jsonData[192]; // Ensure the array is large enough to hold the entire JSON string
        snprintf(jsonData, sizeof(jsonData),
                 "{\"temperature\":%.2f,\"humidity\":%.2f,\"moisture\":%d,\"light\":%.2f,\"password\":\"%s\"}",
                 sensorData.temperature, sensorData.humidity,
                 sensorData.moisture, sensorData.lightLevel, "passpass");

        client.println("POST /measure HTTP/1.1");
        client.print("Host: ");
        client.println(server);
        client.println("User-Agent: TTGO-TCALL/1.0");
        client.println("Content-Type: application/json");
        client.print("Content-Length: ");
        client.println(strlen(jsonData));
        client.println();
        client.println(jsonData);

        uint32_t timeout = millis();

        while (client.connected() && millis() - timeout < 10000L) {
            // Print available data
            while (client.available()) {
                char c = client.read();
                SerialMon.print(c);
                timeout = millis(); // Reset timeout if data is available
            }
        }
    } else {
        SerialMon.println("Connection failed");
        reconnectGPRS();  // Attempt to reconnect if the client connection fails
    }

    client.stop();
    SerialMon.println("Server Disconnected");
}

void connectGPRS() {
    SerialMon.print(F("Connecting to "));
    SerialMon.print(apn);
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        SerialMon.println(" FAIL!");
        delay(10000);
        return;
    }
    SerialMon.println(" SUCCESS!");

    if (modem.isGprsConnected()) {
        SerialMon.println("GPRS connected");
    }
}

void reconnectGPRS() {
    int retryCount = 0;
    const int maxRetries = 5;  // Set a reasonable number of retries

    while (retryCount < maxRetries && !modem.isGprsConnected()) {
        SerialMon.print("Attempting to reconnect to GPRS (attempt ");
        SerialMon.print(retryCount + 1);
        SerialMon.println(")...");

        modem.gprsDisconnect();  // Disconnect any previous connection
        delay(1000);  // Small delay before reconnecting

        modem.init();  // Reinitialize the modem
        connectGPRS();

        if (modem.isGprsConnected()) {
            SerialMon.println("Reconnected to network successfully.");
            break;
        } else {
            SerialMon.println("Reconnect attempt failed.");
            retryCount++;
            delay(5000);  // Wait before retrying
        }
    }

    if (!modem.isGprsConnected()) {
        SerialMon.println("Failed to reconnect after multiple attempts. Restarting ESP32...");
        ESP.restart();  // Reset the ESP32
    }
}
