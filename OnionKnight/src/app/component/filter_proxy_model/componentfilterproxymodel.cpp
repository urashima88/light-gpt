#include "componentfilterproxymodel.h"
#include <QAbstractItemModel>

ComponentFilterProxyModel::ComponentFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setRecursiveFilteringEnabled(true);
    setDynamicSortFilter(true);
}

void ComponentFilterProxyModel::setFilterString(const QString& pattern)
{
    m_filterString = pattern.toLower();
    QSortFilterProxyModel::setFilterFixedString(pattern);
}

bool ComponentFilterProxyModel::filterAcceptsRow(int sourceRow,
                                                 const QModelIndex& sourceParent) const
{
    if (m_filterString.isEmpty())
        return true;

    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    const QString text = sourceModel()->data(index, Qt::DisplayRole).toString();
    return text.contains(m_filterString, Qt::CaseInsensitive);
}