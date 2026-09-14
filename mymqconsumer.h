#ifndef MYMQCONSUMER_H
#define MYMQCONSUMER_H
#include <activemq/core/ActiveMQConnectionFactory.h>
//#include<activemq/core/ActiveMQConnection.h>
#include <activemq/util/Config.h>
#include <activemq/library/ActiveMQCPP.h>
#include <activemq/transport/DefaultTransportListener.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include<QDebug>
#include<QThread>
#include<QString>
#include<QObject>
using namespace activemq;
using namespace activemq::core;
using namespace activemq::transport;
using namespace cms;
using namespace std;

class MyMqConsumer : public QObject,public ExceptionListener, public MessageListener, public DefaultTransportListener
{
    Q_OBJECT
public:
    MyMqConsumer();
    virtual ~MyMqConsumer();
    bool start(const std::string& brokerURI, const std::string& destURI, bool useTopic, bool clientAck );
    void close();
    bool runConsumer();
    virtual void onMessage( const Message* message );
    virtual void onException( const CMSException& ex AMQCPP_UNUSED );
    virtual void transportInterrupted();
    virtual void transportResumed();
signals:
    void comsumerreceiver(QString str);
    void comsumer_connectfail();
private:
    void cleanup();

private:
    Connection* connection;
    Session* session;
    Destination* destination;
    MessageConsumer* consumer;
    bool useTopic;
    bool clientAck;
    std::string brokerURI;
    std::string destURI;
};

#endif // MYMQCONSUMER_H
