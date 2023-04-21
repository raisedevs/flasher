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
    Flasher(QWidget *parent = nullptr);
    ~Flasher();

private:
    Ui::Flasher *ui;
    QDir cacheDir;
    QFile firmwareFile;
    QHash<QString, QSerialPortInfo> serialPortsHash;
    QNetworkAccessManager manager;
    QSerialPort serialPort;
    QProcess process;
    QString esptoolPath;

    void setOutput(QString message);
    void appendOutput(QString message);
    void appendOutputLine(QString message);
    void setEspToolPath();

private slots:
    void reloadSerialPorts();
    void updateUrl();
    void readMacAddress();
    void readMacAddressFinished();
    void downloadFirmware();
    void sslErrors(const QList<QSslError> &sslErrors);
    void firmwareDownloaded(QNetworkReply *reply);
    void monitorSerial();
    void readSerial();
    void readProcess();
    void serialError(QSerialPort::SerialPortError error);
    void flashSerial();
    void flashFinished();
};
#endif // FLASHER_H
