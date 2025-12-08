/**
 * @file main.cpp
 * @brief Enersion GUI Application Entry Point
 * @version 1.0.0
 * 
 * Target: STM32MP257 MYIR Board with HDMI Touchscreen
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QFont>
#include <QFontDatabase>
#include <QDebug>

#include "app_controller.h"
#include "device_manager.h"
#include "di_service.h"
#include "do_service.h"

int main(int argc, char *argv[])
{
    // Enable high DPI scaling for touchscreen
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    
    QGuiApplication app(argc, argv);
    
    // Application metadata
    app.setOrganizationName("Enersion");
    app.setOrganizationDomain("enersion.com");
    app.setApplicationName("Enersion Controller");
    app.setApplicationVersion("1.0.0");
    
    // Set Qt Quick Controls style
    QQuickStyle::setStyle("Material");
    
    // Load custom fonts
    int fontId = QFontDatabase::addApplicationFont(":/fonts/Inter-Regular.ttf");
    if (fontId >= 0) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        if (!fontFamilies.isEmpty()) {
            QFont defaultFont(fontFamilies.first());
            defaultFont.setPointSize(12);
            app.setFont(defaultFont);
        }
    }
    
    // Register QML types
    qmlRegisterSingletonType<AppController>("EnersionApp", 1, 0, "AppController",
        &AppController::create);
    qmlRegisterUncreatableType<DeviceManager>("EnersionApp", 1, 0, "DeviceManager",
        "Access through AppController.deviceManager");
    qmlRegisterUncreatableType<DiService>("EnersionApp", 1, 0, "DiService",
        "Access through AppController.diService");
    qmlRegisterUncreatableType<DoService>("EnersionApp", 1, 0, "DoService",
        "Access through AppController.doService");
    
    // Create QML engine
    QQmlApplicationEngine engine;
    
    // Load main QML file
    const QUrl url(QStringLiteral("qrc:/EnersionApp/qml/main.qml"));
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);
    
    engine.load(url);
    
    if (engine.rootObjects().isEmpty()) {
        qCritical() << "Failed to load QML";
        return -1;
    }
    
    qDebug() << "Enersion Controller started";
    qDebug() << "Version:" << app.applicationVersion();
    
    return app.exec();
}

