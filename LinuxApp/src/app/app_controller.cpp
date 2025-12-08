/**
 * @file app_controller.cpp
 * @brief Application Controller Implementation
 * @version 1.0.0
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#include "app_controller.h"
#include <QDebug>

AppController *AppController::s_instance = nullptr;

AppController::AppController(QObject *parent)
    : QObject(parent)
    , m_deviceManager(new DeviceManager(this))
    , m_diService(new DiService(m_deviceManager, this))
    , m_doService(new DoService(m_deviceManager, this))
    , m_statusMessage("Ready - Select COM port and connect")
{
    s_instance = this;
    setupConnections();
}

AppController::~AppController()
{
    s_instance = nullptr;
}

AppController *AppController::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    
    if (s_instance == nullptr) {
        s_instance = new AppController();
    }
    
    return s_instance;
}

AppController *AppController::instance()
{
    return s_instance;
}

QString AppController::appVersion() const
{
    return QStringLiteral("1.0.0");
}

void AppController::setStatusMessage(const QString &message)
{
    if (m_statusMessage != message) {
        m_statusMessage = message;
        emit statusMessageChanged(m_statusMessage);
        qDebug() << "Status:" << message;
    }
}

void AppController::setupConnections()
{
    // Device Manager signals
    QObject::connect(m_deviceManager, &DeviceManager::connectedChanged,
                     this, &AppController::onConnectionChanged);
    QObject::connect(m_deviceManager, &DeviceManager::connectionError,
                     this, &AppController::onConnectionError);
    
    // DI Service signals
    QObject::connect(m_diService, &DiService::readSuccess,
                     this, &AppController::onDiReadSuccess);
    QObject::connect(m_diService, &DiService::readError,
                     this, &AppController::onDiReadError);
    
    // DO Service signals
    QObject::connect(m_doService, &DoService::writeSuccess,
                     this, &AppController::onDoWriteSuccess);
    QObject::connect(m_doService, &DoService::writeError,
                     this, &AppController::onDoWriteError);
}

void AppController::onConnectionChanged(bool connected)
{
    if (connected) {
        setStatusMessage(QString("Connected to %1").arg(m_deviceManager->portName()));
    } else {
        setStatusMessage("Disconnected");
    }
}

void AppController::onConnectionError(const QString &error)
{
    setStatusMessage(QString("Connection error: %1").arg(error));
}

void AppController::onDiReadSuccess(int count)
{
    setStatusMessage(QString("DI read complete: %1 active inputs").arg(count));
}

void AppController::onDiReadError(const QString &error)
{
    setStatusMessage(QString("DI read error: %1").arg(error));
}

void AppController::onDoWriteSuccess()
{
    setStatusMessage(QString("DO write complete: %1 outputs active")
                    .arg(m_doService->activeCount()));
}

void AppController::onDoWriteError(const QString &error)
{
    setStatusMessage(QString("DO write error: %1").arg(error));
}

