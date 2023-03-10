#include "flasher.h"
#include "./ui_flasher.h"

QString firmware_url =
    "https://raisedevs.github.io/github_pages_firmware/firmware.bin";
const char *firmware_url_root_ca_certificate =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIEvjCCA6agAwIBAgIQBtjZBNVYQ0b2ii+nVCJ+xDANBgkqhkiG9w0BAQsFADBh\n"
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n"
    "QTAeFw0yMTA0MTQwMDAwMDBaFw0zMTA0MTMyMzU5NTlaME8xCzAJBgNVBAYTAlVT\n"
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxKTAnBgNVBAMTIERpZ2lDZXJ0IFRMUyBS\n"
    "U0EgU0hBMjU2IDIwMjAgQ0ExMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKC\n"
    "AQEAwUuzZUdwvN1PWNvsnO3DZuUfMRNUrUpmRh8sCuxkB+Uu3Ny5CiDt3+PE0J6a\n"
    "qXodgojlEVbbHp9YwlHnLDQNLtKS4VbL8Xlfs7uHyiUDe5pSQWYQYE9XE0nw6Ddn\n"
    "g9/n00tnTCJRpt8OmRDtV1F0JuJ9x8piLhMbfyOIJVNvwTRYAIuE//i+p1hJInuW\n"
    "raKImxW8oHzf6VGo1bDtN+I2tIJLYrVJmuzHZ9bjPvXj1hJeRPG/cUJ9WIQDgLGB\n"
    "Afr5yjK7tI4nhyfFK3TUqNaX3sNk+crOU6JWvHgXjkkDKa77SU+kFbnO8lwZV21r\n"
    "eacroicgE7XQPUDTITAHk+qZ9QIDAQABo4IBgjCCAX4wEgYDVR0TAQH/BAgwBgEB\n"
    "/wIBADAdBgNVHQ4EFgQUt2ui6qiqhIx56rTaD5iyxZV2ufQwHwYDVR0jBBgwFoAU\n"
    "A95QNVbRTLtm8KPiGxvDl7I90VUwDgYDVR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQG\n"
    "CCsGAQUFBwMBBggrBgEFBQcDAjB2BggrBgEFBQcBAQRqMGgwJAYIKwYBBQUHMAGG\n"
    "GGh0dHA6Ly9vY3NwLmRpZ2ljZXJ0LmNvbTBABggrBgEFBQcwAoY0aHR0cDovL2Nh\n"
    "Y2VydHMuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0R2xvYmFsUm9vdENBLmNydDBCBgNV\n"
    "HR8EOzA5MDegNaAzhjFodHRwOi8vY3JsMy5kaWdpY2VydC5jb20vRGlnaUNlcnRH\n"
    "bG9iYWxSb290Q0EuY3JsMD0GA1UdIAQ2MDQwCwYJYIZIAYb9bAIBMAcGBWeBDAEB\n"
    "MAgGBmeBDAECATAIBgZngQwBAgIwCAYGZ4EMAQIDMA0GCSqGSIb3DQEBCwUAA4IB\n"
    "AQCAMs5eC91uWg0Kr+HWhMvAjvqFcO3aXbMM9yt1QP6FCvrzMXi3cEsaiVi6gL3z\n"
    "ax3pfs8LulicWdSQ0/1s/dCYbbdxglvPbQtaCdB73sRD2Cqk3p5BJl+7j5nL3a7h\n"
    "qG+fh/50tx8bIKuxT8b1Z11dmzzp/2n3YWzW2fP9NsarA4h20ksudYbj/NhVfSbC\n"
    "EXffPgK2fPOre3qGNm+499iTcc+G33Mw+nur7SpZyEKEOxEXGlLzyQ4UfaJbcme6\n"
    "ce1XR2bFuAJKZTRei9AqPCCcUZlM51Ke92sRKw2Sfh3oius2FkOH6ipjv3U/697E\n"
    "A7sKPPcw7+uvTPyLNhBzPvOk\n"
    "-----END CERTIFICATE-----\n";

Flasher::Flasher(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Flasher)
{
    ui->setupUi(this);

    // TODO: use URL from Raise login
    ui->firmwareUrlEdit->setText(firmware_url);

    for (const QSerialPortInfo &port : QSerialPortInfo::availablePorts()) {
        QString name = port.systemLocation();
        if (name.contains("Bluetooth-Incoming-Port")) {
            // don't include these
            continue;
        }

        serialPortsHash[name] = port;
        ui->serialPortBox->addItem(name);

        if (ui->serialPortBox->currentIndex() < 0) {
            ui->serialPortBox->setCurrentText(name);
            ui->monitorButton->setEnabled(true);
            ui->flashButton->setEnabled(true);
        }
    }

    connect(ui->downloadButton, &QPushButton::clicked,
            this, &Flasher::downloadFirmware);
    connect(&manager, &QNetworkAccessManager::finished,
            this, &Flasher::firmwareDownloaded);

    connect(ui->monitorButton, &QPushButton::clicked,
            this, &Flasher::monitorSerial);
    connect(ui->flashButton, &QPushButton::clicked,
            this, &Flasher::flashSerial);
}

void Flasher::downloadFirmware()
{
    QUrl firmwareUrl(ui->firmwareUrlEdit->text());
    QNetworkRequest firmwareRequest(firmwareUrl);

    ui->outputEdit->clear();
    ui->outputEdit->appendPlainText(QString("Downloading %1...").arg(firmwareUrl.toString()));

    QNetworkReply *reply = manager.get(firmwareRequest);
    connect(reply, &QNetworkReply::sslErrors,
            this, &Flasher::sslErrors);
}

void Flasher::sslErrors(const QList<QSslError> &sslErrors)
{
    for (const QSslError &error : sslErrors) {
        ui->outputEdit->appendPlainText(QString("SSL Error: %1").arg(error.errorString()));
    }
}

void Flasher::firmwareDownloaded(QNetworkReply *reply) {
    if (reply->error()) {
        ui->outputEdit->appendPlainText(QString("Download failed: %1").arg(reply->errorString()));
    } else {
        ui->outputEdit->appendPlainText(
                    QString("Download succeeded: %1 (status code %2) [%3 bytes]").arg(
                        reply->url().toEncoded().constData(),
                        QString::number(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()),
                        QString::number(reply->bytesAvailable())));
    }

    reply->deleteLater();
}

void Flasher::monitorSerial()
{
    const QString portName = ui->serialPortBox->currentText();
    serialPort.setPort(serialPortsHash[portName]);

    ui->outputEdit->clear();
    ui->outputEdit->appendPlainText(QString("Monitoring %1...").arg(portName));
    ui->outputEdit->appendPlainText(QString());

    connect(&serialPort, &QIODevice::readyRead,
            this, &Flasher::readSerial);
    connect(&serialPort, &QSerialPort::errorOccurred,
            this, &Flasher::serialError);

    serialPort.setBaudRate(115200);
    serialPort.open(QIODeviceBase::ReadOnly);
}

void Flasher::readSerial()
{
    // Insert text without a newline
    const QTextCursor &previousCursor = ui->outputEdit->textCursor();
    ui->outputEdit->moveCursor(QTextCursor::End);
    ui->outputEdit->insertPlainText(serialPort.readAll());
    ui->outputEdit->setTextCursor(previousCursor);
}

void Flasher::serialError(QSerialPort::SerialPortError error)
{
    QSerialPort::SerialPortError portError = serialPort.error();
    if (!error && !portError) {
        return;
    }
    QString message = QString("Serial Port Error: %1").arg(error);
    ui->outputEdit->appendPlainText(message);
    serialPort.clearError();
}

void Flasher::flashSerial()
{
    QString esptoolPath = QStandardPaths::findExecutable("esptool.py");
    QString pipPath;
    if (esptoolPath.isEmpty()) {
        pipPath = QStandardPaths::findExecutable("pip3");
        if (pipPath.isEmpty()) {
            pipPath = QStandardPaths::findExecutable("pip");
        }
        if (pipPath.isEmpty()) {
            ui->outputEdit->appendPlainText("Flash Error: could not find esptool.py or pip3 or pip in PATH!");
            return;
        }
    }

    QDir cache = QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    QDir data = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    const bool cache_created = cache.mkpath("pip");
    if (!cache_created) {
        ui->outputEdit->appendPlainText(QString("Flash Error: could not create %1/pip").arg(cache.path()));
        return;
    }

    const bool data_created = data.mkpath("pip");
    if (!data_created) {
        ui->outputEdit->appendPlainText(QString("Flash Error: could not create %1/pip").arg(data.path()));
        return;
    }

    if (esptoolPath.isEmpty()) {
// TODO: set PYTHONPATH
//        process.start(pipPath, {
//            "--prefer-binary", "--isolated", "--upgrade",
//            "--target", data.filePath("pip"),
//            "--cache-dir", cache.filePath("pip"),
//            "install", "esptool",
//        });
//        process.waitForFinished(-1);
    }
}

Flasher::~Flasher()
{
    delete ui;
}

