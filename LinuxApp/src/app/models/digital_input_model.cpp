/**
 * @file digital_input_model.cpp
 * @brief Digital Input Model Implementation
 */

#include "digital_input_model.h"
#include "di_service.h"

DigitalInputModel::DigitalInputModel(DiService *service, QObject *parent)
    : QAbstractListModel(parent)
    , m_service(service)
{
    if (m_service) {
        connect(m_service, &DiService::inputStatesChanged,
                this, &DigitalInputModel::refresh);
    }
}

int DigitalInputModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return DiService::CHANNEL_COUNT;
}

QVariant DigitalInputModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= DiService::CHANNEL_COUNT) {
        return QVariant();
    }
    
    int channel = index.row();
    
    switch (role) {
    case ChannelRole:
        return channel;
    case StateRole:
        return m_service ? m_service->getInput(channel) : false;
    case NameRole:
        return QString("DI%1").arg(channel, 2, 10, QChar('0'));
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> DigitalInputModel::roleNames() const
{
    return {
        { ChannelRole, "channel" },
        { StateRole, "state" },
        { NameRole, "name" }
    };
}

void DigitalInputModel::refresh()
{
    emit dataChanged(index(0), index(DiService::CHANNEL_COUNT - 1));
}

