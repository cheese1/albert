// Copyright (c) 2022 Manuel Schneider

#include "albert/logging.h"
#include "albert/extensions/frontend.h"
#include "albert/extensions/queryhandler.h"
#include "itemsmodel.h"
#include <QStringListModel>
using namespace std;
using namespace albert;

IconProvider ItemsModel::icon_provider;
map<pair<QString /*eid*/, QString /*iid*/>, QIcon> ItemsModel::icon_cache;

int ItemsModel::rowCount(const QModelIndex &parent) const
{
    return (int)items.size();
}

QVariant ItemsModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        const shared_ptr<Item> item = items[index.row()].second;

        switch (role) {
            case (int)ItemRoles::TextRole: return item->text();
            case (int)ItemRoles::SubTextRole: return item->subtext();
            case Qt::ToolTipRole: return QString("%1\n%2").arg(item->text(), item->subtext());
            case (int)ItemRoles::InputActionRole: return item->inputActionText();
            case (int)ItemRoles::IconUrlsRole: return item->iconUrls();
            case (int)ItemRoles::IconPathRole: qFatal("ItemsModel::data ItemRoles::IconPathRole not implemented");
            case (int)ItemRoles::IconRole:
            {
                auto icon_key = make_pair(items[index.row()].first->id(), item->id());
                try {
                    return icon_cache.at(icon_key);
                } catch (const out_of_range &) {
                    QIcon icon = icon_provider.getIcon(item->iconUrls());
                    return icon_cache.emplace(icon_key, icon.isNull() ? QIcon(":unknown") : icon).first->second;
                }
            }
        }
    }
    return {};
}

QHash<int, QByteArray> ItemsModel::roleNames() const
{
    return {
        {(int)ItemRoles::TextRole, "itemTextRole"},
        {(int)ItemRoles::SubTextRole, "itemSubTextRole"},
        {(int)ItemRoles::IconPathRole, "itemIconRole"},
        {(int)ItemRoles::InputActionRole, "itemInputActionRole"}
    };
}

void ItemsModel::add(Extension *extension, shared_ptr<Item> &&item)
{
    beginInsertRows(QModelIndex(), (int)items.size(), (int)items.size());
    items.emplace_back(extension, ::move(item));
    endInsertRows();
}

void ItemsModel::add(Extension *extension, vector<shared_ptr<Item>> &&itemvec)
{
    if (itemvec.empty())
        return;

    beginInsertRows(QModelIndex(), (int)items.size(), (int)(items.size()+itemvec.size()-1));
    items.reserve(items.size()+itemvec.size());
    for (auto &&item : itemvec)
        items.emplace_back(extension, ::move(item));
    endInsertRows();
}

void ItemsModel::add(Extension *extension, const shared_ptr<Item> &item)
{
    beginInsertRows(QModelIndex(), (int)items.size(), (int)items.size());
    items.emplace_back(extension, item);
    endInsertRows();
}

void ItemsModel::add(Extension *extension, const vector<shared_ptr<Item>> &itemvec)
{
    if (itemvec.empty())
        return;

    beginInsertRows(QModelIndex(), (int)items.size(), (int)(items.size()+itemvec.size()-1));
    items.reserve(items.size()+itemvec.size());
    for (auto &item : itemvec)
        items.emplace_back(extension, item);
    endInsertRows();
}

void ItemsModel::add(vector<pair<Extension*,RankItem>>::iterator begin,
                     vector<pair<Extension*,RankItem>>::iterator end)
{
    if (begin == end)
        return;

    beginInsertRows(QModelIndex(), (int)items.size(), (int)(items.size())+(int)(end-begin)-1);
    items.reserve(items.size()+(size_t)(end-begin));
    for (auto it = begin; it != end; ++it)
        items.emplace_back(it->first, ::move(it->second.item));
    endInsertRows();
}

void ItemsModel::clearIconCache()
{
    DEBG << "Clearing icon cache";
    icon_cache.clear();
}

QAbstractListModel *ItemsModel::buildActionsModel(uint i) const
{
    QStringList l;
    for (const auto &a : items[i].second->actions())
        l << a.text;
    return new QStringListModel(l);
}

void ItemsModel::activate(uint i, uint a)
{
    if (i<items.size()){
        auto &item = items[i];
        auto actions = item.second->actions();
        if (a<actions.size()){
            auto &action = actions[a];
            action.function();
            emit activated(item.first->id(), item.second->id(), action.id);
        }
        else
            WARN << "Activated action index is invalid.";
    }
    else
        WARN << "Activated item index is invalid.";
}
