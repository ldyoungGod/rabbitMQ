#include "CRabbitMQ.h"
#include <string.h>
#include <exception>
#include <sys/timeb.h>
CRabbitMQ::CRabbitMQ(const RabbitMQ_INFO &baseInfo):m_channelRecv(nullptr)
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

CRabbitMQ::~CRabbitMQ()
{
    this->stop_recv_msg();
}

bool CRabbitMQ::pub_msg(const std::string strMsg)
{
    try
    {
        bool bSuc = this->init_rabbit_mq(PUB);
        if (bSuc == false)
        {
            return false;
        }

        AmqpClient::BasicMessage::ptr_t msgPtr = AmqpClient::BasicMessage::Create(strMsg);
        this->m_channelPub->BasicPublish(
                   m_basicInfo.exchange,
                   m_basicInfo.routingkey,
                   msgPtr);
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

    bool bSuc = this->init_rabbit_mq(RECV);
    if (bSuc == false)
    {
        printf("init_rabbit_mq_error!\n");
        return false;
    }

    this->m_bwork = true;
    this->m_work_thread = new std::thread(&CRabbitMQ::recv_work_thread, this);
    if (this->m_work_thread == NULL)
    {
        this->m_bwork = false;
        printf("init_rabbit_mq_error!\n");
        return false;
    }
    this->m_work_thread->detach();

    struct timeb tb;
    ftime(&tb);
    m_startTime = tb.time * 1000 + tb.millitm;

    printf("init_rabbit_mq_OK!\n");
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

bool CRabbitMQ::init_rabbit_mq(WORK_TYPE flag)
{
    try
    {
        if(RECV == flag)
        {
            if(nullptr == this->m_channelRecv)
            {
                this->m_channelRecv = AmqpClient::Channel::Create(m_basicInfo.m_mq_host
                                                              ,m_basicInfo.m_mq_port
                                                              ,m_basicInfo.m_mq_username
                                                              ,m_basicInfo.m_mq_passwd
                                                              ,m_basicInfo.m_mq_vhost);
                if (this->m_channelRecv)
                {
                    this->m_channelRecv->DeclareExchange(m_basicInfo.exchange, m_basicInfo.type, false, true, false);
                    std::string queue_name =  this->m_channelRecv->DeclareQueue(m_basicInfo.queue, false, true, false, false);
                    this->m_channelRecv->BindQueue(queue_name, m_basicInfo.exchange, m_basicInfo.routingkey);
                    this->m_str_consumer = this->m_channelRecv->BasicConsume(queue_name, "", true,  false, false, 1);
                }
            }
        }
        else if(PUB == flag)
        {
            if(nullptr == this->m_channelPub)
            {
                this->m_channelPub = AmqpClient::Channel::Create(m_basicInfo.m_mq_host
                                                              ,m_basicInfo.m_mq_port
                                                              ,m_basicInfo.m_mq_username
                                                              ,m_basicInfo.m_mq_passwd
                                                              ,m_basicInfo.m_mq_vhost);
                if (this->m_channelPub)
                {
                    this->m_channelPub->DeclareExchange(m_basicInfo.exchange, m_basicInfo.type, false, true, false);
                    std::string queue_name =  this->m_channelPub->DeclareQueue(m_basicInfo.queue, false, true, false, false);
                    this->m_channelPub->BindQueue(queue_name, m_basicInfo.exchange, m_basicInfo.routingkey);
                }
            }
        }
    }
    catch(std::exception& ex)
    {
        printf("init rabbit error!\n");
        printf(ex.what());
        printf("\n");
        return false;
    }

    return true;
}

bool CRabbitMQ::exit_rabbit_mq()
{
    if(this->m_channelRecv != NULL  && this->m_str_consumer.length() > 0)
    {
        this->m_channelRecv->BasicCancel(this->m_str_consumer);
        this->m_channelRecv = NULL;
        this->m_str_consumer.clear();
    }
    return true;
}

void CRabbitMQ::recv_work_thread()
{
    while(this->m_bwork)
    {
        struct timeb tb;
        ftime(&tb);
        llong nowMsec = tb.time * 1000 + tb.millitm;
        if (this->m_bwork == false)
        {
            break;
        }

        if(m_timeOutMsec > 0)
        {
            if(nowMsec - m_startTime > m_timeOutMsec)
            {
                printf("----rabbit_client_recv TIME_OUT ----\n");
                this->m_bwork = false;
                break;
            }
        }

        AmqpClient::Envelope::ptr_t envelope;
        if (this->m_channelRecv->BasicConsumeMessage(this->m_str_consumer, envelope, 50) == false)
        {
            continue;
        }

        std::string strRoutingKey = envelope->RoutingKey();
        std::string buffer   = envelope->Message()->Body();

        this->m_channelRecv->BasicAck(envelope);
        if(buffer.length() > 0)
        {
            printf("nowMsec:(%lld)--buffer recv:%s\n",nowMsec,buffer.c_str());
        }
    }
}
