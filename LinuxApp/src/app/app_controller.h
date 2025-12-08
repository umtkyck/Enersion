/**
 * @file app_controller.h
 * @brief Application Controller - Main Application Logic
 * @version 1.0.0
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include <QObject>
#include <QQmlEngine>
#include "device_manager.h"
#include "di_service.h"
#include "do_service.h"

/**
 * @brief Application Controller - Coordinates services and exposes to QML
 */
class AppController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    
    Q_PROPERTY(DeviceManager* deviceManager READ deviceManager CONSTANT)
    Q_PROPERTY(DiService* diService READ diService CONSTANT)
    Q_PROPERTY(DoService* doService READ doService CONSTANT)
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    
public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController() override;
    
    // Singleton access for QML
    static AppController *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    static AppController *instance();
    
    // Property getters
    DeviceManager *deviceManager() const { return m_deviceManager; }
    DiService *diService() const { return m_diService; }
    DoService *doService() const { return m_doService; }
    QString appVersion() const;
    QString statusMessage() const { return m_statusMessage; }
    
public slots:
    void setStatusMessage(const QString &message);
    
signals:
    void statusMessageChanged(const QString &message);
    
private slots:
    void onConnectionChanged(bool connected);
    void onConnectionError(const QString &error);
    void onDiReadSuccess(int count);
    void onDiReadError(const QString &error);
    void onDoWriteSuccess();
    void onDoWriteError(const QString &error);
    
private:
    void setupConnections();
    
    static AppController *s_instance;
    
    DeviceManager *m_deviceManager = nullptr;
    DiService *m_diService = nullptr;
    DoService *m_doService = nullptr;
    QString m_statusMessage;
};

#endif // APP_CONTROLLER_H

