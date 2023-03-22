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

    QFont monospaceFont("SF Mono");
    monospaceFont.setStyleHint(QFont::Monospace);
    ui->outputEdit->setFont(monospaceFont);

    // TODO: use URL from Raise login
    ui->firmwareUrlEdit->setText(firmware_url);

    cacheDir.setPath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    firmwareFile.setFileName(cacheDir.filePath("firmware/firmware.bin"));

    reloadSerialPorts();

    connect(ui->downloadButton, &QPushButton::clicked,
            this, &Flasher::downloadFirmware);

    connect(ui->monitorButton, &QPushButton::clicked,
            this, &Flasher::monitorSerial);
    connect(ui->flashButton, &QPushButton::clicked,
            &serialPort, &QSerialPort::close);
    connect(ui->flashButton, &QPushButton::clicked,
            this, &Flasher::flashSerial);
    connect(ui->reloadSerialPortsButton, &QPushButton::clicked,
            this, &Flasher::reloadSerialPorts);

    connect(&manager, &QNetworkAccessManager::finished,
            this, &Flasher::firmwareDownloaded);
    connect(&serialPort, &QIODevice::readyRead,
            this, &Flasher::readSerial);
    connect(&process, &QProcess::readyReadStandardOutput,
            this, &Flasher::readProcess);

    process.setProcessChannelMode(QProcess::MergedChannels);
}

void Flasher::setOutput(QString message)
{
    ui->outputEdit->clear();
    ui->outputEdit->appendPlainText(message);
}

void Flasher::appendOutput(QString message)
{
    // Insert text without a newline
    const QTextCursor &previousCursor = ui->outputEdit->textCursor();
    ui->outputEdit->moveCursor(QTextCursor::End);
    ui->outputEdit->insertPlainText(message);
    ui->outputEdit->setTextCursor(previousCursor);
}

void Flasher::appendOutputLine(QString message)
{
    // Insert text with a newline
    ui->outputEdit->appendPlainText(message);
}

void Flasher::reloadSerialPorts() {
    ui->serialPortBox->clear();
    ui->monitorButton->setEnabled(false);
    ui->flashButton->setEnabled(false);

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
            ui->flashButton->setEnabled(firmwareFile.exists());
        }
    }
}

void Flasher::downloadFirmware()
{
    ui->monitorButton->setEnabled(false);
    ui->flashButton->setEnabled(false);
    ui->downloadButton->setEnabled(false);
    serialPort.close();

    QUrl firmwareUrl(ui->firmwareUrlEdit->text());
    QNetworkRequest firmwareRequest(firmwareUrl);

    setOutput(QString("Downloading %1...").arg(firmwareUrl.toString()));

    QNetworkReply *reply = manager.get(firmwareRequest);
    connect(reply, &QNetworkReply::sslErrors,
            this, &Flasher::sslErrors);
}

void Flasher::sslErrors(const QList<QSslError> &sslErrors)
{
    for (const QSslError &error : sslErrors) {
        appendOutputLine(QString("SSL Error: %1").arg(error.errorString()));
    }
}

void Flasher::firmwareDownloaded(QNetworkReply *reply) {
    if (reply->error()) {
        appendOutputLine(QString("Download failed: %1").arg(reply->errorString()));
    } else {
        cacheDir.mkpath("firmware");
        firmwareFile.open(QIODevice::WriteOnly);
        firmwareFile.write(reply->readAll());
        firmwareFile.close();
        appendOutputLine(QString("Downloaded firmware to %1").arg(firmwareFile.fileName()));
        ui->flashButton->setEnabled(true);
    }

    reloadSerialPorts();
    reply->deleteLater();
}

void Flasher::monitorSerial()
{
    const QString portName = ui->serialPortBox->currentText();
    serialPort.setPort(serialPortsHash[portName]);

    setOutput(QString("Monitoring %1...\n").arg(portName));

    connect(&serialPort, &QSerialPort::errorOccurred,
            this, &Flasher::serialError);

    serialPort.setBaudRate(115200);
    serialPort.open(QIODeviceBase::ReadOnly);
}

void Flasher::readSerial()
{
    appendOutput(serialPort.readAll());
}

void Flasher::readProcess()
{
    appendOutput(process.readAllStandardOutput());
}

void Flasher::serialError(QSerialPort::SerialPortError error)
{
    QSerialPort::SerialPortError portError = serialPort.error();
    if (!error && !portError) {
        return;
    }
    setOutput(QString("Serial Port Error: %1").arg(error));
    serialPort.clearError();
}

void Flasher::flashSerial()
{
    ui->monitorButton->setEnabled(false);

    QDir dataDir = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    const QString esptoolPy = "esptool.py";
    if (esptoolPath.isEmpty()) {
      esptoolPath = QStandardPaths::findExecutable(esptoolPy);
    }
    if (esptoolPath.isEmpty()) {
        const QStringList extraPaths = {
            dataDir.filePath("pip/bin"),
            "/opt/homebrew/bin",
            "/usr/local/bin",
        };
        esptoolPath = QStandardPaths::findExecutable(esptoolPy, extraPaths);
    }

    if (esptoolPath.startsWith(dataDir.filePath("pip"))) {
        qputenv("PYTHONPATH", dataDir.filePath("pip").toUtf8());
    }

    QString pipPath;
    if (esptoolPath.isEmpty()) {
        pipPath = QStandardPaths::findExecutable("pip3");
        if (pipPath.isEmpty()) {
            pipPath = QStandardPaths::findExecutable("pip");
        }
        if (pipPath.isEmpty()) {
            setOutput("Flash Error: could not find esptool.py or pip3 or pip in PATH!");
            return;
        }
    }

    const bool cacheCreated = cacheDir.mkpath("pip");
    if (!cacheCreated) {
        setOutput(QString("Flash Error: could not create %1/pip").arg(cacheDir.path()));
        return;
    }

    const bool dataCreated = dataDir.mkpath("pip");
    if (!dataCreated) {
        setOutput(QString("Flash Error: could not create %1/pip").arg(dataDir.path()));
        return;
    }

    if (esptoolPath.isEmpty()) {
        setOutput("esptool.py not found, installing with pip...\n");
        process.start(pipPath, {
            "install",
            "--prefer-binary", "--isolated", "--upgrade",
            "--target", dataDir.filePath("pip"),
            "--cache-dir", cacheDir.filePath("pip"),
            "esptool",
        });
        esptoolPath = dataDir.filePath("pip/bin/esptool.py");
        connect(&process, &QProcess::finished,
                this, &Flasher::flashSerial);
        return;
    }

    disconnect(&process, &QProcess::finished,
              this, &Flasher::flashSerial);

    const QStringList args = {
        "--chip", "esp32",
        "--port", ui->serialPortBox->currentText(),
        "write_flash",
        "0x10000",
        firmwareFile.fileName(),
    };
    appendOutputLine(QString("Running %1 %2...\n").arg(esptoolPath, args.join(" ")));
    process.start(esptoolPath, args);
    connect(&process, &QProcess::finished,
            this, &Flasher::flashFinished);
}

void Flasher::flashFinished() {
    ui->monitorButton->setEnabled(true);

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        return;
    }

    ui->flashButton->setEnabled(false);
    monitorSerial();
}

Flasher::~Flasher()
{
    delete ui;
}

