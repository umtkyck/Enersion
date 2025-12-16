/**
 * @file test_service.cpp
 * @brief Unit Tests for Service Layer
 * @version 1.0.0
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#include <QtTest/QtTest>
#include <QSignalSpy>
#include "di_service.h"
#include "do_service.h"

class TestService : public QObject
{
    Q_OBJECT
    
private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // DiService Tests
    void testDiService_initialState();
    void testDiService_autoRefresh();
    void testDiService_refreshInterval();
    
    // DoService Tests
    void testDoService_initialState();
    void testDoService_setOutput();
    void testDoService_toggleOutput();
    void testDoService_setAllOutputs();
    void testDoService_clearAllOutputs();
    void testDoService_pendingChanges();
    
private:
    DiService *m_diService = nullptr;
    DoService *m_doService = nullptr;
};

void TestService::initTestCase()
{
    qDebug() << "Starting Service Tests";
    
    // Create services without device manager (offline mode)
    m_diService = new DiService(nullptr);
    m_doService = new DoService(nullptr);
}

void TestService::cleanupTestCase()
{
    delete m_diService;
    delete m_doService;
    
    qDebug() << "Service Tests Complete";
}

// ============================================================================
// DiService Tests
// ============================================================================

void TestService::testDiService_initialState()
{
    QCOMPARE(m_diService->isAutoRefresh(), false);
    QCOMPARE(m_diService->activeCount(), 0);
    QCOMPARE(m_diService->inputStates().size(), DiService::CHANNEL_COUNT);
    
    // All inputs should be false initially
    for (int i = 0; i < DiService::CHANNEL_COUNT; i++) {
        QCOMPARE(m_diService->getInput(i), false);
    }
}

void TestService::testDiService_autoRefresh()
{
    QSignalSpy spy(m_diService, &DiService::autoRefreshChanged);
    
    QCOMPARE(m_diService->isAutoRefresh(), false);
    
    m_diService->setAutoRefresh(true);
    QCOMPARE(m_diService->isAutoRefresh(), true);
    QCOMPARE(spy.count(), 1);
    
    m_diService->setAutoRefresh(false);
    QCOMPARE(m_diService->isAutoRefresh(), false);
    QCOMPARE(spy.count(), 2);
    
    // Setting same value should not emit signal
    m_diService->setAutoRefresh(false);
    QCOMPARE(spy.count(), 2);
}

void TestService::testDiService_refreshInterval()
{
    QSignalSpy spy(m_diService, &DiService::refreshIntervalChanged);
    
    m_diService->setRefreshInterval(2000);
    QCOMPARE(m_diService->refreshInterval(), 2000);
    QCOMPARE(spy.count(), 1);
    
    // Invalid interval (too low) should be ignored
    m_diService->setRefreshInterval(50);
    QCOMPARE(m_diService->refreshInterval(), 2000);
    QCOMPARE(spy.count(), 1);
    
    // Reset to default
    m_diService->setRefreshInterval(1000);
}

// ============================================================================
// DoService Tests
// ============================================================================

void TestService::testDoService_initialState()
{
    QCOMPARE(m_doService->activeCount(), 0);
    QCOMPARE(m_doService->hasPendingChanges(), false);
    QCOMPARE(m_doService->outputStates().size(), DoService::CHANNEL_COUNT);
    
    // All outputs should be false initially
    for (int i = 0; i < DoService::CHANNEL_COUNT; i++) {
        QCOMPARE(m_doService->getOutput(i), false);
    }
}

void TestService::testDoService_setOutput()
{
    QSignalSpy outputSpy(m_doService, &DoService::outputChanged);
    QSignalSpy stateSpy(m_doService, &DoService::outputStatesChanged);
    
    m_doService->setOutput(0, true);
    
    QCOMPARE(m_doService->getOutput(0), true);
    QCOMPARE(outputSpy.count(), 1);
    QCOMPARE(stateSpy.count(), 1);
    
    // Verify signal parameters
    QCOMPARE(outputSpy.first().at(0).toInt(), 0);
    QCOMPARE(outputSpy.first().at(1).toBool(), true);
    
    // Set same value should not emit
    m_doService->setOutput(0, true);
    QCOMPARE(outputSpy.count(), 1);
    
    // Reset
    m_doService->setOutput(0, false);
}

void TestService::testDoService_toggleOutput()
{
    QCOMPARE(m_doService->getOutput(5), false);
    
    m_doService->toggleOutput(5);
    QCOMPARE(m_doService->getOutput(5), true);
    
    m_doService->toggleOutput(5);
    QCOMPARE(m_doService->getOutput(5), false);
}

void TestService::testDoService_setAllOutputs()
{
    QSignalSpy spy(m_doService, &DoService::activeCountChanged);
    
    m_doService->setAllOutputs(true);
    
    QCOMPARE(m_doService->activeCount(), DoService::CHANNEL_COUNT);
    
    for (int i = 0; i < DoService::CHANNEL_COUNT; i++) {
        QCOMPARE(m_doService->getOutput(i), true);
    }
    
    // Reset
    m_doService->clearAllOutputs();
}

void TestService::testDoService_clearAllOutputs()
{
    // First set some outputs
    m_doService->setOutput(0, true);
    m_doService->setOutput(10, true);
    m_doService->setOutput(63, true);
    
    QVERIFY(m_doService->activeCount() > 0);
    
    m_doService->clearAllOutputs();
    
    QCOMPARE(m_doService->activeCount(), 0);
    
    for (int i = 0; i < DoService::CHANNEL_COUNT; i++) {
        QCOMPARE(m_doService->getOutput(i), false);
    }
}

void TestService::testDoService_pendingChanges()
{
    QSignalSpy spy(m_doService, &DoService::pendingChangesChanged);
    
    // Initially no pending changes
    QCOMPARE(m_doService->hasPendingChanges(), false);
    
    // Make a change
    m_doService->setOutput(0, true);
    
    // Should have pending changes
    QCOMPARE(m_doService->hasPendingChanges(), true);
    QVERIFY(spy.count() >= 1);
    
    // Reset
    m_doService->clearAllOutputs();
}

QTEST_MAIN(TestService)
#include "test_service.moc"



