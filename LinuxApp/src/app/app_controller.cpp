/**
 * @file app_controller.cpp
 * @brief Main Application Controller Implementation
 * @version 1.0.0
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#include "app_controller.h"
#include "models/digital_input_model.h"
#include "models/digital_output_model.h"

#include <QDir>
#include <QDebug>

extern "C" {
#include "rs485_serial.h"
#include "rs485_gpio.h"
#include "enersion_protocol.h"
#include "enersion_types.h"
}

AppController::AppController(QObject *parent)
    : QObject(parent)
    , m_isConnected(false)
    , m_currentPort("/dev/ttySTM9")
    , m_currentBaudrate(115200)
    , m_firmwareVersion("N/A")
    , m_uptimeSeconds(0)
    , m_errorCount(0)
    , m_rs485Handle(nullptr)
    , m_protocolHandle(nullptr)
    , m_diModel(new DigitalInputModel(this))
    , m_doModel(new DigitalOutputModel(this))
    , m_diPollTimer(new QTimer(this))
    , m_doPollTimer(new QTimer(this))
    , m_infoTimer(new QTimer(this))
    , m_diPollInterval(100)
    , m_doPollInterval(200)
{
    // Connect timers
    connect(m_diPollTimer, &QTimer::timeout, this, &AppController::pollDigitalInputs);
    connect(m_doPollTimer, &QTimer::timeout, this, &AppController::pollDigitalOutputs);
    connect(m_infoTimer, &QTimer::timeout, this, &AppController::updateDeviceInfo);
    
    // Scan for available ports
    scanPorts();
    
    qDebug() << "AppController initialized";
}

AppController::~AppController()
{
    stopPolling();
    cleanupProtocol();
}

// ============================================================================
// Property Getters
// ============================================================================

bool AppController::isConnected() const
{
    return m_isConnected;
}

QString AppController::connectionStatus() const
{
    if (m_isConnected) {
        return QString("Connected to %1 @ %2").arg(m_currentPort).arg(m_currentBaudrate);
    }
    return "Disconnected";
}

QStringList AppController::availablePorts() const
{
    return m_availablePorts;
}

QString AppController::firmwareVersion() const
{
    return m_firmwareVersion;
}

QString AppController::uptime() const
{
    uint32_t hours = m_uptimeSeconds / 3600;
    uint32_t minutes = (m_uptimeSeconds % 3600) / 60;
    uint32_t seconds = m_uptimeSeconds % 60;
    return QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
}

int AppController::errorCount() const
{
    return m_errorCount;
}

// ============================================================================
// Connection Management
// ============================================================================

void AppController::autoConnect()
{
    qDebug() << "Auto-connecting to default port...";
    connectDevice("/dev/ttySTM9", 115200);
}

void AppController::connectDevice(const QString &port, int baudrate)
{
    qDebug() << "Connecting to" << port << "@" << baudrate;
    
    if (m_isConnected) {
        disconnectDevice();
    }
    
    if (initializeProtocol(port, baudrate)) {
        m_isConnected = true;
        m_currentPort = port;
        m_currentBaudrate = baudrate;
        
        emit connectionChanged();
        
        // Start polling
        startPolling();
        
        // Get initial device info
        updateDeviceInfo();
        
        qDebug() << "Connected successfully";
    } else {
        emit error("Failed to connect to " + port);
    }
}

void AppController::disconnectDevice()
{
    qDebug() << "Disconnecting...";
    
    stopPolling();
    cleanupProtocol();
    
    m_isConnected = false;
    m_firmwareVersion = "N/A";
    m_uptimeSeconds = 0;
    
    emit connectionChanged();
    emit deviceInfoChanged();
}

void AppController::refreshPorts()
{
    scanPorts();
    emit portsChanged();
}

void AppController::scanPorts()
{
    m_availablePorts.clear();
    
    // Add MYIR default RS485 port
    m_availablePorts.append("/dev/ttySTM9 - RS485 (J2 Connector)");
    
    // Scan for other serial ports
    QDir devDir("/dev");
    QStringList filters;
    filters << "ttySTM*" << "ttyUSB*" << "ttyACM*";
    
    QStringList entries = devDir.entryList(filters, QDir::System);
    for (const QString &entry : entries) {
        QString path = "/dev/" + entry;
        if (!m_availablePorts.contains(path) && entry != "ttySTM9") {
            m_availablePorts.append(path + " - " + entry);
        }
    }
    
    qDebug() << "Found ports:" << m_availablePorts;
}

// ============================================================================
// Protocol Initialization
// ============================================================================

bool AppController::initializeProtocol(const QString &device, int baudrate)
{
    rs485_config_t config;
    config.device = device.toUtf8().constData();
    config.baudrate = static_cast<uint32_t>(baudrate);
    config.data_bits = 8;
    config.parity = RS485_PARITY_NONE;
    config.stop_bits = 1;
    config.rs485_mode = true;
    config.timeout_ms = 500;
    
    // Create RS485 handle
    rs485_error_t rs_err = rs485_create(&config, &m_rs485Handle);
    if (rs_err != RS485_OK) {
        qWarning() << "Failed to create RS485 handle:" << rs485_error_string(rs_err);
        return false;
    }
    
    // Open RS485 port
    rs_err = rs485_open(m_rs485Handle);
    if (rs_err != RS485_OK) {
        qWarning() << "Failed to open RS485 port:" << rs485_error_string(rs_err);
        rs485_destroy(&m_rs485Handle);
        m_rs485Handle = nullptr;
        return false;
    }
    
    // Create protocol handle
    enersion_error_t en_err = enersion_create(m_rs485Handle, &m_protocolHandle);
    if (en_err != ENERSION_OK) {
        qWarning() << "Failed to create protocol handle:" << enersion_error_string(en_err);
        rs485_close(m_rs485Handle);
        rs485_destroy(&m_rs485Handle);
        m_rs485Handle = nullptr;
        return false;
    }
    
    // Test connection with ping
    bool diOnline = false;
    bool doOnline = false;
    
    enersion_ping(m_protocolHandle, ENERSION_ADDR_CTRL_DIO, &diOnline);
    enersion_ping(m_protocolHandle, ENERSION_ADDR_CTRL_OUT, &doOnline);
    
    qDebug() << "DI Controller:" << (diOnline ? "Online" : "Offline");
    qDebug() << "DO Controller:" << (doOnline ? "Online" : "Offline");
    
    return diOnline || doOnline;
}

void AppController::cleanupProtocol()
{
    if (m_protocolHandle) {
        enersion_destroy(&m_protocolHandle);
        m_protocolHandle = nullptr;
    }
    
    if (m_rs485Handle) {
        rs485_close(m_rs485Handle);
        rs485_destroy(&m_rs485Handle);
        m_rs485Handle = nullptr;
    }
}

// ============================================================================
// Digital Output Control
// ============================================================================

void AppController::toggleOutput(int channel)
{
    if (!m_isConnected || !m_protocolHandle) {
        return;
    }
    
    if (channel < 0 || channel >= 64) {
        return;
    }
    
    bool currentState = m_doModel->getChannelState(channel);
    setOutput(channel, !currentState);
}

void AppController::setOutput(int channel, bool state)
{
    if (!m_isConnected || !m_protocolHandle) {
        return;
    }
    
    if (channel < 0 || channel >= 64) {
        return;
    }
    
    // Update local model immediately for responsiveness
    m_doModel->setChannelState(channel, state);
    
    // Send to hardware
    enersion_dio_state_t doState;
    m_doModel->getStateData(doState.state);
    
    enersion_error_t err = enersion_write_digital_outputs(m_protocolHandle, &doState);
    if (err != ENERSION_OK) {
        qWarning() << "Failed to write DO:" << enersion_error_string(err);
        m_errorCount++;
        emit deviceInfoChanged();
    }
}

void AppController::setAllOutputs(bool state)
{
    if (!m_isConnected || !m_protocolHandle) {
        return;
    }
    
    enersion_dio_state_t doState;
    
    if (state) {
        enersion_dio_set_all(&doState);
    } else {
        enersion_dio_clear_all(&doState);
    }
    
    enersion_error_t err = enersion_write_digital_outputs(m_protocolHandle, &doState);
    if (err == ENERSION_OK) {
        m_doModel->setAllChannels(state);
    } else {
        qWarning() << "Failed to write DO:" << enersion_error_string(err);
        m_errorCount++;
        emit deviceInfoChanged();
    }
}

void AppController::setPattern(int pattern)
{
    if (!m_isConnected || !m_protocolHandle) {
        return;
    }
    
    enersion_dio_state_t doState;
    for (int i = 0; i < 8; i++) {
        doState.state[i] = static_cast<uint8_t>(pattern);
    }
    
    enersion_error_t err = enersion_write_digital_outputs(m_protocolHandle, &doState);
    if (err == ENERSION_OK) {
        m_doModel->setStateData(doState.state);
    } else {
        qWarning() << "Failed to write pattern:" << enersion_error_string(err);
        m_errorCount++;
        emit deviceInfoChanged();
    }
}

void AppController::setFirstHalf(bool state)
{
    if (!m_isConnected || !m_protocolHandle) {
        return;
    }
    
    enersion_dio_state_t doState;
    enersion_dio_clear_all(&doState);
    
    if (state) {
        for (int i = 0; i < 4; i++) {
            doState.state[i] = 0xFF;
        }
    }
    
    enersion_error_t err = enersion_write_digital_outputs(m_protocolHandle, &doState);
    if (err == ENERSION_OK) {
        m_doModel->setStateData(doState.state);
    } else {
        qWarning() << "Failed to set first half:" << enersion_error_string(err);
        m_errorCount++;
        emit deviceInfoChanged();
    }
}

void AppController::toggleBank(int bank)
{
    if (!m_isConnected || !m_protocolHandle) {
        return;
    }
    
    if (bank < 0 || bank >= 8) {
        return;
    }
    
    enersion_dio_state_t doState;
    m_doModel->getStateData(doState.state);
    
    // Toggle all bits in the bank
    doState.state[bank] = ~doState.state[bank];
    
    enersion_error_t err = enersion_write_digital_outputs(m_protocolHandle, &doState);
    if (err == ENERSION_OK) {
        m_doModel->setStateData(doState.state);
    } else {
        qWarning() << "Failed to toggle bank:" << enersion_error_string(err);
        m_errorCount++;
        emit deviceInfoChanged();
    }
}

// ============================================================================
// Polling
// ============================================================================

void AppController::setDiPollInterval(int ms)
{
    m_diPollInterval = qBound(50, ms, 10000);
    if (m_diPollTimer->isActive()) {
        m_diPollTimer->setInterval(m_diPollInterval);
    }
}

void AppController::setDoPollInterval(int ms)
{
    m_doPollInterval = qBound(50, ms, 10000);
    if (m_doPollTimer->isActive()) {
        m_doPollTimer->setInterval(m_doPollInterval);
    }
}

void AppController::startPolling()
{
    m_diPollTimer->start(m_diPollInterval);
    m_doPollTimer->start(m_doPollInterval);
    m_infoTimer->start(5000);  // Update device info every 5 seconds
    
    qDebug() << "Polling started - DI:" << m_diPollInterval << "ms, DO:" << m_doPollInterval << "ms";
}

void AppController::stopPolling()
{
    m_diPollTimer->stop();
    m_doPollTimer->stop();
    m_infoTimer->stop();
    
    qDebug() << "Polling stopped";
}

void AppController::pollDigitalInputs()
{
    if (!m_isConnected || !m_protocolHandle) {
        return;
    }
    
    enersion_dio_state_t state;
    enersion_error_t err = enersion_read_digital_inputs(m_protocolHandle, &state);
    
    if (err == ENERSION_OK) {
        m_diModel->setStateData(state.state);
    } else {
        qWarning() << "Failed to read DI:" << enersion_error_string(err);
        m_errorCount++;
        emit deviceInfoChanged();
    }
}

void AppController::pollDigitalOutputs()
{
    if (!m_isConnected || !m_protocolHandle) {
        return;
    }
    
    enersion_dio_state_t state;
    enersion_error_t err = enersion_read_digital_outputs(m_protocolHandle, &state);
    
    if (err == ENERSION_OK) {
        m_doModel->setStateData(state.state);
    } else {
        // Don't report this as error - we may have just written to it
    }
}

void AppController::updateDeviceInfo()
{
    if (!m_isConnected || !m_protocolHandle) {
        return;
    }
    
    // Get version from DI controller
    enersion_version_t version;
    if (enersion_get_version(m_protocolHandle, ENERSION_ADDR_CTRL_DIO, &version) == ENERSION_OK) {
        m_firmwareVersion = QString("v%1.%2.%3")
            .arg(version.major)
            .arg(version.minor)
            .arg(version.patch);
    }
    
    // Get status from DI controller
    enersion_status_t status;
    if (enersion_get_status(m_protocolHandle, ENERSION_ADDR_CTRL_DIO, &status) == ENERSION_OK) {
        m_uptimeSeconds = status.uptime;
    }
    
    emit deviceInfoChanged();
}
