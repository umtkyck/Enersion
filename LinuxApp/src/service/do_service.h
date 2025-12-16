/**
 * @file do_service.h
 * @brief Digital Output Service - Business Logic Layer
 * @version 1.0.0
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#ifndef DO_SERVICE_H
#define DO_SERVICE_H

#include <QObject>
#include <QVector>
#include "device_manager.h"

/**
 * @brief Digital Output Service - Manages digital output control
 */
class DoService : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(int activeCount READ activeCount NOTIFY activeCountChanged)
    Q_PROPERTY(QVector<bool> outputStates READ outputStates NOTIFY outputStatesChanged)
    Q_PROPERTY(bool pendingChanges READ hasPendingChanges NOTIFY pendingChangesChanged)
    
public:
    explicit DoService(DeviceManager *deviceManager, QObject *parent = nullptr);
    ~DoService() override;
    
    static constexpr int CHANNEL_COUNT = 64;
    
    // Property getters
    int activeCount() const { return m_activeCount; }
    QVector<bool> outputStates() const { return m_outputStates; }
    bool hasPendingChanges() const { return m_pendingChanges; }
    
    // Output queries
    Q_INVOKABLE bool getOutput(int channel) const;
    Q_INVOKABLE int getActiveOutputCount() const;
    
    // Output control
    Q_INVOKABLE void setOutput(int channel, bool state);
    Q_INVOKABLE void toggleOutput(int channel);
    Q_INVOKABLE void setAllOutputs(bool state);
    Q_INVOKABLE void clearAllOutputs();
    
public slots:
    bool writeOutputs();
    bool readOutputs();
    
signals:
    void activeCountChanged(int count);
    void outputStatesChanged();
    void pendingChangesChanged(bool pending);
    void outputChanged(int channel, bool state);
    void writeSuccess();
    void writeError(const QString &error);
    void readSuccess(int activeCount);
    void readError(const QString &error);
    
private:
    void updateActiveCount();
    void markPendingChanges();
    
    DeviceManager *m_deviceManager = nullptr;
    
    QVector<bool> m_outputStates;
    QVector<bool> m_deviceStates;  // Last known device state
    bool m_pendingChanges = false;
    int m_activeCount = 0;
};

#endif // DO_SERVICE_H



