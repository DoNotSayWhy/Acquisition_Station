#include "mymqconsumer.h"

MyMqConsumer::MyMqConsumer()
{

}
MyMqConsumer::~MyMqConsumer()
{
//    cleanup();

}

void MyMqConsumer::close()
{
    cleanup();
}
//brokerURI ="failover:(tcp://localhost:61616)";
bool MyMqConsumer::start(const std::string& brokerURI, const std::string& destURI, bool useTopic, bool clientAck )
{
    this->connection = NULL;
    this->session = NULL;
    this->destination = NULL;
    this->consumer = NULL;

    this->useTopic = useTopic;
    this->brokerURI = brokerURI;
    this->destURI = destURI;
    this->clientAck = clientAck;

    return runConsumer();
}

bool MyMqConsumer::runConsumer()
{
    //qDebug()<<"ConsumerThread:"<<QThread::currentThread();
    try {

        // Create a ConnectionFactory
        //ActiveMQConnectionFactory* connectionFactory =  new ActiveMQConnectionFactory( brokerURI );
        QString serveruri = QString::fromStdString(brokerURI) + "?transport.useAsyncSend=true&maxReconnectDelay=10000";
        ActiveMQConnectionFactory* connectionFactory = new ActiveMQConnectionFactory(serveruri.toStdString());
        //connection = connectionFactory->createConnection("userName","pwd");
        connection = connectionFactory->createConnection();

        //delete connectionFactory;
//        ActiveMQConnection* amqConnection = dynamic_cast<ActiveMQConnection*>( connection );
//        if( amqConnection != NULL ) {
//            amqConnection->addTransportListener( this );
//        }
        connection->start();
        connection->setExceptionListener(this);

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

        // Create a MessageConsumer from the Session to the Topic or Queue
        consumer = session->createConsumer( destination );
        consumer->setMessageListener( this );
        return true;
    } catch (CMSException& e) {
        qDebug()<<"SimpleAsyncConsumer createConnection faill!";
        e.printStackTrace();
        return false;
    }
}

// Called from the consumer since this class is a registered MessageListener.
void MyMqConsumer::onMessage( const Message* message )
{

    static int count = 0;
    try{
        count++;
        const TextMessage* txtMessage =
            dynamic_cast< const TextMessage* >( message );
        string text;

        if( txtMessage != NULL ) {
            text = txtMessage->getText();
        } else {
            text = "NOT A BYTE SMESSAGE!";
        }

        if( clientAck ) {
            message->acknowledge();
        }

        //qDebug()<<"ConsumerThread:"<<QThread::currentThread();
        //qDebug()<<"收到字符串:"<<text.c_str();
        emit comsumerreceiver(QString::fromStdString(text.c_str()));

    } catch (CMSException& e) {
        qDebug()<<"SimpleAsyncConsumer 接受消息失败!";
        e.printStackTrace();
    }
}

void MyMqConsumer::onException( const CMSException& ex AMQCPP_UNUSED )
{
    qDebug()<<"CMS Exception occurred.  Shutting down client.\n";
    //exit(1);
}

void MyMqConsumer::transportInterrupted()
{
    qDebug()<< "The Connection's Transport has been Interrupted.";
}

void MyMqConsumer::transportResumed()
{
    qDebug()<< "The Connection's Transport has been Restored.";
}



void MyMqConsumer::cleanup(){

        //*************************************************
        // Always close destination, consumers and producers before
        // you destroy their sessions and connection.
        //*************************************************

        // Destroy resources.
        try{
            if( destination != NULL ) delete destination;
        }catch (CMSException& e) {
        }
        destination = NULL;

        try{
            if( consumer != NULL ) delete consumer;
        }catch (CMSException& e) {
        }
        consumer = NULL;

        // Close open resources.
        try{
            if( session != NULL ) session->close();
            if( connection != NULL ) connection->close();
        }catch (CMSException& e) {
        }

        // Now Destroy them
        try{
            if( session != NULL ) delete session;
        }catch (CMSException& e) {
        }
        session = NULL;

        try{
            if( connection != NULL ) delete connection;
        }catch (CMSException& e) {
        }
        connection = NULL;
}
