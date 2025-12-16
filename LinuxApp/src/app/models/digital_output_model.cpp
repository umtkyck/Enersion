/**
 * @file digital_output_model.cpp
 * @brief Digital Output Data Model Implementation
 * @version 1.0.0
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#include "digital_output_model.h"
#include <cstring>

DigitalOutputModel::DigitalOutputModel(QObject *parent)
    : QObject(parent)
{
    std::memset(m_state, 0, sizeof(m_state));
}

int DigitalOutputModel::activeCount() const
{
    int count = 0;
    for (int i = 0; i < 64; i++) {
        if (getChannelState(i)) {
            count++;
        }
    }
    return count;
}

bool DigitalOutputModel::getChannelState(int channel) const
{
    if (channel < 0 || channel >= 64) {
        return false;
    }
    
    int byteIdx = channel / 8;
    int bitIdx = channel % 8;
    
    return (m_state[byteIdx] & (1 << bitIdx)) != 0;
}

void DigitalOutputModel::setChannelState(int channel, bool state)
{
    if (channel < 0 || channel >= 64) {
        return;
    }
    
    int byteIdx = channel / 8;
    int bitIdx = channel % 8;
    
    bool currentState = (m_state[byteIdx] & (1 << bitIdx)) != 0;
    
    if (currentState != state) {
        if (state) {
            m_state[byteIdx] |= (1 << bitIdx);
        } else {
            m_state[byteIdx] &= ~(1 << bitIdx);
        }
        
        emit channelChanged(channel, state);
        emit dataChanged();
    }
}

void DigitalOutputModel::setAllChannels(bool state)
{
    uint8_t value = state ? 0xFF : 0x00;
    
    bool changed = false;
    for (int i = 0; i < 8; i++) {
        if (m_state[i] != value) {
            m_state[i] = value;
            changed = true;
        }
    }
    
    if (changed) {
        emit dataChanged();
    }
}

void DigitalOutputModel::getStateData(uint8_t *buffer) const
{
    if (buffer) {
        std::memcpy(buffer, m_state, sizeof(m_state));
    }
}

void DigitalOutputModel::setStateData(const uint8_t *buffer)
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
