#include "mymqproducer.h"

MyMqProducer::MyMqProducer()
{

}

MyMqProducer::~MyMqProducer()
{

    if (bytesMessage != nullptr) {
        delete bytesMessage;
        bytesMessage = nullptr;
    }


    if (textMessage != nullptr) {
        delete textMessage;
        textMessage = nullptr;
    }

//    cleanup();
}

bool MyMqProducer::start( const std::string& brokerURI, unsigned int numMessages, const std::string& destURI, bool useTopic = false, bool clientAck = false )
{
    this->connection = NULL;
    this->session = NULL;
    this->destination = NULL;
    this->messageproducer = NULL;
    this->numMessages = numMessages;
    this->useTopic = useTopic;
    this->brokerURI = brokerURI;
    this->destURI = destURI;
    this->clientAck = clientAck;

    return initialize();
}

bool MyMqProducer::initialize()
{
    try {
        // Create a ConnectionFactory
        ActiveMQConnectionFactory* connectionFactory = new ActiveMQConnectionFactory("tcp://127.0.0.1:61616?transport.useAsyncSend=true&maxReconnectDelay=10000");
        //connection = connectionFactory->createConnection("userName","pwd");

        connection = connectionFactory->createConnection();

        connection->start();

        // Create a Session
        if( clientAck ) {
            session = connection->createSession( Session::CLIENT_ACKNOWLEDGE );
        } else {
            session = connection->createSession( Session::AUTO_ACKNOWLEDGE );
        }

        // Create the destination (Topic or Queue)
        if( useTopic ) {
            destination = session->createTopic( destURI );
        } else {
            destination = session->createQueue( destURI );
        }
        // Create a MessageProducer from the Session to the Topic or Queue
        messageproducer = session->createProducer( destination );
        messageproducer->setDeliveryMode( DeliveryMode::NON_PERSISTENT );

        return true;
    }catch ( CMSException& e ) {
        qDebug()<<"createConnection faill!";
        emit producer_connectfail();
        e.printStackTrace();
        return false;

    }
}

void MyMqProducer::send(const char* Message,int nSize)
{
    // 创建一个byte类型的消息
    bytesMessage = session->createBytesMessage((unsigned char*)Message,nSize);
    // 发送消息
    messageproducer->send(bytesMessage );
    delete bytesMessage;
    bytesMessage = nullptr;
    //cleanup();
}

void MyMqProducer::sendTxtMsg(const std::string& textMsg)
{
    // 消息内容
    // 创建一个文本类型的消息
    TextMessage* message = session->createTextMessage();
    message->setStringProperty("language","c++");
    //message->setText(UnicodeToUTF8(ANSIToUnicode(textMsg)));
    message->setText(textMsg);
    messageproducer->send(message);
    delete message;
    textMessage = nullptr;
    //cleanup();
}
void MyMqProducer::onException( const CMSException& ex AMQCPP_UNUSED )
{
    qDebug()<<"CMS Exception occurred.  Shutting down client.\n";
    //exit(1);
}

void MyMqProducer::transportInterrupted()
{
    //std::cout << "The Connection's Transport has been Interrupted." << std::endl;
    qDebug()<<"The Connection's Transport has been Interrupted.";
}

void MyMqProducer::transportResumed()
{
   // std::cout << "The Connection's Transport has been Restored." << std::endl;
    qDebug()<<"The Connection's Transport has been Restored.";
}

void MyMqProducer::cleanup()
{
    // Destroy resources.
    try{
        if( destination != NULL ) delete destination;
    }catch ( CMSException& e ) { e.printStackTrace(); }
    destination = NULL;

    try{
        if( messageproducer != NULL ) delete messageproducer;
    }catch ( CMSException& e ) { e.printStackTrace(); }
    messageproducer = NULL;

    // Close open resources.
    try{
        if( session != NULL ) session->close();
        if( connection != NULL ) connection->close();
    }catch ( CMSException& e ) { e.printStackTrace(); }

    try{
        if( session != NULL ) delete session;
    }catch ( CMSException& e ) { e.printStackTrace(); }
    session = NULL;

    try{
        if( connection != NULL ) delete connection;
    }catch ( CMSException& e ) { e.printStackTrace(); }
    connection = NULL;
}

