/**
 * @file do_service.cpp
 * @brief Digital Output Service Implementation
 * @version 1.0.0
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#include "do_service.h"
#include <QDebug>

extern "C" {
#include "enersion_protocol.h"
}

DoService::DoService(DeviceManager *deviceManager, QObject *parent)
    : QObject(parent)
    , m_deviceManager(deviceManager)
    , m_outputStates(CHANNEL_COUNT, false)
    , m_deviceStates(CHANNEL_COUNT, false)
{
}

DoService::~DoService() = default;

bool DoService::getOutput(int channel) const
{
    if (channel >= 0 && channel < CHANNEL_COUNT) {
        return m_outputStates[channel];
    }
    return false;
}

int DoService::getActiveOutputCount() const
{
    return m_activeCount;
}

void DoService::setOutput(int channel, bool state)
{
    if (channel >= 0 && channel < CHANNEL_COUNT) {
        if (m_outputStates[channel] != state) {
            m_outputStates[channel] = state;
            updateActiveCount();
            markPendingChanges();
            emit outputChanged(channel, state);
            emit outputStatesChanged();
        }
    }
}

void DoService::toggleOutput(int channel)
{
    if (channel >= 0 && channel < CHANNEL_COUNT) {
        setOutput(channel, !m_outputStates[channel]);
    }
}

void DoService::setAllOutputs(bool state)
{
    bool changed = false;
    
    for (int i = 0; i < CHANNEL_COUNT; i++) {
        if (m_outputStates[i] != state) {
            m_outputStates[i] = state;
            changed = true;
            emit outputChanged(i, state);
        }
    }
    
    if (changed) {
        updateActiveCount();
        markPendingChanges();
        emit outputStatesChanged();
    }
}

void DoService::clearAllOutputs()
{
    setAllOutputs(false);
}

bool DoService::writeOutputs()
{
    if (m_deviceManager == nullptr || !m_deviceManager->isConnected()) {
        emit writeError("Not connected to device");
        return false;
    }
    
    enersion_handle_t protocol = m_deviceManager->protocolHandle();
    if (protocol == nullptr) {
        emit writeError("Protocol not initialized");
        return false;
    }
    
    // Build output state
    enersion_dio_state_t state = {};
    enersion_dio_clear_all(&state);
    
    for (int i = 0; i < CHANNEL_COUNT; i++) {
        if (m_outputStates[i]) {
            enersion_dio_set_bit(&state, static_cast<uint8_t>(i), true);
        }
    }
    
    // Write to device
    enersion_error_t err = enersion_write_digital_outputs(protocol, &state);
    
    if (err != ENERSION_OK) {
        emit writeError(QString("Write failed: %1")
                       .arg(enersion_error_string(err)));
        return false;
    }
    
    // Update device states to match
    m_deviceStates = m_outputStates;
    m_pendingChanges = false;
    emit pendingChangesChanged(false);
    
    emit writeSuccess();
    
    qDebug() << "DO write complete:" << m_activeCount << "outputs active";
    
    return true;
}

bool DoService::readOutputs()
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
    
    // Read from device
    enersion_dio_state_t state = {};
    enersion_error_t err = enersion_read_digital_outputs(protocol, &state);
    
    if (err != ENERSION_OK) {
        emit readError(QString("Read failed: %1")
                      .arg(enersion_error_string(err)));
        return false;
    }
    
    // Update states
    bool statesChanged = false;
    
    for (int i = 0; i < CHANNEL_COUNT; i++) {
        bool newState = enersion_dio_get_bit(&state, static_cast<uint8_t>(i));
        
        if (newState != m_outputStates[i]) {
            m_outputStates[i] = newState;
            statesChanged = true;
            emit outputChanged(i, newState);
        }
        
        m_deviceStates[i] = newState;
    }
    
    if (statesChanged) {
        updateActiveCount();
        emit outputStatesChanged();
    }
    
    // Clear pending changes since we synced with device
    m_pendingChanges = false;
    emit pendingChangesChanged(false);
    
    emit readSuccess(m_activeCount);
    
    qDebug() << "DO read complete:" << m_activeCount << "outputs active";
    
    return true;
}

void DoService::updateActiveCount()
{
    int count = 0;
    for (int i = 0; i < CHANNEL_COUNT; i++) {
        if (m_outputStates[i]) {
            count++;
        }
    }
    
    if (count != m_activeCount) {
        m_activeCount = count;
        emit activeCountChanged(m_activeCount);
    }
}

void DoService::markPendingChanges()
{
    // Check if current states differ from device states
    bool pending = false;
    for (int i = 0; i < CHANNEL_COUNT; i++) {
        if (m_outputStates[i] != m_deviceStates[i]) {
            pending = true;
            break;
        }
    }
    
    if (pending != m_pendingChanges) {
        m_pendingChanges = pending;
        emit pendingChangesChanged(pending);
    }
}



