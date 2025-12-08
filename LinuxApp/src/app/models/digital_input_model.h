/**
 * @file digital_input_model.h
 * @brief Digital Input Model for QML
 * @version 1.0.0
 */

#ifndef DIGITAL_INPUT_MODEL_H
#define DIGITAL_INPUT_MODEL_H

#include <QAbstractListModel>
#include <QVector>

class DiService;

class DigitalInputModel : public QAbstractListModel
{
    Q_OBJECT
    
public:
    enum Roles {
        ChannelRole = Qt::UserRole + 1,
        StateRole,
        NameRole
    };
    
    explicit DigitalInputModel(DiService *service, QObject *parent = nullptr);
    
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    
public slots:
    void refresh();
    
private:
    DiService *m_service = nullptr;
};

#endif // DIGITAL_INPUT_MODEL_H

