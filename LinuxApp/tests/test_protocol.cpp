/**
 * @file test_protocol.cpp
 * @brief Unit Tests for Enersion Protocol Layer
 * @version 1.0.0
 * 
 * @copyright (c) 2024 Enersion. All rights reserved.
 */

#include <QtTest/QtTest>

extern "C" {
#include "enersion_protocol.h"
#include "enersion_crc.h"
#include "enersion_types.h"
}

class TestProtocol : public QObject
{
    Q_OBJECT
    
private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // CRC Tests
    void testCrc16_emptyData();
    void testCrc16_singleByte();
    void testCrc16_multipleBytes();
    void testCrc16_verify();
    void testCrc16_append();
    
    // Packet Encoding Tests
    void testEncodePacket_ping();
    void testEncodePacket_withData();
    void testEncodePacket_nullPointer();
    void testEncodePacket_bufferOverflow();
    
    // Packet Decoding Tests
    void testDecodePacket_valid();
    void testDecodePacket_invalidStart();
    void testDecodePacket_invalidEnd();
    void testDecodePacket_invalidCrc();
    void testDecodePacket_tooShort();
    
    // DIO State Tests
    void testDioState_getBit();
    void testDioState_setBit();
    void testDioState_clearAll();
    void testDioState_setAll();
    
    // Error String Tests
    void testErrorString_validCodes();
    void testErrorString_invalidCode();
};

void TestProtocol::initTestCase()
{
    qDebug() << "Starting Protocol Tests";
}

void TestProtocol::cleanupTestCase()
{
    qDebug() << "Protocol Tests Complete";
}

// ============================================================================
// CRC Tests
// ============================================================================

void TestProtocol::testCrc16_emptyData()
{
    uint16_t crc = enersion_crc16(nullptr, 0);
    QCOMPARE(crc, static_cast<uint16_t>(ENERSION_CRC_INIT));
}

void TestProtocol::testCrc16_singleByte()
{
    uint8_t data[] = { 0x01 };
    uint16_t crc = enersion_crc16(data, sizeof(data));
    
    // CRC should be non-zero and different from init
    QVERIFY(crc != ENERSION_CRC_INIT);
    QVERIFY(crc != 0);
}

void TestProtocol::testCrc16_multipleBytes()
{
    uint8_t data[] = { 0x01, 0x02, 0x03, 0x04 };
    uint16_t crc = enersion_crc16(data, sizeof(data));
    
    // Same data should produce same CRC
    uint16_t crc2 = enersion_crc16(data, sizeof(data));
    QCOMPARE(crc, crc2);
}

void TestProtocol::testCrc16_verify()
{
    uint8_t data[] = { 0x01, 0x02, 0x03, 0x04, 0x00, 0x00 };
    
    // Append CRC
    enersion_crc16_append(data, 4);
    
    // Verify should pass
    QVERIFY(enersion_crc16_verify(data, 6));
    
    // Corrupt data and verify should fail
    data[1] = 0xFF;
    QVERIFY(!enersion_crc16_verify(data, 6));
}

void TestProtocol::testCrc16_append()
{
    uint8_t data[10] = { 0x01, 0x10, 0x01, 0x00 };
    
    enersion_crc16_append(data, 4);
    
    // CRC bytes should be non-zero
    QVERIFY(data[4] != 0 || data[5] != 0);
}

// ============================================================================
// Packet Encoding Tests
// ============================================================================

void TestProtocol::testEncodePacket_ping()
{
    enersion_packet_t packet = {};
    packet.dest_addr = ENERSION_ADDR_CTRL_DIO;
    packet.src_addr = ENERSION_ADDR_MASTER;
    packet.command = ENERSION_CMD_PING;
    packet.data_len = 0;
    
    uint8_t buffer[ENERSION_MAX_PACKET_SIZE];
    size_t encoded_len = 0;
    
    enersion_error_t err = enersion_encode_packet(&packet, buffer, sizeof(buffer), &encoded_len);
    
    QCOMPARE(err, ENERSION_OK);
    QCOMPARE(encoded_len, static_cast<size_t>(8)); // Start + 4 header + 2 CRC + End
    QCOMPARE(buffer[0], static_cast<uint8_t>(ENERSION_START_BYTE));
    QCOMPARE(buffer[encoded_len - 1], static_cast<uint8_t>(ENERSION_END_BYTE));
}

void TestProtocol::testEncodePacket_withData()
{
    enersion_packet_t packet = {};
    packet.dest_addr = ENERSION_ADDR_CTRL_OUT;
    packet.src_addr = ENERSION_ADDR_MASTER;
    packet.command = ENERSION_CMD_WRITE_DO;
    packet.data_len = 8;
    
    for (int i = 0; i < 8; i++) {
        packet.data[i] = 0xFF;
    }
    
    uint8_t buffer[ENERSION_MAX_PACKET_SIZE];
    size_t encoded_len = 0;
    
    enersion_error_t err = enersion_encode_packet(&packet, buffer, sizeof(buffer), &encoded_len);
    
    QCOMPARE(err, ENERSION_OK);
    QCOMPARE(encoded_len, static_cast<size_t>(16)); // Start + 4 header + 8 data + 2 CRC + End
}

void TestProtocol::testEncodePacket_nullPointer()
{
    uint8_t buffer[ENERSION_MAX_PACKET_SIZE];
    size_t encoded_len = 0;
    
    enersion_error_t err = enersion_encode_packet(nullptr, buffer, sizeof(buffer), &encoded_len);
    QCOMPARE(err, ENERSION_ERR_NULL_POINTER);
}

void TestProtocol::testEncodePacket_bufferOverflow()
{
    enersion_packet_t packet = {};
    packet.data_len = 250;
    
    uint8_t buffer[10]; // Too small
    size_t encoded_len = 0;
    
    enersion_error_t err = enersion_encode_packet(&packet, buffer, sizeof(buffer), &encoded_len);
    QCOMPARE(err, ENERSION_ERR_BUFFER_OVERFLOW);
}

// ============================================================================
// Packet Decoding Tests
// ============================================================================

void TestProtocol::testDecodePacket_valid()
{
    // Build a valid packet
    uint8_t buffer[] = {
        0xAA,       // Start
        0x10,       // Dest (Master)
        0x02,       // Src (DIO)
        0x02,       // Cmd (PING_RESPONSE)
        0x00,       // Len
        0x00, 0x00, // CRC (will be calculated)
        0x55        // End
    };
    
    // Calculate and insert CRC
    uint16_t crc = enersion_crc16(&buffer[1], 4);
    buffer[5] = crc & 0xFF;
    buffer[6] = (crc >> 8) & 0xFF;
    
    enersion_packet_t packet = {};
    enersion_error_t err = enersion_decode_packet(buffer, sizeof(buffer), &packet);
    
    QCOMPARE(err, ENERSION_OK);
    QCOMPARE(packet.dest_addr, static_cast<uint8_t>(0x10));
    QCOMPARE(packet.src_addr, static_cast<uint8_t>(0x02));
    QCOMPARE(packet.command, static_cast<uint8_t>(0x02));
    QCOMPARE(packet.data_len, static_cast<uint8_t>(0));
}

void TestProtocol::testDecodePacket_invalidStart()
{
    uint8_t buffer[] = { 0x00, 0x10, 0x02, 0x02, 0x00, 0x00, 0x00, 0x55 };
    
    enersion_packet_t packet = {};
    enersion_error_t err = enersion_decode_packet(buffer, sizeof(buffer), &packet);
    
    QCOMPARE(err, ENERSION_ERR_INVALID_PACKET);
}

void TestProtocol::testDecodePacket_invalidEnd()
{
    uint8_t buffer[] = { 0xAA, 0x10, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00 };
    
    enersion_packet_t packet = {};
    enersion_error_t err = enersion_decode_packet(buffer, sizeof(buffer), &packet);
    
    QCOMPARE(err, ENERSION_ERR_INVALID_PACKET);
}

void TestProtocol::testDecodePacket_invalidCrc()
{
    uint8_t buffer[] = { 0xAA, 0x10, 0x02, 0x02, 0x00, 0xFF, 0xFF, 0x55 };
    
    enersion_packet_t packet = {};
    enersion_error_t err = enersion_decode_packet(buffer, sizeof(buffer), &packet);
    
    QCOMPARE(err, ENERSION_ERR_INVALID_CRC);
}

void TestProtocol::testDecodePacket_tooShort()
{
    uint8_t buffer[] = { 0xAA, 0x10, 0x02 };
    
    enersion_packet_t packet = {};
    enersion_error_t err = enersion_decode_packet(buffer, sizeof(buffer), &packet);
    
    QCOMPARE(err, ENERSION_ERR_INVALID_PACKET);
}

// ============================================================================
// DIO State Tests
// ============================================================================

void TestProtocol::testDioState_getBit()
{
    enersion_dio_state_t state = {};
    state.state[0] = 0x05; // Bits 0 and 2 set
    
    QVERIFY(enersion_dio_get_bit(&state, 0));
    QVERIFY(!enersion_dio_get_bit(&state, 1));
    QVERIFY(enersion_dio_get_bit(&state, 2));
    QVERIFY(!enersion_dio_get_bit(&state, 3));
    
    // Out of range
    QVERIFY(!enersion_dio_get_bit(&state, 64));
    QVERIFY(!enersion_dio_get_bit(&state, 255));
}

void TestProtocol::testDioState_setBit()
{
    enersion_dio_state_t state = {};
    
    enersion_dio_set_bit(&state, 0, true);
    enersion_dio_set_bit(&state, 7, true);
    enersion_dio_set_bit(&state, 63, true);
    
    QVERIFY(enersion_dio_get_bit(&state, 0));
    QVERIFY(enersion_dio_get_bit(&state, 7));
    QVERIFY(enersion_dio_get_bit(&state, 63));
    
    enersion_dio_set_bit(&state, 0, false);
    QVERIFY(!enersion_dio_get_bit(&state, 0));
}

void TestProtocol::testDioState_clearAll()
{
    enersion_dio_state_t state;
    
    // Set all bits
    for (int i = 0; i < 8; i++) {
        state.state[i] = 0xFF;
    }
    
    enersion_dio_clear_all(&state);
    
    for (int i = 0; i < 64; i++) {
        QVERIFY(!enersion_dio_get_bit(&state, i));
    }
}

void TestProtocol::testDioState_setAll()
{
    enersion_dio_state_t state = {};
    
    enersion_dio_set_all(&state);
    
    for (int i = 0; i < 64; i++) {
        QVERIFY(enersion_dio_get_bit(&state, i));
    }
}

// ============================================================================
// Error String Tests
// ============================================================================

void TestProtocol::testErrorString_validCodes()
{
    QVERIFY(strlen(enersion_error_string(ENERSION_OK)) > 0);
    QVERIFY(strlen(enersion_error_string(ENERSION_ERR_NULL_POINTER)) > 0);
    QVERIFY(strlen(enersion_error_string(ENERSION_ERR_INVALID_CRC)) > 0);
    QVERIFY(strlen(enersion_error_string(ENERSION_ERR_TIMEOUT)) > 0);
}

void TestProtocol::testErrorString_invalidCode()
{
    const char *str = enersion_error_string(static_cast<enersion_error_t>(-100));
    QVERIFY(str != nullptr);
    QVERIFY(strlen(str) > 0);
}

QTEST_MAIN(TestProtocol)
#include "test_protocol.moc"

