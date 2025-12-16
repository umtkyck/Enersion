/**
 * @file enersion_test.c
 * @brief Enersion DI/DO Controller Test Application
 * @target MYIR STM32MP257 Board
 * 
 * Tests RS485 communication with:
 * - Controller DIO (0x02) - 64 Digital Inputs
 * - Controller OUT (0x03) - 64 Digital Outputs
 * 
 * Compile on board:
 *   gcc -o enersion_test enersion_test.c ../hal/rs485_serial.c ../hal/rs485_gpio.c \
 *       ../protocol/enersion_crc.c ../protocol/enersion_protocol.c -I../hal -I../protocol
 * 
 * Run:
 *   ./enersion_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "rs485_serial.h"
#include "rs485_gpio.h"
#include "enersion_protocol.h"
#include "enersion_types.h"

/* ============================================================================
 * Configuration
 * ============================================================================ */

#define RS485_DEVICE        "/dev/ttySTM9"
#define RS485_BAUDRATE      115200

/* ============================================================================
 * Global Variables
 * ============================================================================ */

static volatile int g_running = 1;
static rs485_handle_t g_rs485 = NULL;
static enersion_handle_t g_protocol = NULL;

/* ============================================================================
 * Signal Handler
 * ============================================================================ */

static void signal_handler(int sig)
{
    (void)sig;
    printf("\nShutting down...\n");
    g_running = 0;
}

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

static void print_dio_state(const char *name, const enersion_dio_state_t *state)
{
    printf("%s State:\n", name);
    printf("  ");
    
    for (int i = 0; i < 64; i++) {
        if (i > 0 && i % 8 == 0) {
            printf(" ");
        }
        printf("%c", enersion_dio_get_bit(state, (uint8_t)i) ? '1' : '0');
    }
    printf("\n");
    
    /* Count active */
    int active = 0;
    for (int i = 0; i < 64; i++) {
        if (enersion_dio_get_bit(state, (uint8_t)i)) {
            active++;
        }
    }
    printf("  Active: %d / 64\n", active);
}

static void print_version(const char *name, const enersion_version_t *ver)
{
    printf("%s Version: v%d.%d.%d.%d (MCU ID: 0x%02X)\n",
           name, ver->major, ver->minor, ver->patch, ver->build, ver->mcu_id);
}

static void print_status(const char *name, const enersion_status_t *status)
{
    printf("%s Status:\n", name);
    printf("  MCU ID: 0x%02X\n", status->mcu_id);
    printf("  Health: %d%%\n", status->health);
    printf("  Uptime: %u seconds\n", status->uptime);
    printf("  RX Packets: %u\n", status->rx_packet_count);
    printf("  TX Packets: %u\n", status->tx_packet_count);
    printf("  Errors: %u\n", status->error_count);
}

/* ============================================================================
 * Test Functions
 * ============================================================================ */

static int test_ping(enersion_addr_t addr, const char *name)
{
    bool online = false;
    enersion_error_t err = enersion_ping(g_protocol, addr, &online);
    
    if (err == ENERSION_OK && online) {
        printf("[OK] %s (0x%02X) is ONLINE\n", name, addr);
        return 0;
    } else {
        printf("[FAIL] %s (0x%02X) is OFFLINE\n", name, addr);
        return -1;
    }
}

static int test_version(enersion_addr_t addr, const char *name)
{
    enersion_version_t version = {0};
    enersion_error_t err = enersion_get_version(g_protocol, addr, &version);
    
    if (err == ENERSION_OK) {
        print_version(name, &version);
        return 0;
    } else {
        printf("[FAIL] %s get version: %s\n", name, enersion_error_string(err));
        return -1;
    }
}

static int test_status(enersion_addr_t addr, const char *name)
{
    enersion_status_t status = {0};
    enersion_error_t err = enersion_get_status(g_protocol, addr, &status);
    
    if (err == ENERSION_OK) {
        print_status(name, &status);
        return 0;
    } else {
        printf("[FAIL] %s get status: %s\n", name, enersion_error_string(err));
        return -1;
    }
}

static int test_read_di(void)
{
    enersion_dio_state_t state = {0};
    enersion_error_t err = enersion_read_digital_inputs(g_protocol, &state);
    
    if (err == ENERSION_OK) {
        print_dio_state("Digital Inputs", &state);
        return 0;
    } else {
        printf("[FAIL] Read DI: %s\n", enersion_error_string(err));
        return -1;
    }
}

static int test_read_do(void)
{
    enersion_dio_state_t state = {0};
    enersion_error_t err = enersion_read_digital_outputs(g_protocol, &state);
    
    if (err == ENERSION_OK) {
        print_dio_state("Digital Outputs", &state);
        return 0;
    } else {
        printf("[FAIL] Read DO: %s\n", enersion_error_string(err));
        return -1;
    }
}

static int test_write_do(uint8_t pattern)
{
    enersion_dio_state_t state = {0};
    
    /* Set pattern */
    for (int i = 0; i < 8; i++) {
        state.state[i] = pattern;
    }
    
    printf("Writing pattern 0x%02X to all outputs...\n", pattern);
    
    enersion_error_t err = enersion_write_digital_outputs(g_protocol, &state);
    
    if (err == ENERSION_OK) {
        printf("[OK] Write DO successful\n");
        return 0;
    } else {
        printf("[FAIL] Write DO: %s\n", enersion_error_string(err));
        return -1;
    }
}

/* ============================================================================
 * Main Menu
 * ============================================================================ */

static void print_menu(void)
{
    printf("\n");
    printf("========================================\n");
    printf(" Enersion Controller Test Menu\n");
    printf("========================================\n");
    printf("  1. Scan devices (ping all)\n");
    printf("  2. Get DI Controller info\n");
    printf("  3. Get DO Controller info\n");
    printf("  4. Read Digital Inputs\n");
    printf("  5. Read Digital Outputs\n");
    printf("  6. Write DO - All OFF (0x00)\n");
    printf("  7. Write DO - All ON (0xFF)\n");
    printf("  8. Write DO - Pattern (0xAA)\n");
    printf("  9. Write DO - Pattern (0x55)\n");
    printf("  m. Monitor DI (continuous)\n");
    printf("  q. Quit\n");
    printf("========================================\n");
    printf("Select: ");
}

static void monitor_di(void)
{
    printf("\nMonitoring Digital Inputs (Press Ctrl+C to stop)...\n\n");
    
    while (g_running) {
        enersion_dio_state_t state = {0};
        enersion_error_t err = enersion_read_digital_inputs(g_protocol, &state);
        
        if (err == ENERSION_OK) {
            /* Clear line and print */
            printf("\r");
            for (int i = 0; i < 64; i++) {
                if (i > 0 && i % 8 == 0) printf(" ");
                printf("%c", enersion_dio_get_bit(&state, (uint8_t)i) ? '1' : '0');
            }
            fflush(stdout);
        }
        
        usleep(500000);  /* 500ms */
    }
    printf("\n");
}

/* ============================================================================
 * Main
 * ============================================================================ */

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    
    int result = 0;
    
    printf("\n");
    printf("=============================================\n");
    printf(" Enersion DI/DO Controller Test\n");
    printf(" Target: MYIR STM32MP257\n");
    printf("=============================================\n");
    printf("\n");
    
    /* Setup signal handler */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    /* Initialize RS485 */
    printf("Initializing RS485 on %s @ %d baud...\n", RS485_DEVICE, RS485_BAUDRATE);
    
    rs485_config_t rs485_config = {
        .device = RS485_DEVICE,
        .baudrate = RS485_BAUDRATE,
        .data_bits = 8,
        .parity = RS485_PARITY_NONE,
        .stop_bits = 1,
        .rs485_mode = true,
        .timeout_ms = 500
    };
    
    rs485_error_t rs_err = rs485_create(&rs485_config, &g_rs485);
    if (rs_err != RS485_OK) {
        printf("ERROR: Failed to create RS485: %s\n", rs485_error_string(rs_err));
        return 1;
    }
    
    rs_err = rs485_open(g_rs485);
    if (rs_err != RS485_OK) {
        printf("ERROR: Failed to open RS485: %s\n", rs485_error_string(rs_err));
        rs485_destroy(&g_rs485);
        return 1;
    }
    
    printf("[OK] RS485 initialized\n");
    
    /* Initialize Protocol */
    printf("Initializing Enersion protocol...\n");
    
    enersion_error_t en_err = enersion_create(g_rs485, &g_protocol);
    if (en_err != ENERSION_OK) {
        printf("ERROR: Failed to create protocol: %s\n", enersion_error_string(en_err));
        rs485_close(g_rs485);
        rs485_destroy(&g_rs485);
        return 1;
    }
    
    printf("[OK] Protocol initialized\n\n");
    
    /* Initial device scan */
    printf("Scanning for devices...\n");
    test_ping(ENERSION_ADDR_CTRL_DIO, "Controller DIO");
    test_ping(ENERSION_ADDR_CTRL_OUT, "Controller OUT");
    
    /* Main loop */
    char input[16];
    
    while (g_running) {
        print_menu();
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        printf("\n");
        
        switch (input[0]) {
            case '1':
                printf("Scanning devices...\n");
                test_ping(ENERSION_ADDR_CTRL_DIO, "Controller DIO");
                test_ping(ENERSION_ADDR_CTRL_OUT, "Controller OUT");
                break;
                
            case '2':
                printf("DI Controller Info:\n");
                test_ping(ENERSION_ADDR_CTRL_DIO, "Controller DIO");
                test_version(ENERSION_ADDR_CTRL_DIO, "Controller DIO");
                test_status(ENERSION_ADDR_CTRL_DIO, "Controller DIO");
                break;
                
            case '3':
                printf("DO Controller Info:\n");
                test_ping(ENERSION_ADDR_CTRL_OUT, "Controller OUT");
                test_version(ENERSION_ADDR_CTRL_OUT, "Controller OUT");
                test_status(ENERSION_ADDR_CTRL_OUT, "Controller OUT");
                break;
                
            case '4':
                test_read_di();
                break;
                
            case '5':
                test_read_do();
                break;
                
            case '6':
                test_write_do(0x00);
                break;
                
            case '7':
                test_write_do(0xFF);
                break;
                
            case '8':
                test_write_do(0xAA);
                break;
                
            case '9':
                test_write_do(0x55);
                break;
                
            case 'm':
            case 'M':
                monitor_di();
                break;
                
            case 'q':
            case 'Q':
                g_running = 0;
                break;
                
            default:
                printf("Invalid option\n");
                break;
        }
    }
    
    /* Cleanup */
    printf("\nCleaning up...\n");
    
    enersion_destroy(&g_protocol);
    rs485_close(g_rs485);
    rs485_destroy(&g_rs485);
    
    printf("Done.\n");
    
    return result;
}



