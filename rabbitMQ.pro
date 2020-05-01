TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += $$PWD/rabitmq/
DEPENDPATH += $$PWD/libs/

LIBS += -lpthread

LIBS += -L$$PWD/libs/ -lSimpleAmqpClient
LIBS += -L$$PWD/libs/ -lrabbitmq




SOURCES += main.cpp \
    CRabbitMQ.cpp

HEADERS += \
    CRabbitMQ.h \
    rabitmq/SimpleAmqpClient/AmqpException.h \
    rabitmq/SimpleAmqpClient/AmqpLibraryException.h \
    rabitmq/SimpleAmqpClient/AmqpResponseLibraryException.h \
    rabitmq/SimpleAmqpClient/BadUriException.h \
    rabitmq/SimpleAmqpClient/BasicMessage.h \
    rabitmq/SimpleAmqpClient/Channel.h \
    rabitmq/SimpleAmqpClient/ConnectionClosedException.h \
    rabitmq/SimpleAmqpClient/ConsumerCancelledException.h \
    rabitmq/SimpleAmqpClient/ConsumerTagNotFoundException.h \
    rabitmq/SimpleAmqpClient/Envelope.h \
    rabitmq/SimpleAmqpClient/MessageReturnedException.h \
    rabitmq/SimpleAmqpClient/SimpleAmqpClient.h \
    rabitmq/SimpleAmqpClient/Table.h \
    rabitmq/SimpleAmqpClient/Util.h \
    rabitmq/SimpleAmqpClient/Version.h
