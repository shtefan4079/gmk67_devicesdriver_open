#ifndef PROTOCOLCONTROLLER_H
#define PROTOCOLCONTROLLER_H

#include "HidManager.h"
#include <QColor>
#include <QMap>
#include <QObject>
#include <QVariantMap>

class ProtocolController : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionChanged)

public:
  explicit ProtocolController(QObject *parent = nullptr);
  ~ProtocolController();

  bool isConnected() const;

  Q_INVOKABLE void applyLighting(int mode, const QVariantMap &colors);
  Q_INVOKABLE void applyKeymap(const QVariantMap &mapping);
  Q_INVOKABLE QVariantMap getScancodes() const;
  Q_INVOKABLE void connectToDevice();

signals:
  void connectionChanged();

private:
  HidManager *hid;
  QList<QByteArray> lightTemplate;
  QMap<QString, int> scancodes;

  void initTemplates();
  void initScancodes();
  QByteArray parseHexString(const QString &hex);
};

#endif // PROTOCOLCONTROLLER_H
