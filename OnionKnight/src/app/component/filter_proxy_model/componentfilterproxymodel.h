#pragma once

#include <QSortFilterProxyModel>

class ComponentFilterProxyModel: public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit ComponentFilterProxyModel(QObject* parent = nullptr);

    void setFilterString(const QString& pattern);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    QString m_filterString;
};
