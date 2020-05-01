#include <iostream>
#include <CRabbitMQ.h>
using namespace std;

int main(int argc, char *argv[])
{
    RabbitMQ_INFO info;
    info.exchange        = "baseInfo.exchange";
    info.m_mq_host       = "192.168.2.139";
    info.m_mq_passwd     = "guest";
    info.m_mq_port       = 5672;
    info.m_mq_username   = "guest";
    info.m_mq_vhost      = "/";
    info.queue           = "baseInfo.queue";
    info.routingkey      = "baseInfo.routingkey";
    info.type            = "direct";// DIRECT("direct"), FANOUT("fanout"), TOPIC("topic"), HEADERS("headers");
    CRabbitMQ mqClient(info);

    mqClient.start_recv_msg();

    for(int i = 0; i< 100000; i++)
    {
        char sz[20] = {0};
        sprintf(sz,"%d:test",i+1);
        if(!mqClient.pub_msg(sz))
        {
            printf("publish message failed!\n");
        }
        else
        {
            printf("publish message (%d) succeed!\n",i+1);
        }
    }

    char c;
    scanf("%c",&c);
    return 0;
}
