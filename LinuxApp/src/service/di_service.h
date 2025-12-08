/**
 * @file di_service.h
 * @brief Digital Input Service - Business Logic Layer
 * @version 1.0.0
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#ifndef DI_SERVICE_H
#define DI_SERVICE_H

#include <QObject>
#include <QVector>
#include <QTimer>
#include "device_manager.h"

/**
 * @brief Digital Input Service - Manages digital input reading and monitoring
 */
class DiService : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(bool autoRefresh READ isAutoRefresh WRITE setAutoRefresh NOTIFY autoRefreshChanged)
    Q_PROPERTY(int refreshInterval READ refreshInterval WRITE setRefreshInterval NOTIFY refreshIntervalChanged)
    Q_PROPERTY(int activeCount READ activeCount NOTIFY activeCountChanged)
    Q_PROPERTY(QVector<bool> inputStates READ inputStates NOTIFY inputStatesChanged)
    
public:
    explicit DiService(DeviceManager *deviceManager, QObject *parent = nullptr);
    ~DiService() override;
    
    static constexpr int CHANNEL_COUNT = 64;
    
    // Property getters
    bool isAutoRefresh() const { return m_autoRefresh; }
    int refreshInterval() const { return m_refreshInterval; }
    int activeCount() const { return m_activeCount; }
    QVector<bool> inputStates() const { return m_inputStates; }
    
    // Property setters
    void setAutoRefresh(bool enable);
    void setRefreshInterval(int ms);
    
    // Input queries
    Q_INVOKABLE bool getInput(int channel) const;
    Q_INVOKABLE int getActiveInputCount() const;
    
public slots:
    bool readInputs();
    void startAutoRefresh();
    void stopAutoRefresh();
    
signals:
    void autoRefreshChanged(bool enabled);
    void refreshIntervalChanged(int interval);
    void activeCountChanged(int count);
    void inputStatesChanged();
    void inputChanged(int channel, bool state);
    void readSuccess(int activeCount);
    void readError(const QString &error);
    
private slots:
    void onRefreshTimeout();
    
private:
    DeviceManager *m_deviceManager = nullptr;
    QTimer *m_refreshTimer = nullptr;
    
    QVector<bool> m_inputStates;
    bool m_autoRefresh = false;
    int m_refreshInterval = 1000;  // 1 second default
    int m_activeCount = 0;
};

#endif // DI_SERVICE_H

