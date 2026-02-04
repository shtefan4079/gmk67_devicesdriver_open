#include "ProtocolController.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>

ProtocolController::ProtocolController(QObject *parent) : QObject(parent) {
  hid = new HidManager(this);
  initTemplates();
  initScancodes();
  connectToDevice();
}

ProtocolController::~ProtocolController() {}

bool ProtocolController::isConnected() const { return hid->isConnected(); }

void ProtocolController::connectToDevice() {
  bool wasConnected = isConnected();
  if (hid->connectDevice() != wasConnected) {
    emit connectionChanged();
  }
}

QVariantMap ProtocolController::getScancodes() const {
  QVariantMap map;
  for (auto it = scancodes.begin(); it != scancodes.end(); ++it) {
    map.insert(it.key(), it.value());
  }
  return map;
}

QByteArray ProtocolController::parseHexString(const QString &hex) {
  QByteArray data;
  QStringList parts = hex.split(':');
  for (const QString &part : parts) {
    bool ok;
    data.append(static_cast<char>(part.toInt(&ok, 16)));
  }
  return data;
}

void ProtocolController::applyLighting(int mode, const QVariantMap &colors) {
  connectToDevice();

  qDebug() << "Incoming Color Map Size:" << colors.size();

  QMap<int, QColor> colorMap;
  for (auto it = colors.begin(); it != colors.end(); ++it) {
    QColor c;
    if (it.value().canConvert<QColor>()) {
      c = it.value().value<QColor>();
    } else {
      c = QColor(it.value().toString());
    }

    if (!c.isValid()) {
      qDebug() << "Invalid color for key" << it.key() << "Value:" << it.value();
      c = QColor(Qt::magenta);
    }
    colorMap.insert(it.key().toInt(), c);
  }

  QColor targetColor(Qt::black);

  if (colorMap.size() > 60) {
    bool allSame = true;
    QColor first = colorMap.first();
    for (const QColor &c : colorMap) {
      if (c != first) {
        allSame = false;
        break;
      }
    }
    if (allSame) {
      targetColor = first;
      qDebug() << "Detected Full-Board Uniform Color, setting global target to:"
               << targetColor.name();
    }
  }

  qDebug() << "Target Global Color (Background):" << targetColor.name();

  for (int i = 0; i < lightTemplate.size(); ++i) {
    QByteArray b = lightTemplate[i];

    if ((unsigned char)b[0] == 0x04 && (unsigned char)b[1] == 0x13) {
      b[8] = static_cast<char>(mode);
    }

    if ((unsigned char)b[0] == 0x80) {
      b[8] = static_cast<char>(mode);
      b[1] = targetColor.red();
      b[2] = targetColor.green();
      b[3] = targetColor.blue();
    }

    int j = 0;
    int size = b.size();
    while (j <= size - 4) {
      unsigned char idx = (unsigned char)b[j];
      if (colorMap.contains(idx)) {
        QColor c = colorMap[idx];
        b[j + 1] = c.red();
        b[j + 2] = c.green();
        b[j + 3] = c.blue();
        j += 4;
      } else if ((unsigned char)b[j + 1] == 0xFF &&
                 (unsigned char)b[j + 2] == 0x00 &&
                 (unsigned char)b[j + 3] == 0x00) {
        b[j + 1] = targetColor.red();
        b[j + 2] = targetColor.green();
        b[j + 3] = targetColor.blue();
        j += 4;
      } else {
        j += 1;
      }
    }

    hid->sendFeatureReport(b);
    if (i == 10)
      QThread::msleep(50);
  }
}

void ProtocolController::applyKeymap(const QVariantMap &mapping) {
  connectToDevice();

  QList<int> blocks = {0x11, 0x27};
  for (int block : blocks) {
    hid->sendFeatureReport(parseHexString(
        "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:"
        "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:"
        "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"));

    QByteArray b1;
    b1.resize(2);
    b1[0] = 0x04;
    b1[1] = 0x18;
    hid->sendFeatureReport(b1);

    QByteArray b2;
    b2.resize(9);
    b2[0] = 0x04;
    b2[1] = static_cast<char>(block);
    b2[8] = 0x09;
    hid->sendFeatureReport(b2);

    QByteArray matrix;
    matrix.resize(576);
    matrix.fill(0);

    if (block == 0x11) {
      for (auto it = mapping.begin(); it != mapping.end(); ++it) {
        int idx = it.key().toInt();
        QString name = it.value().toString();
        if (scancodes.contains(name)) {
          if (idx * 4 + 2 < 576) {
            matrix[idx * 4] = 0x02;
            matrix[idx * 4 + 2] = static_cast<char>(scancodes[name]);
          }
        }
      }
    }

    for (int i = 0; i < 576; i += 64) {
      hid->sendFeatureReport(matrix.mid(i, 64));
    }

    QByteArray end1;
    end1.resize(64);
    end1.fill(0);
    end1[62] = 0xaa;
    end1[63] = 0x55;
    hid->sendFeatureReport(end1);

    QByteArray end2;
    end2.resize(2);
    end2[0] = 0x04;
    end2[1] = 0x02;
    hid->sendFeatureReport(end2);
    QByteArray end3;
    end3.resize(2);
    end3[0] = 0x04;
    end3[1] = 0xf0;
    hid->sendFeatureReport(end3);
  }
}

void ProtocolController::initTemplates() {
  QStringList raw = {
      "04:18:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:"
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:"
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
      "04:23:00:00:00:00:00:00:09:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:"
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:"
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
      "00:00:00:00:01:ff:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:"
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:"
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:14:ff:00:00:15:ff:00:00:"
      "16:ff:00:00:17:ff:00:00:18:ff:00:00:19:ff:00:00:1a:ff:00:00:1b:ff:00:00:"
      "1c:ff:00:00:1d:ff:00:00:1e:ff:00:00:1f:ff:00:00",
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:25:ff:00:00:"
      "26:ff:00:00:27:ff:00:00:28:ff:00:00:29:ff:00:00:2a:ff:00:00:2b:ff:00:00:"
      "2c:ff:00:00:2d:ff:00:00:2e:ff:00:00:2f:ff:00:00",
      "30:ff:00:00:31:ff:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:"
      "00:00:00:00:37:ff:00:00:38:ff:00:00:39:ff:00:00:3a:ff:00:00:3b:ff:00:00:"
      "3c:ff:00:00:3d:ff:00:00:3e:ff:00:00:3f:ff:00:00",
      "40:ff:00:00:41:ff:00:00:42:ff:00:00:43:ff:00:00:00:00:00:00:00:00:00:00:"
      "00:00:00:00:00:00:00:00:00:00:00:00:49:ff:00:00:4a:ff:00:00:4b:ff:00:00:"
      "4c:ff:00:00:4d:ff:00:00:4e:ff:00:00:4f:ff:00:00",
      "50:ff:00:00:51:ff:00:00:52:ff:00:00:53:ff:00:00:54:ff:00:00:55:ff:00:00:"
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:5b:ff:00:00:"
      "5c:ff:00:00:5d:ff:00:00:5e:ff:00:00:5f:ff:00:00",
      "60:ff:00:00:00:00:00:00:00:00:00:00:63:ff:00:00:64:ff:00:00:65:ff:00:00:"
      "66:ff:00:00:67:ff:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:"
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:"
      "76:ff:00:00:77:ff:00:00:00:00:00:00:79:ff:00:00:00:00:00:00:00:00:00:00:"
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:"
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:"
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:aa:55",
      "04:02:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:"
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:"
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
      "04:f0:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:"
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:"
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
      "04:18:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:"
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:"
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
      "04:13:00:00:00:00:00:00:01:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:"
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:"
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
      "80:ff:00:00:00:00:00:00:01:0f:0f:00:00:00:aa:55:00:00:00:00:00:00:00:00:"
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:"
      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"};
  for (const QString &s : raw) {
    lightTemplate.append(parseHexString(s));
  }
}

void ProtocolController::initScancodes() {
  scancodes["A"] = 0x04;
  scancodes["B"] = 0x05;
  scancodes["C"] = 0x06;
  scancodes["D"] = 0x07;
  scancodes["E"] = 0x08;
  scancodes["F"] = 0x09;
  scancodes["G"] = 0x0A;
  scancodes["H"] = 0x0B;
  scancodes["I"] = 0x0C;
  scancodes["J"] = 0x0D;
  scancodes["K"] = 0x0E;
  scancodes["L"] = 0x0F;
  scancodes["M"] = 0x10;
  scancodes["N"] = 0x11;
  scancodes["O"] = 0x12;
  scancodes["P"] = 0x13;
  scancodes["Q"] = 0x14;
  scancodes["R"] = 0x15;
  scancodes["S"] = 0x16;
  scancodes["T"] = 0x17;
  scancodes["U"] = 0x18;
  scancodes["V"] = 0x19;
  scancodes["W"] = 0x1A;
  scancodes["X"] = 0x1B;
  scancodes["Y"] = 0x1C;
  scancodes["Z"] = 0x1D;
  scancodes["1"] = 0x1E;
  scancodes["2"] = 0x1F;
  scancodes["3"] = 0x20;
  scancodes["4"] = 0x21;
  scancodes["5"] = 0x22;
  scancodes["6"] = 0x23;
  scancodes["7"] = 0x24;
  scancodes["8"] = 0x25;
  scancodes["9"] = 0x26;
  scancodes["0"] = 0x27;
  scancodes["ENTER"] = 0x28;
  scancodes["ESC"] = 0x29;
  scancodes["BACKSPACE"] = 0x2A;
  scancodes["TAB"] = 0x2B;
  scancodes["SPACE"] = 0x2C;
  scancodes["MINUS"] = 0x2D;
  scancodes["EQUAL"] = 0x2E;
  scancodes["LBRACKET"] = 0x2F;
  scancodes["RBRACKET"] = 0x30;
  scancodes["BACKSLASH"] = 0x31;
  scancodes["SEMICOLON"] = 0x33;
  scancodes["QUOTE"] = 0x34;
  scancodes["TILDE"] = 0x35;
  scancodes["COMMA"] = 0x36;
  scancodes["PERIOD"] = 0x37;
  scancodes["SLASH"] = 0x38;
  scancodes["CAPS_LOCK"] = 0x39;
  scancodes["F1"] = 0x3A;
  scancodes["F2"] = 0x3B;
  scancodes["F3"] = 0x3C;
  scancodes["F4"] = 0x3D;
  scancodes["F5"] = 0x3E;
  scancodes["F6"] = 0x3F;
  scancodes["F7"] = 0x40;
  scancodes["F8"] = 0x41;
  scancodes["F9"] = 0x42;
  scancodes["F10"] = 0x43;
  scancodes["F11"] = 0x44;
  scancodes["F12"] = 0x45;
  scancodes["PRINTSCREEN"] = 0x46;
  scancodes["SCROLLLOCK"] = 0x47;
  scancodes["PAUSE"] = 0x48;
  scancodes["INSERT"] = 0x49;
  scancodes["HOME"] = 0x4A;
  scancodes["PAGE_UP"] = 0x4B;
  scancodes["DELETE"] = 0x4C;
  scancodes["END"] = 0x4D;
  scancodes["PAGE_DOWN"] = 0x4E;
  scancodes["RIGHT"] = 0x4F;
  scancodes["LEFT"] = 0x50;
  scancodes["DOWN"] = 0x51;
  scancodes["UP"] = 0x52;
  scancodes["MUTE"] = 0x7F;
  scancodes["VOL_UP"] = 0x80;
  scancodes["VOL_DOWN"] = 0x81;
  scancodes["POWER"] = 0x66;
  scancodes["SLEEP"] = 0x74;
  scancodes["WAKE"] = 0x75;
}
