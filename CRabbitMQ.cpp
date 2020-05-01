#include "CRabbitMQ.h"
#include <string.h>
#include <exception>
#include <sys/timeb.h>
CRabbitMQ::CRabbitMQ(const RabbitMQ_INFO &baseInfo):m_channel(nullptr)
{
    m_basicInfo.exchange        = baseInfo.exchange;
    m_basicInfo.m_mq_host       = baseInfo.m_mq_host;
    m_basicInfo.m_mq_passwd     = baseInfo.m_mq_passwd;
    m_basicInfo.m_mq_port       = baseInfo.m_mq_port;
    m_basicInfo.m_mq_username   = baseInfo.m_mq_username;
    m_basicInfo.m_mq_vhost      = baseInfo.m_mq_vhost;
    m_basicInfo.queue           = baseInfo.queue;
    m_basicInfo.routingkey      = baseInfo.routingkey;
    m_basicInfo.type            = baseInfo.type;
}

bool CRabbitMQ::pub_msg(const std::string strMsg)
{
    try
    {
        bool bSuc = this->init_rabbit_mq();
        if (bSuc == false)
        {
            return false;
        }
        this->m_channel->BasicPublish(
                   m_basicInfo.exchange,
                   m_basicInfo.routingkey,
                   AmqpClient::BasicMessage::Create(strMsg));
    }
    catch(std::exception& e)
    {
        printf("publish message error!(%s)\n",e.what());
        return false;
    }
    return true;
}

bool CRabbitMQ::start_recv_msg(llong timeOutMsec)
{
    m_timeOutMsec = timeOutMsec;

    bool bSuc = this->init_rabbit_mq();
    if (bSuc == false)
    {
        return false;
    }

    this->m_bwork = true;
    this->m_work_thread = new std::thread(&CRabbitMQ::recv_work_thread, this);
    if (this->m_work_thread == NULL)
    {
        this->m_bwork = false;
        return false;
    }
    this->m_work_thread->detach();

    struct timeb t1;
    ftime(&t1);
    m_startTime = t1.time * 1000 + t1.millitm;

    return true;
}

void CRabbitMQ::stop_recv_msg()
{
    if(this->m_bwork)
    {
        this->m_bwork = false;
        usleep(100 * 1000);
    }
    this->exit_rabbit_mq();

    if(this->m_work_thread && this->m_work_thread->joinable())
    {
        this->m_work_thread->join();
        delete this->m_work_thread;
        this->m_work_thread = NULL;
    }
}

bool CRabbitMQ::init_rabbit_mq()
{
    if(nullptr != this->m_channel)
    {
        return true;
    }
    try
    {
        this->m_channel = AmqpClient::Channel::Create(m_basicInfo.m_mq_host
                                                      ,m_basicInfo.m_mq_port
                                                      ,m_basicInfo.m_mq_username
                                                      ,m_basicInfo.m_mq_passwd
                                                      ,m_basicInfo.m_mq_vhost);
        if (this->m_channel == nullptr)
        {
            return false;
        }

        this->m_channel->DeclareExchange(m_basicInfo.exchange, m_basicInfo.type, false, true, false);
        std::string queue_name =  this->m_channel->DeclareQueue(m_basicInfo.queue, false, true, false, false);
        this->m_channel->BindQueue(queue_name, m_basicInfo.exchange, m_basicInfo.routingkey);
        this->m_str_consumer = this->m_channel->BasicConsume(queue_name, "", true,  false, false, 1);
    }
    catch(std::exception &e)
    {
        printf("init rabbit error!\n");
        printf(e.what());
        printf("\n");
        return false;
    }
    return true;
}

bool CRabbitMQ::exit_rabbit_mq()
{
    if(this->m_channel != NULL  && this->m_str_consumer.length() > 0)
    {
        this->m_channel->BasicCancel(this->m_str_consumer);
        this->m_channel = NULL;
        this->m_str_consumer.clear();
    }
    return true;
}

void CRabbitMQ::recv_work_thread()
{
    while(this->m_bwork)
    {
        if (this->m_bwork == false)
        {
            break;
        }

        if(m_timeOutMsec > 0)
        {
            struct timeb tb;
            ftime(&tb);
            llong nowSec = tb.time * 1000 + tb.millitm;
            printf("--%ld\n",nowSec);
            if(nowSec - m_startTime > m_timeOutMsec)
            {
                printf("----rabbit_client_recv TIME_OUT ----\n");
                this->m_bwork = false;
                break;
            }
        }

        AmqpClient::Envelope::ptr_t envelope;
        if (this->m_channel->BasicConsumeMessage(this->m_str_consumer, envelope, 100) == false)
        {
            continue;
        }

        std::string strRoutingKey = envelope->RoutingKey();
        std::string buffer   = envelope->Message()->Body();

        this->m_channel->BasicAck(envelope);
        printf("%s\n",buffer.c_str());
    }
}
