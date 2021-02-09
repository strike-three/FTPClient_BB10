/*
 * ServerListItemFactory.cpp
 *
 *  Created on: Feb 7, 2021
 *      Author: aranade
 */

#include <src/ServerListItemFactory.h>
#include <src/ServerListItem.h>

ServerListItemFactory::ServerListItemFactory()
{
    // TODO Auto-generated constructor stub

}

VisualNode * ServerListItemFactory::createItem(ListView* list, const QString &type)
{

    Q_UNUSED(list);
    Q_UNUSED(type);

    ServerListItem *serverListItem = new ServerListItem();
    return serverListItem;
}

void ServerListItemFactory::updateItem(ListView* list, VisualNode *listItem, const QString &type,
        const QVariantList &indexPath, const QVariant &data)
{
    Q_UNUSED(list);
    Q_UNUSED(indexPath);
    Q_UNUSED(type);

    ServerListItem *serverListItem = static_cast<ServerListItem *>(listItem);
    serverListItem->updateListItem(data);
}
