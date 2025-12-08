/**
 * @file di_service.cpp
 * @brief Digital Input Service Implementation
 * @version 1.0.0
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#include "di_service.h"
#include <QDebug>

extern "C" {
#include "enersion_protocol.h"
}

DiService::DiService(DeviceManager *deviceManager, QObject *parent)
    : QObject(parent)
    , m_deviceManager(deviceManager)
    , m_refreshTimer(new QTimer(this))
    , m_inputStates(CHANNEL_COUNT, false)
{
    QObject::connect(m_refreshTimer, &QTimer::timeout,
                     this, &DiService::onRefreshTimeout);
}

DiService::~DiService()
{
    stopAutoRefresh();
}

void DiService::setAutoRefresh(bool enable)
{
    if (m_autoRefresh != enable) {
        m_autoRefresh = enable;
        
        if (enable) {
            startAutoRefresh();
        } else {
            stopAutoRefresh();
        }
        
        emit autoRefreshChanged(enable);
    }
}

void DiService::setRefreshInterval(int ms)
{
    if (ms >= 100 && ms != m_refreshInterval) {
        m_refreshInterval = ms;
        
        if (m_refreshTimer->isActive()) {
            m_refreshTimer->setInterval(m_refreshInterval);
        }
        
        emit refreshIntervalChanged(ms);
    }
}

bool DiService::getInput(int channel) const
{
    if (channel >= 0 && channel < CHANNEL_COUNT) {
        return m_inputStates[channel];
    }
    return false;
}

int DiService::getActiveInputCount() const
{
    return m_activeCount;
}

bool DiService::readInputs()
{
    if (m_deviceManager == nullptr || !m_deviceManager->isConnected()) {
        emit readError("Not connected to device");
        return false;
    }
    
    enersion_handle_t protocol = m_deviceManager->protocolHandle();
    if (protocol == nullptr) {
        emit readError("Protocol not initialized");
        return false;
    }
    
    // Read digital inputs
    enersion_dio_state_t state = {};
    enersion_error_t err = enersion_read_digital_inputs(protocol, &state);
    
    if (err != ENERSION_OK) {
        emit readError(QString("Read failed: %1")
                      .arg(enersion_error_string(err)));
        return false;
    }
    
    // Update states
    int newActiveCount = 0;
    bool statesChanged = false;
    
    for (int i = 0; i < CHANNEL_COUNT; i++) {
        bool newState = enersion_dio_get_bit(&state, static_cast<uint8_t>(i));
        
        if (newState != m_inputStates[i]) {
            m_inputStates[i] = newState;
            statesChanged = true;
            emit inputChanged(i, newState);
        }
        
        if (newState) {
            newActiveCount++;
        }
    }
    
    if (statesChanged) {
        emit inputStatesChanged();
    }
    
    if (newActiveCount != m_activeCount) {
        m_activeCount = newActiveCount;
        emit activeCountChanged(m_activeCount);
    }
    
    emit readSuccess(m_activeCount);
    
    qDebug() << "DI read complete:" << m_activeCount << "active inputs";
    
    return true;
}

void DiService::startAutoRefresh()
{
    if (!m_refreshTimer->isActive()) {
        m_refreshTimer->start(m_refreshInterval);
        qDebug() << "Auto-refresh started with interval:" << m_refreshInterval << "ms";
    }
}

void DiService::stopAutoRefresh()
{
    if (m_refreshTimer->isActive()) {
        m_refreshTimer->stop();
        qDebug() << "Auto-refresh stopped";
    }
}

void DiService::onRefreshTimeout()
{
    readInputs();
}

