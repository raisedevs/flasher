#include "flasher.h"
#include "./ui_flasher.h"

const char* FLASHER_USER_AGENT = "Raise-Flasher-Qt";
const char* RAISE_DEV_CONSOLE_DOMAIN = "https://console.raise.dev";

Flasher::Flasher(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Flasher)
{
    ui->setupUi(this);

    QFont monospaceFont("SF Mono");
    monospaceFont.setStyleHint(QFont::Monospace);
    ui->outputEdit->setFont(monospaceFont);

    cacheDir.setPath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    firmwareFile.setFileName(cacheDir.filePath("firmware/firmware.bin"));

    reloadSerialPorts();

    connect(ui->reloadSerialPortsButton, &QPushButton::clicked,
            this, &Flasher::reloadSerialPorts);

    connect(ui->macAddressEdit, &QLineEdit::textEdited,
            this, &Flasher::updateUrl);

    connect(ui->readMacAddressButton, &QPushButton::clicked,
            this, &Flasher::readMacAddress);

    connect(ui->workspaceEdit, &QLineEdit::textEdited,
            this, &Flasher::updateUrl);

    connect(ui->downloadButton, &QPushButton::clicked,
            this, &Flasher::downloadFirmware);

    connect(ui->monitorButton, &QPushButton::clicked,
            this, &Flasher::monitorSerial);

    connect(ui->flashButton, &QPushButton::clicked,
            &serialPort, &QSerialPort::close);
    connect(ui->flashButton, &QPushButton::clicked,
            this, &Flasher::flashSerial);

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
    ui->readMacAddressButton->setEnabled(false);
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
            ui->readMacAddressButton->setEnabled(true);
            ui->monitorButton->setEnabled(true);
            ui->flashButton->setEnabled(firmwareFile.exists());
        }
    }
}

void Flasher::setEspToolPath() {
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
}

void Flasher::updateUrl()
{
    const QString workspace = ui->workspaceEdit->text();
    const QString mac_address = ui->macAddressEdit->text();
    if (workspace.isEmpty() || mac_address.isEmpty()) {
        ui->updaterUrlEdit->clear();
        ui->downloadButton->setEnabled(false);
        return;
    }

    ui->updaterUrlEdit->setText(
        QString(RAISE_DEV_CONSOLE_DOMAIN) +
        "/workspaces/" +
        workspace +
        "/updater" +
        "?mac_address=" +
        mac_address
    );
    ui->downloadButton->setEnabled(true);
}

void Flasher::readMacAddress()
{
    ui->monitorButton->setEnabled(false);
    ui->flashButton->setEnabled(false);

    if (esptoolPath.isEmpty()) {
        setEspToolPath();
    }
    if (esptoolPath.isEmpty()) {
        ui->monitorButton->setEnabled(true);
        ui->flashButton->setEnabled(true);
        return;
    }

    const QStringList args = {
        "--chip", "esp32",
        "--port", ui->serialPortBox->currentText(),
        "read_mac",
    };
    appendOutputLine(QString("Running %1 %2...\n").arg(esptoolPath, args.join(" ")));
    process.start(esptoolPath, args);
    connect(&process, &QProcess::finished,
            this, &Flasher::readMacAddressFinished);
}

void Flasher::readMacAddressFinished()
{
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        return;
    }

    const QString output = ui->outputEdit->toPlainText();
    const static QRegularExpression macRegex("^MAC: (([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2}))$", QRegularExpression::MultilineOption);
    const QRegularExpressionMatch match = macRegex.match(output);

    if (!match.hasMatch()) {
        return;
    }
    const QString captured = match.captured(1);
    ui->macAddressEdit->setText(captured);
}

void Flasher::downloadFirmware()
{
    ui->monitorButton->setEnabled(false);
    ui->flashButton->setEnabled(false);
    ui->downloadButton->setEnabled(false);
    serialPort.close();

    QUrl firmwareUrl(ui->updaterUrlEdit->text());
    QNetworkRequest firmwareRequest(firmwareUrl);
    firmwareRequest.setHeader(QNetworkRequest::UserAgentHeader, FLASHER_USER_AGENT);

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
    ui->readMacAddressButton->setEnabled(false);
    ui->monitorButton->setEnabled(false);

    if (esptoolPath.isEmpty()) {
        setEspToolPath();
    }
    if (esptoolPath.isEmpty()) {
        ui->readMacAddressButton->setEnabled(true);
        ui->monitorButton->setEnabled(true);
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
