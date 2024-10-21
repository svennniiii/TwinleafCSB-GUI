#ifndef DRIVER_H
#define DRIVER_H

#include <QObject>
#include <qqml.h>
#include <QSerialPortInfo>
#include <QSettings>
#include <QLoggingCategory>

class TwinleafCSB;

class CsbChannel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double offset READ getOffset WRITE setOffset NOTIFY offsetChanged)
    Q_PROPERTY(double amplitude READ getAmplitude WRITE setAmplitude NOTIFY amplitudeChanged)
    Q_PROPERTY(double frequency READ getFrequency WRITE setFrequency NOTIFY frequencyChanged)
    Q_ENUMS(Channel)
    Q_ENUMS(Parameter)

public:
    enum class Channel {
        None,
        X,
        Y,
        Z,
    };

    enum class Parameter {
        None,
        Offset,
        ModAmplitude,
        ModFrequency,
    };

    explicit CsbChannel(TwinleafCSB *const parent, Channel channel);
    ~CsbChannel();

    void setOffset(double value);
    double getOffset() const { return m_offset; }

    void setFrequency(double value);
    double getFrequency() const { return m_frequency; }

    void setAmplitude(double value);
    double getAmplitude() const { return m_amplitude; }

    void loadSettings(QSettings &settings);
    void saveSettings(QSettings &settings) const;

signals:
    void offsetChanged();
    void amplitudeChanged();
    void frequencyChanged();

private:
    QString getChannelPrefix() const;

private:
    double m_offset;
    double m_amplitude;
    double m_frequency;

    const QPointer<TwinleafCSB> m_twinleafCsb;
    const Channel m_channel;
};

class TwinleafCSB : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString lastResponse MEMBER m_lastResponse NOTIFY responseReceived)
    Q_PROPERTY(QString lastCommand MEMBER m_lastCommand NOTIFY commandSent)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionStatusChanged)
    Q_PROPERTY(QString currentDevice WRITE connectDevice READ getCurrentDevice NOTIFY connectionStatusChanged)

    Q_PROPERTY(CsbChannel *xChannel READ getXChannel)
    Q_PROPERTY(CsbChannel *yChannel READ getYChannel)
    Q_PROPERTY(CsbChannel *zChannel READ getZChannel)

    Q_ENUMS(SentStatus)
    Q_ENUMS(ConnectStatus)

    QML_ELEMENT

public:
    enum class SentStatus {
        SentAndConfirmed = 0,
        AlreadySet = 1,
        Error = 2,
        SentButWrongResponse,
    };

    enum class ConnectStatus {
        Connected = 0,
        DeviceNotFound = 1,
    };

    explicit TwinleafCSB(QObject *parent = nullptr);
    ~TwinleafCSB();

    Q_INVOKABLE QStringList getDeviceList() const;
    Q_INVOKABLE CsbChannel *getChannel(const QString &channelName) const;
    bool isConnected() const;

    Q_INVOKABLE void loadSettings(bool loadDevice = true);
    void saveSettings() const;

public slots:
    void disconnectDevice();
    ConnectStatus connectDevice(const QString &deviceString);
    SentStatus sendCommand(const QString &command, const QVariant &value);
    SentStatus sendCommand(const QString &command);

signals:
    void deviceListChanged();
    void responseReceived();
    void commandSent();
    void connectionStatusChanged();

    void xChannelChanged();

private:
    void logResponse(const QString &message);
    TwinleafCSB::SentStatus writeCommand(const QByteArray &command, const QString &expectedResponse = "");
    QString generateDeviceString(const QSerialPortInfo &portInfo) const;
    QString generateDeviceStringFromPortInfo(const QSerialPortInfo &portInfo) const;

    CsbChannel *getXChannel() const { return m_xChannel.data(); }
    CsbChannel *getYChannel() const { return m_yChannel.data(); }
    CsbChannel *getZChannel() const { return m_zChannel.data(); }

    QString getCurrentDevice() const { return m_currentDevice; }

private:
    QScopedPointer<QSerialPort> m_serialPort;
    QString m_currentDevice;
    QString m_lastResponse;
    QString m_lastCommand;
    const QStringList m_channels;
    const QPointer<CsbChannel> m_xChannel;
    const QPointer<CsbChannel> m_yChannel;
    const QPointer<CsbChannel> m_zChannel;

    QMap<QString, QVariant> m_previousCommands;

    static constexpr int MAX_WRITE_ITERATIONS = 10;
    static constexpr int WRITE_TIMEOUT_MS = 20;
    static constexpr int WRITE_SLEEP_MS = 10;
};

Q_DECLARE_LOGGING_CATEGORY(twinleafCsbLog)

#endif // DRIVER_H
