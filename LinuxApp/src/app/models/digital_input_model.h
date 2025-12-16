/**
 * @file digital_input_model.h
 * @brief Digital Input Data Model for QML
 * @version 1.0.0
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#ifndef DIGITAL_INPUT_MODEL_H
#define DIGITAL_INPUT_MODEL_H

#include <QObject>
#include <QVector>
#include <cstdint>

/**
 * @brief Digital Input Model
 * 
 * Manages state of 64 digital input channels for QML binding.
 */
class DigitalInputModel : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(int activeCount READ activeCount NOTIFY dataChanged)
    Q_PROPERTY(bool isPolling READ isPolling NOTIFY pollingChanged)
    
public:
    explicit DigitalInputModel(QObject *parent = nullptr);
    
    // Property getters
    int activeCount() const;
    bool isPolling() const;
    
    // Channel access
    Q_INVOKABLE bool getChannelState(int channel) const;
    
    // Bulk data access
    void getStateData(uint8_t *buffer) const;
    void setStateData(const uint8_t *buffer);
    
    // Polling state
    void setPolling(bool polling);
    
signals:
    void dataChanged();
    void pollingChanged();
    void channelChanged(int channel, bool state);
    
private:
    uint8_t m_state[8];  // 64 bits = 8 bytes
    bool m_isPolling;
};

#endif // DIGITAL_INPUT_MODEL_H
