#include "HidManager.h"
#include <QThread>

HidManager::HidManager(QObject *parent) : QObject(parent) { hid_init(); }

HidManager::~HidManager() {
  closeDevice();
  hid_exit();
}

bool HidManager::connectDevice() {
  if (isConnected())
    return true;

  struct hid_device_info *devs, *cur_dev;
  devs = hid_enumerate(VID, PID);
  cur_dev = devs;

  while (cur_dev) {
    if (cur_dev->interface_number == 0) {
      handle = hid_open_path(cur_dev->path);
      if (handle) {
        qDebug() << "Connected to device at path:" << cur_dev->path;
        break;
      }
    }
    cur_dev = cur_dev->next;
  }
  hid_free_enumeration(devs);

  return isConnected();
}

void HidManager::closeDevice() {
  if (handle) {
    hid_close(handle);
    handle = nullptr;
  }
}

bool HidManager::isConnected() const { return handle != nullptr; }

void HidManager::sendFeatureReport(const QByteArray &data) {
  if (!handle && !connectDevice()) {
    qWarning() << "Device not connected, cannot send report";
    return;
  }

  unsigned char buffer[65];
  buffer[0] = 0;
  memset(buffer + 1, 0, 64);

  int len = qMin(data.size(), 64);
  memcpy(buffer + 1, data.constData(), len);

  int res = hid_send_feature_report(handle, buffer, 65);
  if (res < 0) {
    qWarning() << "Failed to send feature report:" << hid_error(handle);
    closeDevice();
  }

  QThread::msleep(5);

  unsigned char buf[65];
  buf[0] = 0;
  hid_get_feature_report(handle, buf, 65);
}
