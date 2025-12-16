/**
 * @file connection_model.cpp
 * @brief Connection Model Implementation
 */

#include "connection_model.h"
#include <QSerialPortInfo>

ConnectionModel::ConnectionModel(QObject *parent)
    : QAbstractListModel(parent)
{
    refresh();
}

int ConnectionModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_ports.size();
}

QVariant ConnectionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_ports.size()) {
        return QVariant();
    }
    
    const PortInfo &port = m_ports[index.row()];
    
    switch (role) {
    case PortNameRole:
        return port.portName;
    case DescriptionRole:
        return port.description;
    case DisplayRole:
    case Qt::DisplayRole:
        return QString("%1 - %2").arg(port.portName, port.description);
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ConnectionModel::roleNames() const
{
    return {
        { PortNameRole, "portName" },
        { DescriptionRole, "description" },
        { DisplayRole, "display" }
    };
}

void ConnectionModel::refresh()
{
    beginResetModel();
    m_ports.clear();
    
    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto &info : ports) {
        m_ports.append({ info.portName(), info.description() });
    }
    
    // Add default embedded ports if empty
    if (m_ports.isEmpty()) {
        m_ports.append({ "/dev/ttySTM0", "STM32 UART" });
        m_ports.append({ "/dev/ttySTM1", "STM32 UART" });
        m_ports.append({ "/dev/ttyUSB0", "USB Serial" });
    }
    
    endResetModel();
}

QString ConnectionModel::getPortName(int index) const
{
    if (index >= 0 && index < m_ports.size()) {
        return m_ports[index].portName;
    }
    return QString();
}



