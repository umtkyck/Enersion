/**
 * @file digital_output_model.cpp
 * @brief Digital Output Model Implementation
 */

#include "digital_output_model.h"
#include "do_service.h"

DigitalOutputModel::DigitalOutputModel(DoService *service, QObject *parent)
    : QAbstractListModel(parent)
    , m_service(service)
{
    if (m_service) {
        connect(m_service, &DoService::outputStatesChanged,
                this, &DigitalOutputModel::refresh);
    }
}

int DigitalOutputModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return DoService::CHANNEL_COUNT;
}

QVariant DigitalOutputModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= DoService::CHANNEL_COUNT) {
        return QVariant();
    }
    
    int channel = index.row();
    
    switch (role) {
    case ChannelRole:
        return channel;
    case StateRole:
        return m_service ? m_service->getOutput(channel) : false;
    case NameRole:
        return QString("DO%1").arg(channel, 2, 10, QChar('0'));
    default:
        return QVariant();
    }
}

bool DigitalOutputModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= DoService::CHANNEL_COUNT || !m_service) {
        return false;
    }
    
    if (role == StateRole) {
        m_service->setOutput(index.row(), value.toBool());
        emit dataChanged(index, index, { role });
        return true;
    }
    
    return false;
}

Qt::ItemFlags DigitalOutputModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QHash<int, QByteArray> DigitalOutputModel::roleNames() const
{
    return {
        { ChannelRole, "channel" },
        { StateRole, "state" },
        { NameRole, "name" }
    };
}

void DigitalOutputModel::refresh()
{
    emit dataChanged(index(0), index(DoService::CHANNEL_COUNT - 1));
}

