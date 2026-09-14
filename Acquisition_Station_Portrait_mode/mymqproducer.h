#ifndef MYMQPRODUCER_H
#define MYMQPRODUCER_H

#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/util/Config.h>
#include <activemq/library/ActiveMQCPP.h>
#include <activemq/transport/DefaultTransportListener.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include<QString>
#include<QDebug>
using namespace activemq;
using namespace activemq::core;
using namespace decaf;
using namespace decaf::lang;
using namespace decaf::util;
using namespace decaf::util::concurrent;
using namespace cms;
using namespace std;


class MyMqProducer: public QObject
{
    Q_OBJECT
public:
    MyMqProducer();
    virtual ~MyMqProducer();
    bool start( const std::string& brokerURI, unsigned int numMessages, const std::string& destURI, bool useTopic , bool clientAck);
    void send(const char* bytesMessage,int nSize);
    void sendTxtMsg(const std::string& textMsg);

    void close();
    virtual void onException( const CMSException& ex AMQCPP_UNUSED );
    virtual void transportInterrupted();
    virtual void transportResumed();

private:
    void cleanup();
    virtual bool initialize();

signals:
    void producer_connectfail();
private:
    Connection* connection;
    Session* session;
    Destination* destination;
    MessageProducer* messageproducer;
    bool useTopic;
    bool clientAck;
    unsigned int numMessages;
    std::string brokerURI;
    std::string destURI;
    BytesMessage* bytesMessage = nullptr;
    TextMessage* textMessage = nullptr;

};

#endif // MYMQPRODUCER_H
