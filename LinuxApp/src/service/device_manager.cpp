/**
 * @file device_manager.cpp
 * @brief Device Manager Service Implementation
 * @version 1.0.0
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#include "device_manager.h"
#include <QSerialPortInfo>
#include <QDebug>

DeviceManager::DeviceManager(QObject *parent)
    : QObject(parent)
    , m_heartbeatTimer(new QTimer(this))
{
    QObject::connect(m_heartbeatTimer, &QTimer::timeout,
                     this, &DeviceManager::performHeartbeat);
}

DeviceManager::~DeviceManager()
{
    disconnect();
}

bool DeviceManager::connect(const QString &portName, int baudrate)
{
    // Disconnect if already connected
    if (m_connected) {
        disconnect();
    }
    
    // Configure RS485
    rs485_config_t config = {};
    QByteArray portBytes = portName.toUtf8();
    config.device = portBytes.constData();
    config.baudrate = static_cast<uint32_t>(baudrate);
    config.data_bits = 8;
    config.parity = RS485_PARITY_NONE;
    config.stop_bits = 1;
    config.rs485_mode = true;
    config.timeout_ms = 500;
    
    // Create RS485 handle
    rs485_error_t err = rs485_create(&config, &m_rs485);
    if (err != RS485_OK) {
        emit connectionError(QString("Failed to create RS485: %1")
                            .arg(rs485_error_string(err)));
        return false;
    }
    
    // Open port
    err = rs485_open(m_rs485);
    if (err != RS485_OK) {
        emit connectionError(QString("Failed to open port: %1")
                            .arg(rs485_error_string(err)));
        rs485_destroy(&m_rs485);
        m_rs485 = nullptr;
        return false;
    }
    
    // Create protocol handle
    enersion_error_t perr = enersion_create(m_rs485, &m_protocol);
    if (perr != ENERSION_OK) {
        emit connectionError(QString("Failed to create protocol: %1")
                            .arg(enersion_error_string(perr)));
        rs485_close(m_rs485);
        rs485_destroy(&m_rs485);
        m_rs485 = nullptr;
        return false;
    }
    
    m_portName = portName;
    m_connected = true;
    
    emit connectedChanged(true);
    emit portNameChanged(m_portName);
    
    qDebug() << "Connected to" << portName << "@" << baudrate;
    
    // Start heartbeat monitoring
    startHeartbeat();
    
    return true;
}

void DeviceManager::disconnect()
{
    stopHeartbeat();
    
    if (m_protocol != nullptr) {
        enersion_destroy(&m_protocol);
        m_protocol = nullptr;
    }
    
    if (m_rs485 != nullptr) {
        rs485_close(m_rs485);
        rs485_destroy(&m_rs485);
        m_rs485 = nullptr;
    }
    
    if (m_connected) {
        m_connected = false;
        m_diOnline = false;
        m_doOnline = false;
        m_diHealth = 0;
        m_doHealth = 0;
        
        emit connectedChanged(false);
        emit diOnlineChanged(false);
        emit doOnlineChanged(false);
        emit diHealthChanged(0);
        emit doHealthChanged(0);
        
        qDebug() << "Disconnected";
    }
}

bool DeviceManager::isConnected() const
{
    return m_connected;
}

QStringList DeviceManager::availablePorts() const
{
    QStringList ports;
    
    const auto portInfos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : portInfos) {
        ports.append(QString("%1 - %2").arg(info.portName(), info.description()));
    }
    
    // Add MYIR STM32MP257 RS485 port as default
    if (ports.isEmpty()) {
        ports.append("/dev/ttySTM9 - RS485 (J2 Connector)");
        ports.append("/dev/ttySTM0 - STM32 UART");
        ports.append("/dev/ttySTM1 - STM32 UART");
        ports.append("/dev/ttyUSB0 - USB Serial");
    }
    
    return ports;
}

bool DeviceManager::pingDevice(int address)
{
    if (!m_connected || m_protocol == nullptr) {
        return false;
    }
    
    bool online = false;
    enersion_error_t err = enersion_ping(m_protocol,
                                         static_cast<enersion_addr_t>(address),
                                         &online);
    
    return (err == ENERSION_OK) && online;
}

DeviceInfo DeviceManager::getDeviceInfo(int address)
{
    DeviceInfo info;
    info.address = address;
    
    if (!m_connected || m_protocol == nullptr) {
        return info;
    }
    
    // Get device name
    info.name = QString::fromUtf8(enersion_device_name(
        static_cast<enersion_addr_t>(address)));
    
    // Ping device
    bool online = false;
    enersion_ping(m_protocol, static_cast<enersion_addr_t>(address), &online);
    info.online = online;
    
    if (online) {
        // Get version
        enersion_version_t version = {};
        if (enersion_get_version(m_protocol,
                                 static_cast<enersion_addr_t>(address),
                                 &version) == ENERSION_OK) {
            info.version = QString("v%1.%2.%3.%4")
                          .arg(version.major)
                          .arg(version.minor)
                          .arg(version.patch)
                          .arg(version.build);
        }
        
        // Get status
        enersion_status_t status = {};
        if (enersion_get_status(m_protocol,
                                static_cast<enersion_addr_t>(address),
                                &status) == ENERSION_OK) {
            info.health = status.health;
            info.uptime = status.uptime;
            info.rxPackets = status.rx_packet_count;
            info.txPackets = status.tx_packet_count;
            info.errors = status.error_count;
        }
    }
    
    return info;
}

void DeviceManager::scanDevices()
{
    if (!m_connected) {
        return;
    }
    
    // Scan DI Controller (0x02)
    if (pingDevice(ENERSION_ADDR_CTRL_DIO)) {
        emit deviceDiscovered(ENERSION_ADDR_CTRL_DIO, "Controller DIO");
        updateDeviceStatus(ENERSION_ADDR_CTRL_DIO, true, 100);
    }
    
    // Scan DO Controller (0x03)
    if (pingDevice(ENERSION_ADDR_CTRL_OUT)) {
        emit deviceDiscovered(ENERSION_ADDR_CTRL_OUT, "Controller OUT");
        updateDeviceStatus(ENERSION_ADDR_CTRL_OUT, true, 100);
    }
}

void DeviceManager::startHeartbeat()
{
    if (!m_heartbeatTimer->isActive()) {
        m_heartbeatTimer->start(HEARTBEAT_INTERVAL_MS);
        qDebug() << "Heartbeat monitoring started";
    }
}

void DeviceManager::stopHeartbeat()
{
    if (m_heartbeatTimer->isActive()) {
        m_heartbeatTimer->stop();
        qDebug() << "Heartbeat monitoring stopped";
    }
}

void DeviceManager::performHeartbeat()
{
    if (!m_connected || m_protocol == nullptr) {
        return;
    }
    
    // Heartbeat DI Controller
    uint8_t diHealth = 0;
    enersion_error_t err = enersion_heartbeat(m_protocol,
                                              ENERSION_ADDR_CTRL_DIO,
                                              &diHealth);
    
    bool diOnline = (err == ENERSION_OK);
    if (diOnline != m_diOnline) {
        m_diOnline = diOnline;
        emit diOnlineChanged(diOnline);
    }
    
    if (diOnline) {
        if (static_cast<int>(diHealth) != m_diHealth) {
            m_diHealth = static_cast<int>(diHealth);
            emit diHealthChanged(m_diHealth);
        }
        emit heartbeatReceived(ENERSION_ADDR_CTRL_DIO, m_diHealth);
    }
    
    // Heartbeat DO Controller
    uint8_t doHealth = 0;
    err = enersion_heartbeat(m_protocol, ENERSION_ADDR_CTRL_OUT, &doHealth);
    
    bool doOnline = (err == ENERSION_OK);
    if (doOnline != m_doOnline) {
        m_doOnline = doOnline;
        emit doOnlineChanged(doOnline);
    }
    
    if (doOnline) {
        if (static_cast<int>(doHealth) != m_doHealth) {
            m_doHealth = static_cast<int>(doHealth);
            emit doHealthChanged(m_doHealth);
        }
        emit heartbeatReceived(ENERSION_ADDR_CTRL_OUT, m_doHealth);
    }
}

void DeviceManager::updateDeviceStatus(int address, bool online, int health)
{
    if (address == ENERSION_ADDR_CTRL_DIO) {
        if (online != m_diOnline) {
            m_diOnline = online;
            emit diOnlineChanged(online);
        }
        if (health != m_diHealth) {
            m_diHealth = health;
            emit diHealthChanged(health);
        }
    } else if (address == ENERSION_ADDR_CTRL_OUT) {
        if (online != m_doOnline) {
            m_doOnline = online;
            emit doOnlineChanged(online);
        }
        if (health != m_doHealth) {
            m_doHealth = health;
            emit doHealthChanged(health);
        }
    }
}

