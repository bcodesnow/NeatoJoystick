#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "../TCPSender/tcpsender.h"
#include "joysticktransmitter.h"
#include <QObject>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    JoystickTransmitter jt;
    TCPSender tcs(nullptr);
    jt.set_ref_to_tcpSender(&tcs);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    engine.rootContext()->setContextProperty("joystickTransmitter", &jt);
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
