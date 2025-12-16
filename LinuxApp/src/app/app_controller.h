/**
 * @file app_controller.h
 * @brief Main Application Controller - Qt/QML Backend
 * @version 1.0.0
 * 
 * Manages communication between QML UI and hardware via the protocol layer.
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QStringList>
#include <memory>

// Forward declarations
extern "C" {
    struct rs485_handle_tag;
    struct enersion_ctx_tag;
}

class DigitalInputModel;
class DigitalOutputModel;

/**
 * @brief Main Application Controller
 * 
 * Provides QML interface for:
 * - RS485 connection management
 * - Digital Input reading
 * - Digital Output control
 * - System status monitoring
 */
class AppController : public QObject
{
    Q_OBJECT
    
    // Connection properties
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionChanged)
    Q_PROPERTY(QStringList availablePorts READ availablePorts NOTIFY portsChanged)
    
    // Device info properties
    Q_PROPERTY(QString firmwareVersion READ firmwareVersion NOTIFY deviceInfoChanged)
    Q_PROPERTY(QString uptime READ uptime NOTIFY deviceInfoChanged)
    Q_PROPERTY(int errorCount READ errorCount NOTIFY deviceInfoChanged)
    
public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController() override;
    
    // Property getters
    bool isConnected() const;
    QString connectionStatus() const;
    QStringList availablePorts() const;
    QString firmwareVersion() const;
    QString uptime() const;
    int errorCount() const;
    
    // Model accessors (for QML)
    DigitalInputModel* diModel() const { return m_diModel; }
    DigitalOutputModel* doModel() const { return m_doModel; }
    
public slots:
    // Connection management
    void autoConnect();
    void connectDevice(const QString &port, int baudrate);
    void disconnectDevice();
    void refreshPorts();
    
    // Digital Output control
    void toggleOutput(int channel);
    void setOutput(int channel, bool state);
    void setAllOutputs(bool state);
    void setPattern(int pattern);
    void setFirstHalf(bool state);
    void toggleBank(int bank);
    
    // Polling control
    void setDiPollInterval(int ms);
    void setDoPollInterval(int ms);
    void startPolling();
    void stopPolling();
    
signals:
    void connectionChanged();
    void portsChanged();
    void deviceInfoChanged();
    void error(const QString &message);
    
private slots:
    void pollDigitalInputs();
    void pollDigitalOutputs();
    void updateDeviceInfo();
    
private:
    // Initialize/cleanup
    bool initializeProtocol(const QString &device, int baudrate);
    void cleanupProtocol();
    
    // Scan for available serial ports
    void scanPorts();
    
    // Connection state
    bool m_isConnected;
    QString m_currentPort;
    int m_currentBaudrate;
    QStringList m_availablePorts;
    
    // Device info
    QString m_firmwareVersion;
    uint32_t m_uptimeSeconds;
    int m_errorCount;
    
    // Protocol handles
    struct rs485_handle_tag *m_rs485Handle;
    struct enersion_ctx_tag *m_protocolHandle;
    
    // Data models
    DigitalInputModel *m_diModel;
    DigitalOutputModel *m_doModel;
    
    // Polling timers
    QTimer *m_diPollTimer;
    QTimer *m_doPollTimer;
    QTimer *m_infoTimer;
    
    // Polling intervals
    int m_diPollInterval;
    int m_doPollInterval;
};

#endif // APP_CONTROLLER_H
