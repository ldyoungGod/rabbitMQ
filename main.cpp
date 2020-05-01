#include <iostream>
#include <CRabbitMQ.h>
using namespace std;

int main(int argc, char *argv[])
{
    RabbitMQ_INFO info;
    info.exchange        = "baseInfo.exchange";
    info.m_mq_host       = "192.168.2.139";
    info.m_mq_passwd     = "admin";
    info.m_mq_port       = 5672;
    info.m_mq_username   = "admin";
    info.m_mq_vhost      = "/";
    info.queue           = "baseInfo.queue";
    info.routingkey      = "baseInfo.routingkey";
    info.type            = "direct";// DIRECT("direct"), FANOUT("fanout"), TOPIC("topic"), HEADERS("headers");
    CRabbitMQ mqClient(info);

    for(int i = 0; i< 10000; i++)
    {
        char sz[20] = {0};
        sprintf(sz,"%d:test",i+1);
        mqClient.pub_msg(sz);
    }
    mqClient.start_recv_msg(1000 * 5);

    char c;
    scanf("%c",&c);
    return 0;
}
