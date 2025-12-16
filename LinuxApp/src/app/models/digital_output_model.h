/**
 * @file digital_output_model.h
 * @brief Digital Output Data Model for QML
 * @version 1.0.0
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#ifndef DIGITAL_OUTPUT_MODEL_H
#define DIGITAL_OUTPUT_MODEL_H

#include <QObject>
#include <cstdint>

/**
 * @brief Digital Output Model
 * 
 * Manages state of 64 digital output channels for QML binding.
 */
class DigitalOutputModel : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(int activeCount READ activeCount NOTIFY dataChanged)
    
public:
    explicit DigitalOutputModel(QObject *parent = nullptr);
    
    // Property getters
    int activeCount() const;
    
    // Channel access
    Q_INVOKABLE bool getChannelState(int channel) const;
    void setChannelState(int channel, bool state);
    
    // Bulk operations
    void setAllChannels(bool state);
    
    // Bulk data access
    void getStateData(uint8_t *buffer) const;
    void setStateData(const uint8_t *buffer);
    
signals:
    void dataChanged();
    void channelChanged(int channel, bool state);
    
private:
    uint8_t m_state[8];  // 64 bits = 8 bytes
};

#endif // DIGITAL_OUTPUT_MODEL_H
