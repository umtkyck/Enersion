/**
 * @file device_manager.h
 * @brief Device Manager Service - Connection and Health Management
 * @version 1.0.0
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QThread>
#include <memory>

extern "C" {
#include "enersion_protocol.h"
#include "rs485_serial.h"
}

/**
 * @brief Device information structure
 */
struct DeviceInfo {
    Q_GADGET
    Q_PROPERTY(int address MEMBER address)
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(bool online MEMBER online)
    Q_PROPERTY(int health MEMBER health)
    Q_PROPERTY(QString version MEMBER version)
    Q_PROPERTY(quint32 uptime MEMBER uptime)
    
public:
    int address = 0;
    QString name;
    bool online = false;
    int health = 0;
    QString version;
    quint32 uptime = 0;
    quint32 rxPackets = 0;
    quint32 txPackets = 0;
    quint32 errors = 0;
};

/**
 * @brief Device Manager - Manages RS485 connection and device communication
 */
class DeviceManager : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(QString portName READ portName NOTIFY portNameChanged)
    Q_PROPERTY(int diControllerHealth READ diControllerHealth NOTIFY diHealthChanged)
    Q_PROPERTY(int doControllerHealth READ doControllerHealth NOTIFY doHealthChanged)
    Q_PROPERTY(bool diOnline READ isDiOnline NOTIFY diOnlineChanged)
    Q_PROPERTY(bool doOnline READ isDoOnline NOTIFY doOnlineChanged)
    
public:
    explicit DeviceManager(QObject *parent = nullptr);
    ~DeviceManager() override;
    
    // Connection management
    Q_INVOKABLE bool connect(const QString &portName, int baudrate = 115200);
    Q_INVOKABLE void disconnect();
    Q_INVOKABLE bool isConnected() const;
    Q_INVOKABLE QStringList availablePorts() const;
    
    // Device queries
    Q_INVOKABLE bool pingDevice(int address);
    Q_INVOKABLE DeviceInfo getDeviceInfo(int address);
    Q_INVOKABLE void scanDevices();
    
    // Protocol access for services
    enersion_handle_t protocolHandle() const { return m_protocol; }
    
    // Property getters
    QString portName() const { return m_portName; }
    int diControllerHealth() const { return m_diHealth; }
    int doControllerHealth() const { return m_doHealth; }
    bool isDiOnline() const { return m_diOnline; }
    bool isDoOnline() const { return m_doOnline; }
    
signals:
    void connectedChanged(bool connected);
    void portNameChanged(const QString &portName);
    void diHealthChanged(int health);
    void doHealthChanged(int health);
    void diOnlineChanged(bool online);
    void doOnlineChanged(bool online);
    void deviceDiscovered(int address, const QString &name);
    void connectionError(const QString &error);
    void heartbeatReceived(int address, int health);
    
public slots:
    void startHeartbeat();
    void stopHeartbeat();
    
private slots:
    void performHeartbeat();
    
private:
    void updateDeviceStatus(int address, bool online, int health);
    
    rs485_handle_t m_rs485 = nullptr;
    enersion_handle_t m_protocol = nullptr;
    QString m_portName;
    bool m_connected = false;
    
    // Device status
    bool m_diOnline = false;
    bool m_doOnline = false;
    int m_diHealth = 0;
    int m_doHealth = 0;
    
    // Heartbeat timer
    QTimer *m_heartbeatTimer = nullptr;
    static constexpr int HEARTBEAT_INTERVAL_MS = 2000;
};

#endif // DEVICE_MANAGER_H

