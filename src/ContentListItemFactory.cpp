/*
 * ContentListItemFactory.cpp
 *
 *  Created on: Feb 20, 2021
 *      Author: aranade
 */

#include <src/ContentListItemFactory.h>
#include <src/ContentListItem.h>

ContentListItemFactory::ContentListItemFactory()
{
    // TODO Auto-generated constructor stub

}

VisualNode * ContentListItemFactory::createItem(ListView* list, const QString &type)
{

    Q_UNUSED(list);
    Q_UNUSED(type);

    ContentListItem *contentListItem = new ContentListItem();
    return contentListItem;
}

void ContentListItemFactory::updateItem(ListView* list, VisualNode *listItem, const QString &type,
        const QVariantList &indexPath, const QVariant &data)
{
    Q_UNUSED(list);
    Q_UNUSED(indexPath);
    Q_UNUSED(type);

    ContentListItem *contentListItem = static_cast<ContentListItem *>(listItem);
    contentListItem->updateListItem(data);
}
