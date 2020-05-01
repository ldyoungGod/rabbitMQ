#ifndef CRABBITMQ_H
#define CRABBITMQ_H
#include <string>
#include <thread>
#include "rabitmq/SimpleAmqpClient/SimpleAmqpClient.h"

typedef long long llong;

struct RabbitMQ_INFO
{
    std::string m_mq_host;
    int         m_mq_port;
    std::string m_mq_username;
    std::string m_mq_passwd;
    std::string m_mq_vhost;

    std::string exchange;
    std::string queue;
    std::string routingkey;
    std::string type;
};

class CRabbitMQ
{
public:
    explicit CRabbitMQ(const RabbitMQ_INFO& baseInfo);
public:
    bool pub_msg(const std::string strMsg);
    bool start_recv_msg(llong timeOutMsec = 0);
    void stop_recv_msg();

private:
    bool init_rabbit_mq();
    bool exit_rabbit_mq();
    void recv_work_thread();

private:
    RabbitMQ_INFO               m_basicInfo;
    llong                       m_timeOutMsec;
    llong                       m_startTime;//mq start time

    bool                        m_bwork;
    std::thread*                m_work_thread;

    std::string                 m_str_consumer;
    AmqpClient::Channel::ptr_t  m_channel;

};

#endif // CRABBITMQ_H
