/**
 * @file main.cpp
 * @brief Enersion Control System - Application Entry Point
 * @version 1.0.0
 * 
 * Qt/QML Application for STM32MP257 MYIR touchscreen.
 * Controls 64 Digital Inputs and 64 Digital Outputs via RS485.
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
#include "models/digital_input_model.h"
#include "models/digital_output_model.h"

int main(int argc, char *argv[])
{
    // Enable high DPI scaling
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    
    // Create application
    QGuiApplication app(argc, argv);
    
    // Application info
    app.setOrganizationName("Enersion");
    app.setOrganizationDomain("enersion.com");
    app.setApplicationName("Enersion Control System");
    app.setApplicationVersion("1.0.0");
    
    // Set default style
    QQuickStyle::setStyle("Basic");
    
    // Load custom fonts if available
    int fontId = QFontDatabase::addApplicationFont(":/fonts/Roboto-Regular.ttf");
    if (fontId != -1) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        if (!fontFamilies.isEmpty()) {
            QFont defaultFont(fontFamilies.first());
            defaultFont.setPixelSize(14);
            app.setFont(defaultFont);
        }
    }
    
    // Create main controller
    AppController appController;
    
    // Create QML engine
    QQmlApplicationEngine engine;
    
    // Register context properties
    QQmlContext *context = engine.rootContext();
    context->setContextProperty("appController", &appController);
    context->setContextProperty("diModel", appController.diModel());
    context->setContextProperty("doModel", appController.doModel());
    
    // Add QML import path
    engine.addImportPath("qrc:/");
    engine.addImportPath(":/ui/qml");
    
    // Load main QML file
    const QUrl url(QStringLiteral("qrc:/ui/qml/main.qml"));
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            qCritical() << "Failed to load QML";
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);
    
    engine.load(url);
    
    qInfo() << "Enersion Control System started";
    qInfo() << "Target: MYIR STM32MP257";
    qInfo() << "RS485: /dev/ttySTM9 @ 115200";
    
    return app.exec();
}
