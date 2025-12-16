/**
 * @file digital_input_model.cpp
 * @brief Digital Input Data Model Implementation
 * @version 1.0.0
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#include "digital_input_model.h"
#include <cstring>

DigitalInputModel::DigitalInputModel(QObject *parent)
    : QObject(parent)
    , m_isPolling(false)
{
    std::memset(m_state, 0, sizeof(m_state));
}

int DigitalInputModel::activeCount() const
{
    int count = 0;
    for (int i = 0; i < 64; i++) {
        if (getChannelState(i)) {
            count++;
        }
    }
    return count;
}

bool DigitalInputModel::isPolling() const
{
    return m_isPolling;
}

bool DigitalInputModel::getChannelState(int channel) const
{
    if (channel < 0 || channel >= 64) {
        return false;
    }
    
    int byteIdx = channel / 8;
    int bitIdx = channel % 8;
    
    return (m_state[byteIdx] & (1 << bitIdx)) != 0;
}

void DigitalInputModel::getStateData(uint8_t *buffer) const
{
    if (buffer) {
        std::memcpy(buffer, m_state, sizeof(m_state));
    }
}

void DigitalInputModel::setStateData(const uint8_t *buffer)
{
    if (!buffer) {
        return;
    }
    
    bool changed = false;
    
    for (int i = 0; i < 8; i++) {
        if (m_state[i] != buffer[i]) {
            changed = true;
            
            // Emit individual channel changes
            for (int bit = 0; bit < 8; bit++) {
                bool oldState = (m_state[i] & (1 << bit)) != 0;
                bool newState = (buffer[i] & (1 << bit)) != 0;
                
                if (oldState != newState) {
                    emit channelChanged(i * 8 + bit, newState);
                }
            }
            
            m_state[i] = buffer[i];
        }
    }
    
    if (changed) {
        emit dataChanged();
    }
}

void DigitalInputModel::setPolling(bool polling)
{
    if (m_isPolling != polling) {
        m_isPolling = polling;
        emit pollingChanged();
    }
}
