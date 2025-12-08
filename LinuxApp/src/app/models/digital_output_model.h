/**
 * @file digital_output_model.h
 * @brief Digital Output Model for QML
 * @version 1.0.0
 */

#ifndef DIGITAL_OUTPUT_MODEL_H
#define DIGITAL_OUTPUT_MODEL_H

#include <QAbstractListModel>

class DoService;

class DigitalOutputModel : public QAbstractListModel
{
    Q_OBJECT
    
public:
    enum Roles {
        ChannelRole = Qt::UserRole + 1,
        StateRole,
        NameRole
    };
    
    explicit DigitalOutputModel(DoService *service, QObject *parent = nullptr);
    
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QHash<int, QByteArray> roleNames() const override;
    
public slots:
    void refresh();
    
private:
    DoService *m_service = nullptr;
};

#endif // DIGITAL_OUTPUT_MODEL_H

