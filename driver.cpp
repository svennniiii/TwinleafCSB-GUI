#include "driver.h"

#include <QSerialPortInfo>
#include <QSerialPort>
#include <QThread>
#include <QRegularExpression>
#include <cmath>

Q_LOGGING_CATEGORY(twinleafCsbLog, "twinleaf.csb")

TwinleafCSB::TwinleafCSB(QObject *parent)
    : QObject(parent)
    , m_serialPort(new QSerialPort(this))
    , m_channels({"x", "y", "z"})
    , m_xChannel(new CsbChannel(this, CsbChannel::Channel::X))
    , m_yChannel(new CsbChannel(this, CsbChannel::Channel::Y))
    , m_zChannel(new CsbChannel(this, CsbChannel::Channel::Z))
{
}

TwinleafCSB::~TwinleafCSB()
{
    saveSettings();
}

QStringList TwinleafCSB::getDeviceList() const
{
    QStringList devices;
    for (const QSerialPortInfo &serialPortInfo : QSerialPortInfo::availablePorts()) {
        if (!serialPortInfo.hasVendorIdentifier() || !serialPortInfo.hasProductIdentifier()) {
            continue;
        }

        QString deviceString = generateDeviceString(serialPortInfo);
        if (!deviceString.isEmpty()) {
            devices.append(deviceString);
        }
    }

    return devices;
}

CsbChannel *TwinleafCSB::getChannel(const QString &channelName) const
{
    if (channelName == "x") {
        return m_xChannel;
    } else if (channelName == "y") {
        return m_yChannel;
    } else if (channelName == "z") {
        return m_zChannel;
    } else {
        return nullptr;
    }
}

bool TwinleafCSB::isConnected() const
{
    return m_serialPort->isOpen() && m_serialPort->isWritable();
}

void TwinleafCSB::loadSettings(bool loadDevice)
{
    QSettings settings("_config.ini", QSettings::IniFormat);

    if (loadDevice) {
        connectDevice(settings.value("currentDevice", "").toString());
        return;
    }

    if (!isConnected()) {
        return;
    }

    settings.beginGroup(m_currentDevice);

    for (const QString &channel : m_channels) {
        settings.beginGroup(channel);
        getChannel(channel)->loadSettings(settings);
        settings.endGroup();
    }

    settings.endGroup();
}

void TwinleafCSB::saveSettings() const
{
    QSettings settings("_config.ini", QSettings::IniFormat);
    settings.setValue("currentDevice", m_currentDevice);

    if (!isConnected()) {
        return;
    }

    settings.beginGroup(m_currentDevice);
    for (const QString &channel : m_channels) {
        settings.beginGroup(channel);
        getChannel(channel)->saveSettings(settings);
        settings.endGroup();
    }
    settings.endGroup();
}

void TwinleafCSB::disconnectDevice()
{
    if (!m_serialPort->isOpen()) {
        logResponse("No device was connected.");
        return;
    }

    if (m_serialPort->waitForReadyRead(200)) {
        m_serialPort->readAll();
    }
    m_serialPort->close();

    m_currentDevice.clear();
    emit connectionStatusChanged();

    m_previousCommands.clear();
    logResponse("Disconnected.");
}

TwinleafCSB::ConnectStatus TwinleafCSB::connectDevice(const QString &deviceString)
{
    if (deviceString == m_currentDevice) {
        return ConnectStatus::Connected;
    }

    if (isConnected()) {
        disconnectDevice();
    }

    m_previousCommands.clear();

    if (deviceString.isEmpty()) {
        return ConnectStatus::DeviceNotFound;
    }

    QSerialPortInfo portInfo;
    for (const QSerialPortInfo &serialPortInfo : QSerialPortInfo::availablePorts()) {
        if (deviceString == generateDeviceString(serialPortInfo)) {
            portInfo = serialPortInfo;
            break;
        }
    }

    if (portInfo.isNull()) {
        logResponse("Cannot connect Twinleaf CSB.");
        return ConnectStatus::DeviceNotFound;
    }

    m_currentDevice = deviceString;

    m_serialPort->setPort(portInfo);
    m_serialPort->open(QSerialPort::ReadWrite);
    m_serialPort->setBaudRate(QSerialPort::Baud115200);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    loadSettings(false);

    emit connectionStatusChanged();

    logResponse("Connected.");
    return ConnectStatus::Connected;
}

TwinleafCSB::SentStatus TwinleafCSB::sendCommand(const QString &command, const QVariant &value)
{
    QByteArray fullCommand;

    QVariant roundedValue = value;
    if (value.canConvert<double>()) {
        roundedValue = QVariant(std::round(value.toDouble() * 1000000.0) / 1000000.0);
    }

    fullCommand.append((command + " " + roundedValue.toString() + "\r\n").toLatin1());

    if (m_previousCommands.contains(command) && m_previousCommands.value(command) == roundedValue) {
        logResponse(fullCommand + " (already set)");
        return SentStatus::AlreadySet;
    }

    m_previousCommands.insert(command, roundedValue);
    QString expectedResponse = QString("# %1(%2) = %2\r\n").arg(command, roundedValue.toString());

    return writeCommand(fullCommand, expectedResponse);
}

TwinleafCSB::SentStatus TwinleafCSB::sendCommand(const QString &command)
{
    QByteArray fullCommand;
    fullCommand.append((command + "\r\n").toLatin1());

    return writeCommand(fullCommand);
}

void TwinleafCSB::logResponse(const QString &message)
{
    m_lastResponse = message;
    emit responseReceived();
    qCDebug(twinleafCsbLog) << message;
}

TwinleafCSB::SentStatus TwinleafCSB::writeCommand(const QByteArray &command, const QString &expectedResponse)
{
    m_lastCommand = command;
    emit commandSent();

    if (!m_serialPort->isWritable()) {
        logResponse("Cannot write to Twinleaf CSB.");
        return SentStatus::Error;
    }

    m_serialPort->write(command);
    for (int i = 0; !m_serialPort->canReadLine() && (i < MAX_WRITE_ITERATIONS); i++) {
        m_serialPort->waitForReadyRead(WRITE_TIMEOUT_MS);
        QThread::msleep(WRITE_SLEEP_MS);
    }

    QString response = m_serialPort->readLine(256);
    logResponse(response);

    if (expectedResponse.isEmpty() || response == expectedResponse) {
        return SentStatus::SentAndConfirmed;
    } else {
        return SentStatus::SentButWrongResponse;
    }
}

QString TwinleafCSB::generateDeviceString(const QSerialPortInfo &portInfo) const
{
    if (!portInfo.hasVendorIdentifier() || !portInfo.hasProductIdentifier()) {
        return "";
    }

    return generateDeviceStringFromPortInfo(portInfo);
}

QString TwinleafCSB::generateDeviceStringFromPortInfo(const QSerialPortInfo &portInfo) const
{
    if (portInfo.vendorIdentifier() != 1155) {
        return "";
    }

    QString vendorId = QString::number(portInfo.vendorIdentifier());
    QString productId = QString::number(portInfo.productIdentifier());
    QString serialNumber = portInfo.serialNumber();

    return QString("SN: %0; VID: %2; PID: %1").arg(serialNumber, productId, vendorId);
}

CsbChannel::CsbChannel(TwinleafCSB *const parent, Channel channel)
    : QObject(parent)
    , m_offset(0.0)
    , m_amplitude(0.0)
    , m_frequency(0.0)
    , m_twinleafCsb(parent)
    , m_channel(channel)
{
}

CsbChannel::~CsbChannel()
{
}

void CsbChannel::setOffset(double value)
{
    QString command = getChannelPrefix();

    if (command.isEmpty()) {
        return;
    }

    command += ".current";

    switch (m_twinleafCsb->sendCommand(command, value)) {
    case TwinleafCSB::SentStatus::AlreadySet:
    case TwinleafCSB::SentStatus::Error:
    case TwinleafCSB::SentStatus::SentButWrongResponse:
        return;
    case TwinleafCSB::SentStatus::SentAndConfirmed:
        m_offset = value;
        emit offsetChanged();
        return;
    }
}

void CsbChannel::setFrequency(double value)
{
    QString command = getChannelPrefix();

    if (command.isEmpty()) {
        return;
    }

    command += ".modulation.frequency";

    switch (m_twinleafCsb->sendCommand(command, value)) {
    case TwinleafCSB::SentStatus::AlreadySet:
    case TwinleafCSB::SentStatus::Error:
    case TwinleafCSB::SentStatus::SentButWrongResponse:
        return;
    case TwinleafCSB::SentStatus::SentAndConfirmed:
        m_frequency = value;
        emit frequencyChanged();
        return;
    }
}

void CsbChannel::setAmplitude(double value)
{
    QString command = getChannelPrefix();

    if (command.isEmpty()) {
        return;
    }

    command += ".modulation.amplitude";

    switch (m_twinleafCsb->sendCommand(command, value)) {
    case TwinleafCSB::SentStatus::AlreadySet:
    case TwinleafCSB::SentStatus::Error:
    case TwinleafCSB::SentStatus::SentButWrongResponse:
        return;
    case TwinleafCSB::SentStatus::SentAndConfirmed:
        m_amplitude = value;
        emit amplitudeChanged();
        return;
    }
}

QString CsbChannel::getChannelPrefix() const
{
    switch (m_channel) {
    case Channel::X:
        return "coil.x";
    case Channel::Y:
        return "coil.y";
    case Channel::Z:
        return "coil.z";
    case Channel::None:
        return "";
    }

    return "";
}

void CsbChannel::loadSettings(QSettings &settings)
{
    setOffset(settings.value("offset", qQNaN()).toDouble());
    setAmplitude(settings.value("amplitude", qQNaN()).toDouble());
    setFrequency(settings.value("frequency", qQNaN()).toDouble());
}

void CsbChannel::saveSettings(QSettings &settings) const
{
    settings.setValue("offset", m_offset);
    settings.setValue("amplitude", m_amplitude);
    settings.setValue("frequency", m_frequency);
}
