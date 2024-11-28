// importar as bibliotecacas necessarias
#define ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <Stepper.h>

const int stepsPerRevolution = 2048;  // o numero de passos por volta

#define IN1 19
#define IN2 18
#define IN3 5
#define IN4 17

// inicializar a biblioteca do motor de passo
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

// coloque as credenciais da sua conexão de internet
const char* ssid = "iPhone de Rafael";
const char* password = "25015626";

const char* PARAM_INPUT_1 = "state";
const char* PARAM_INPUT_2 = "value";

const int output = 2;

bool motorActive = false;  // para saber se o motor esta ativo ou não

// Crie o objeto AsyncWebServer na porta 80
AsyncWebServer server(80);
// pagina HTML
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<head>  
  <meta name="viewport" content="width=device-width, initial-scale=1">  
  <title>ESP Web Server</title>  
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}     
    h2 {font-size: 2.4rem;}     
    p {font-size: 2.2rem;}     
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}     
    .switch {position: relative; display: inline-block; width: 120px; height: 68px}      
    .switch input {display: none}
  </style> 
</head>  
<body>   
  <h2>Alimentador de Pet</h2>   
  

  <!-- Botão para iniciar o motor -->
  <p><button onclick="startMotor()">Iniciar Motor</button></p>
  
  <script> 
    function startMotor() {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/startMotor", true);  // Envia uma solicitação GET para ativar o motor
      xhr.send();  // Envia a requisição
    }
    
  </script> 
</body>
</html>

)rawliteral";

// Substitui o espaço reservado pela seção de botão na sua página da web
String processor(const String& var){
  if(var == ""){
    String buttons = "";
    String outputStateValue = outputState();
    buttons+= "<p><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"output\" ";
    return buttons;
  }
  return String();
}

String outputState(){
  if(digitalRead(output)){
    return "checked";
  }
  else {
    return "";
  }
}

void setup(){
  // definir a velocidade
  myStepper.setSpeed(5);
  // inicialização da serial para o esp32
  Serial.begin(115200);

  pinMode(output, OUTPUT);
  digitalWrite(output, LOW);

  // conectar no Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // mostrar o endereço IP
  Serial.println(WiFi.localIP());

  // rota para levar para o servidor web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Envie uma solicitação GET para a atualização do esp
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      digitalWrite(output, inputMessage.toInt());
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });
  
  

  server.on("/startMotor", HTTP_GET, [] (AsyncWebServerRequest *request) {
    motorActive = true;  // deixa o motor ativo
    Serial.println("Starting stepper motor...");
    request->send(200, "text/plain", "Motor Started");
  });

  // inicia o server
  server.begin();
}

void loop() {
  // se o motor ativar, ira girar o que foi definido
  if (motorActive) {
    Serial.println("Motor stepping...");
    myStepper.step(380);  // gira o motor uma volta inteira(pode ser modificado se quiser)
    delay(1000);  // espera 1 segundo antes de parar ou continuar
    myStepper.step(-380);
    delay(1000); // o mesmo só que inverso
    motorActive = false; //desliga o motor
  }
}
