#include "Backend/ProtocolController.h"
#include <QDebug>       // Added for qDebug and qCritical
#include <QDirIterator> // Added for QDirIterator
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QtQml>

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);

  QQuickStyle::setStyle("Material");

  qmlRegisterType<ProtocolController>("gmk67_ui.backend", 1, 0,
                                      "ProtocolController");

  QQmlApplicationEngine engine;
  const QUrl url(QStringLiteral("qrc:/gmk67_ui/src/UI/Main.qml"));

  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreated, &app,
      [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
          QCoreApplication::exit(-1);
      },
      Qt::QueuedConnection);

  engine.load(url);

  return app.exec();
}
