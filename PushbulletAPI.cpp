#include "PushbulletAPI.hpp"

/**
 * @brief Construct a new PushbulletAPI object
 * 
 * @param t_accessToken : Pushbullet API access token.
 * @param t_fingerprint : Pushbullet SSL fingerprint for verification of secure connection.
 * @param t_verifiedConnectionOnly : Use only verifite secure connection, when false - when fingerprint is not equal, note will be sent.
 */
PushbulletAPI::PushbulletAPI(const String t_accessToken):
m_accessToken(t_accessToken)
{
    this->m_fingerprint = "";
    this->m_verifiedConnectionOnly = false;
    this->m_secure_client = new WiFiClientSecure;
    this->m_connectionLastState = 2;
}

PushbulletAPI::PushbulletAPI(const String t_accessToken, const String t_fingerprint, bool t_verifiedConnectionOnly):
m_accessToken(t_accessToken),
m_fingerprint(t_fingerprint),
m_verifiedConnectionOnly(t_verifiedConnectionOnly)
{
    this->m_secure_client = new WiFiClientSecure;
    this->m_connectionLastState = 0;
}

/**
 * @brief trying to connected to pushbullet server for 5 times.
 * 
 * @return true : connection establish.
 * @return false : could not connect to pushbullet server.
 */
bool PushbulletAPI::getConnection()
{
    if(connectionStatus()) return true;

    bool connected = false;

    // try to establish connection for 5 times.
    for (size_t i = 0; i < 5; i++)
    {
        if(m_secure_client->connect(PUSHBULLET_HOST, PUSHBULLET_PORT))
        {
            connected = true;
            break;
        }
        else
        {
            serialPrint("Pushbullet server connection failed. Retrying...", 3);
        }
    }

    if (!connected)
    {
        serialPrint("Could not connect to Pushbullet server.", 3);
    }

    return connected;
}

/**
 * @brief open connection to Pushbullet API server
 * 
 * @return true : connection establish.
 * @return false : could not connect to pushbullet server.
 */
bool PushbulletAPI::openConnection()
{
    // set fingerprint
    this->m_secure_client->setFingerprint(m_fingerprint.c_str());

    // if last known state is insecure connection, continue in insecure.
    if(this->m_connectionLastState == 2)
    {
        serialPrint("Last known connection - Insecure, we continue to work in Insecure mode.", 1);
        openInsecureConnection();
    }
    // if unknown or secure, go secure of if enabled by user switch to insecure.
    else
    {
        if(m_verifiedConnectionOnly)
        {
            if(getConnection()) m_connectionLastState = 1;
        }
        else
        {
            if(getConnection()) m_connectionLastState = 1;
            else openInsecureConnection();
        }
    }
    
    return connectionStatus();
}

/**
 * @brief open insecure connection to Pushbullet API server when secure connection is not available.
 * 
 * @return true : connection establish.
 * @return false : could not connect to pushbullet server.
 */
bool PushbulletAPI::openInsecureConnection()
{
    if(this->m_verifiedConnectionOnly) return false;

    this->m_secure_client->setInsecure();

    // try to establish connection for 5 times.
    serialPrint("Trying to connect without fingerprint verification.", 2);

    if(getConnection())
    {
        serialPrint("Conenction established without fingerprint verification.", 2);
        serialPrint("Please update the fingerprint for secure connection.", 2);
        this->m_connectionLastState = 2;
        return true;
    }
    else
    {
        serialPrint("Could not establish connection to Pushbullet server, please check internet conenction.", 3);
        this->m_verifiedConnectionOnly = 0;
        return false;
    }
    
}

/**
 * @brief close connection, used after create http request
 * 
 */
void PushbulletAPI::closeConnection()
{
    serialPrint("Connection closed", 1); // Debug mode
    this->m_secure_client->stop();
}

/**
 * @brief return the connection status to pushbullet api.
 * 
 * @return true : connected
 * @return false : disconnected
 */
bool PushbulletAPI::connectionStatus()
{
    if(this->m_secure_client->connected() == 1)
    {
        serialPrint("Connection status : true", 1); // Debug mode
		return true;
	}
	else
    {
        serialPrint("Connection status : false", 1); // Debug mode
		return false;
	}
}

/**
 * @brief create new HTTP Post Request format.
 * 
 * @param t_url : the url to the request
 * @param t_body : body of the request
 */
String PushbulletAPI::createHttpPostRequest(String t_host, String t_url, String t_accessToken ,String t_messagebody)
{
    String request = String("POST ") + t_url + " HTTP/1.1\r\n" +
                        "Host: " + t_host + "\r\n" +
                        "Authorization: Bearer " + t_accessToken + "\r\n" +
                        "Content-Type: application/json\r\n" +
                        "Content-Length: " + t_messagebody.length() + "\r\n\r\n" +
                        t_messagebody;
    return request;
}

/**
 * @brief send a pushbullet http request to api.
 * 
 * @param t_messagebody : message body for pushbullet http post request. 
 */
void PushbulletAPI::sendPushbulletHttpRequest(String t_messagebody)
{
    String post_request = createHttpPostRequest(this->PUSHBULLET_HOST,
                                                this->PUSHBULLET_PUSH_URL, 
                                                this->m_accessToken, t_messagebody);
    // Debug mode
    serialPrint("Pusbullet HTTP request:", 1);
    serialPrint(post_request, 1);

    this->m_secure_client->print(post_request);

    serialPrint("Push message sent.", 0);
}


/**
 * @brief push regular/note notifaction message to pushbullet client.
 * 
 * @param t_title : message title
 * @param t_body  : message body
 */
void PushbulletAPI::pushNotifcation(String t_title, String t_body)
{
    // check if connection is availble.
    if( connectionStatus() == false )
    {
        if(openConnection() == false)
        return;
    }

    String messagebody = {"{\"body\":\"" + t_body + "\",\"title\":\"" + t_title + "\",\"type\":\"note\"}"};
    sendPushbulletHttpRequest(messagebody);
    debugResponse();
    closeConnection();
}

/**
 * @brief push note type with url notifaction message to pushbullet client.
 * 
 * @param t_title : message title
 * @param t_body  : message body
 */
void PushbulletAPI::pushNotifcationUrl(String t_title, String t_body, String t_url)
{
    // check if connection is availble.
    if( connectionStatus() == false )
    {
        if(openConnection() == false)
        return;
    }


    String messagebody = {"{\"body\":\"" + t_body + "\",\"title\":\"" + t_title + "\",\"url\":\"" + t_url + "\",\"type\":\"link\"}"};
    sendPushbulletHttpRequest(messagebody);
    debugResponse();
    closeConnection();
}

/**
 * @brief push regular/note notifaction message to pushbullet client.
 *
 * @param t_title : message title
 * @param t_body  : message body
 * @param t_email : receiver email
 */
void PushbulletAPI::pushNotifcationEmail(String t_title, String t_body, String t_email)
{
    // check if connection is availble.
    if( connectionStatus() == false )
    {
        if(openConnection() == false)
        return;
    }

    String messagebody = {"{\"body\":\"" + t_body + "\",\"email\":\"" + t_email + "\",\"title\":\"" + t_title + "\",\"type\":\"note\"}"};
    sendPushbulletHttpRequest(messagebody);
    debugResponse();
    closeConnection();
}

/**
 * @brief if DEBUG is enabled, will print the server response.
 * 
 */
void PushbulletAPI::debugResponse()
{
    #ifdef DEBUG_PUSHBULLETAPI
    while (m_secure_client->available() == 0);

    while (m_secure_client->available())
    {
        String line = m_secure_client->readStringUntil('\n');
        serialPrint(line, 1);
    }
    # endif // DEBUG_PUSHBULLETAPI
}


/**
 * @brief PushbulletAPI Serial Print function, for easy debug & Usefull information.
 * default is set to info. 
 * 
 * @param t_message : message to print.
 * @param t_msgType : 0 - Info, 1 - Debug, 2 - Warning, 3 - Error
 */
void PushbulletAPI::serialPrint(const String t_message, const int t_msgType)
{   
    switch (t_msgType)
    {
    case 1: // Debug
        #ifdef DEBUG_PUSHBULLETAPI
            Serial.println("[DEBUG] PushbulletAPI : " + t_message);
        #endif // DEBUG_PUSHBULLETAPI
        break;
    case 2: // Warning
        Serial.println(" [WARNING] PushbulletAPI : " + t_message);
        break;
    case 3: // Error
        Serial.println("[ERROR] PushbulletAPI : " + t_message);
        break;
    default:
        Serial.println("[INFO] PushbulletAPI : " + t_message);
        break;
    }
}
