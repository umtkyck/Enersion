/**
 * @file connection_model.h
 * @brief Connection Model for available serial ports
 * @version 1.0.0
 */

#ifndef CONNECTION_MODEL_H
#define CONNECTION_MODEL_H

#include <QAbstractListModel>
#include <QStringList>

class ConnectionModel : public QAbstractListModel
{
    Q_OBJECT
    
public:
    enum Roles {
        PortNameRole = Qt::UserRole + 1,
        DescriptionRole,
        DisplayRole
    };
    
    explicit ConnectionModel(QObject *parent = nullptr);
    
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    
public slots:
    void refresh();
    QString getPortName(int index) const;
    
private:
    struct PortInfo {
        QString portName;
        QString description;
    };
    
    QVector<PortInfo> m_ports;
};

#endif // CONNECTION_MODEL_H



