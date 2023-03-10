#ifndef FLASHER_H
#define FLASHER_H

#include <QMainWindow>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QSerialPort>
#include <QSerialPortInfo>

#include <QStandardPaths>
#include <QDir>
#include <QProcess>


QT_BEGIN_NAMESPACE
namespace Ui { class Flasher; }
QT_END_NAMESPACE

class Flasher : public QMainWindow
{
    Q_OBJECT

public:
    void extracted();
    Flasher(QWidget *parent = nullptr);
    ~Flasher();

private:
    Ui::Flasher *ui;
    QNetworkAccessManager manager;
    QHash<QString, QSerialPortInfo> serialPortsHash;
    QSerialPort serialPort;
    QProcess process;

private slots:
    void downloadFirmware();
    void sslErrors(const QList<QSslError> &sslErrors);
    void firmwareDownloaded(QNetworkReply *reply);
    void monitorSerial();
    void readSerial();
    void serialError(QSerialPort::SerialPortError error);
    void flashSerial();
};
#endif // FLASHER_H
