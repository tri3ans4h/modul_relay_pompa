#include <SoftwareSerial.h>

SoftwareSerial espSerial(10, 11); // RX, TX
//list command
//command=relay     -> get value relay, HIGH | LOW
//command=volume    -> get volume , in liter / cm3
//command=relayoff  -> set relay LOW
//command=relayon   -> set relay HIGH

//from other source
//command=sensor&value=23         -> read sensor  from remote
#define ASCII_0 48
#define ESP8266 espSerial
#define RELAY 4
#define DST_IP "192.168.43.69"
String SSID = "TestAP";
String PASSWORD = "12345678";
int LED = 13;

boolean FAIL_8266 = false;

String strHTML = "";

#define BUFFER_SIZE 128
char buffer[BUFFER_SIZE];

void setup()
{
    pinMode(LED, OUTPUT);
    pinMode(RELAY, OUTPUT);

    digitalWrite(LED, LOW);
    delay(300);
    digitalWrite(LED, HIGH);
    delay(200);
    digitalWrite(LED, LOW);
    delay(300);
    digitalWrite(LED, HIGH);
    delay(200);
    digitalWrite(LED, LOW);
    delay(300);
    digitalWrite(LED, HIGH);
    delay(200);
    digitalWrite(LED, LOW);

    do
    {
        Serial.begin(9600);
        ESP8266.begin(9600);

        //Wait Serial Monitor to start
        while(!Serial);
        Serial.println("--- Start ---");

        ESP8266.println("AT+RST");
        delay(1000);
        if(ESP8266.find("ready"))
        {
            Serial.println("Module is ready");
            ESP8266.println("AT+CWMODE=1");
            delay(2000);

            //Quit existing AP, for demo
            Serial.println("Quit AP");
            ESP8266.println("AT+CWQAP");
            delay(1000);

            clearESP8266SerialBuffer();
            if(cwJoinAP())
            {
                Serial.println("CWJAP Success");
                FAIL_8266 = false;

                delay(3000);
                clearESP8266SerialBuffer();
                //Get and display my IP
                sendESP8266Cmdln("AT+CIFSR", 1000);
                //Set multi connections
                sendESP8266Cmdln("AT+CIPMUX=1", 1000);
                //Setup web server on port 80
                sendESP8266Cmdln("AT+CIPSERVER=1,80",1000);

                Serial.println("Server setup finish");
            }
            else
            {
                Serial.println("CWJAP Fail");
                delay(500);
                FAIL_8266 = true;
            }
        }
        else
        {
            Serial.println("Module have no response.");
            delay(500);
            FAIL_8266 = true;
        }
    }
    while(FAIL_8266);

    digitalWrite(LED, HIGH);
}


void loop()
{
    int connectionId;


    if(ESP8266.readBytesUntil('\n', buffer, BUFFER_SIZE)>0)
    {
        Serial.println("Something received");
        Serial.println(buffer);
        if(strncmp(buffer, "+IPD,", 5)==0)
        {
            Serial.println("+IPD, found");
            sscanf(buffer+5, "%d", &connectionId);
            Serial.println("connectionId: " + String(connectionId));
            delay(1000);
            clearESP8266SerialBuffer();

            // sendHTTPResponse(connectionId, strHTML);

            String command = getCommand();
            if(command=="relay") //req -> command=relay
            {                         //resp -> command=relay&value=1/0    
                strHTML = "command=relay&value="+RELAY;
                sendResponse(connectionId, strHTML);
            }
            else if(command="relayon")  //req -> command=relayon
            {                           //resp -> command=relayon&value=1/0    
                digitalWrite(RELAY,HIGH);
                strHTML = "command=relayon&value="+RELAY;
                sendResponse(connectionId, strHTML);
            }
            else if(command=="relayoff")//req -> command=relayon
            {                           //resp -> command=relayon&value=1/0    
                digitalWrite(RELAY,LOW);
                strHTML = "command=relayoff&value="+RELAY;
                sendResponse(connectionId, strHTML);
            }
            //Close TCP/UDP
            String cmdCIPCLOSE = "AT+CIPCLOSE=";
            cmdCIPCLOSE += connectionId;
            sendESP8266Cmdln(cmdCIPCLOSE, 1000);
        }
    }
}
//getCommand , temukan perintah +IPD,0,200:command=sensor
int getValue()
{
    bool haveValue = false;
    int valueStartPos = 0;
    char value[20];
    for (int i=0; i<strlen(buffer); i++)
    {
        if (!haveValue) // just get the first occurrence of name
        {
            if (  (buffer[i]=='v') &&
                    (buffer[i+1]=='a') &&
                    (buffer[i+2]=='l') &&
                    (buffer[i+3]=='u')  &&
                    (buffer[i+4]=='e') &&
                    (buffer[i+5]=='='))
            {
                haveCommand = true;
                valueStartPos = i+6;
            }
        }
        if (haveValue)
        {
            int tempPos = 0;
            bool finishedCopying = false;
            for (int i=valueStartPos; i<strlen(buffer); i++)
            {
                if ( (buffer[i]==' ') && !finishedCopying )
                {
                    finishedCopying = true;
                }
                if ( !finishedCopying )
                {
                    value[tempPos] = buffer[i];
                    tempPos++;
                }
            }
            value[tempPos] = 0;

        }


    }
    return value;
}

String getCommand()
{
    bool haveCommand = false;
    int commandStartPos = 0;
    char command[20];
    for (int i=0; i<strlen(buffer); i++)
    {
        if (!haveCommand) // just get the first occurrence of name
        {
            //cek jika ada perintah sensor=?
            if (  (buffer[i]=='c') &&
                    (buffer[i+1]=='o') &&
                    (buffer[i+2]=='m') &&
                    (buffer[i+3]=='m')  &&
                    (buffer[i+4]=='a') &&
                    (buffer[i+5]=='n') &&
                    (buffer[i+6]=='d') &&
                    (buffer[i+7]=='='))
            {
                haveCommand = true;
                commandStartPos = i+7;
            }

        }

        if (haveCommand)
        {
            int tempPos = 0;
            bool finishedCopying = false;
            for (int i=commandStartPos; i<strlen(buffer); i++)
            {
                if ( (buffer[i]==' ') && !finishedCopying )
                {
                    finishedCopying = true;
                }
                if ( !finishedCopying )
                {
                    command[tempPos] = buffer[i];
                    tempPos++;
                }
            }
            command[tempPos] = 0;

        }

    }

    return String(command);
}
void sendResponse(int id, String content)
{
    String response;
    response = content;
    String cmd = "AT+CIPSEND=";
    cmd += id;
    cmd += ",";
    cmd += response.length();
    Serial.println("--- AT+CIPSEND ---");
    sendESP8266Cmdln(cmd, 1000);
    Serial.println("--- data ---");
    sendESP8266Data(response, 1000);
}


boolean waitOKfromESP8266(int timeout)
{
    do
    {
        Serial.println("wait OK...");
        delay(1000);
        if(ESP8266.find("OK"))
        {
            return true;
        }

    }
    while((timeout--)>0);
    return false;
}

boolean cwJoinAP()
{
    String cmd="AT+CWJAP=\"" + SSID + "\",\"" + PASSWORD + "\"";
    ESP8266.println(cmd);
    return waitOKfromESP8266(10);
}

//Send command to ESP8266, assume OK, no error check
//wait some time and display respond
void sendESP8266Cmdln(String cmd, int waitTime)
{
    ESP8266.println(cmd);
    delay(waitTime);
    clearESP8266SerialBuffer();
}

//Basically same as sendESP8266Cmdln()
//But call ESP8266.print() instead of call ESP8266.println()
void sendESP8266Data(String data, int waitTime)
{
    ESP8266.print(data);
    delay(waitTime);
    clearESP8266SerialBuffer();
}

//Clear and display Serial Buffer for ESP8266
void clearESP8266SerialBuffer()
{
    Serial.println("= clearESP8266SerialBuffer() =");
    while (ESP8266.available() > 0)
    {
        char a = ESP8266.read();
        Serial.write(a);
    }
    Serial.println("==============================");
}
