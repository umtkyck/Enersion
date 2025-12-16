"""
Enersion Protocol Module

Custom protocol for DI/DO controller communication.
"""

from .enersion import (
    EnersionProtocol,
    Packet,
    Address,
    Command,
    Version,
    Status,
    DIOState,
    crc16,
    verify_crc,
    START_BYTE,
    END_BYTE,
    DIO_CHANNEL_COUNT,
    DIO_BYTE_COUNT
)

__all__ = [
    'EnersionProtocol',
    'Packet',
    'Address',
    'Command',
    'Version',
    'Status',
    'DIOState',
    'crc16',
    'verify_crc',
    'START_BYTE',
    'END_BYTE',
    'DIO_CHANNEL_COUNT',
    'DIO_BYTE_COUNT'
]

