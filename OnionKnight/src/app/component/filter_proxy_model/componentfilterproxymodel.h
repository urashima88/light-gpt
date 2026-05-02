#pragma once

#include <QSortFilterProxyModel>

class ComponentFilterProxyModel: public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit ComponentFilterProxyModel(QObject* parent = nullptr);

    void setFilterString(const QString& pattern);

    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    QString m_filterString;
};
