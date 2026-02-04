#ifndef HIDMANAGER_H
#define HIDMANAGER_H

#include <QByteArray>
#include <QDebug>
#include <QObject>
#include <hidapi/hidapi.h>

class HidManager : public QObject {
  Q_OBJECT
public:
  explicit HidManager(QObject *parent = nullptr);
  ~HidManager();

  bool connectDevice();
  void closeDevice();
  bool isConnected() const;
  void sendFeatureReport(const QByteArray &data);

private:
  hid_device *handle = nullptr;
  const unsigned short VID = 0x05AC;
  const unsigned short PID = 0x024F;
};

#endif // HIDMANAGER_H
