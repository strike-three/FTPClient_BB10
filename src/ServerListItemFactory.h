/*
 * ServerListItemFactory.h
 *
 *  Created on: Feb 7, 2021
 *      Author: aranade
 */

#ifndef SERVERLISTITEMFACTORY_H_
#define SERVERLISTITEMFACTORY_H_

#include <bb/cascades/ListItemProvider>

using namespace bb::cascades;

class ServerListItemFactory: public bb::cascades::ListItemProvider
{
public:
    ServerListItemFactory();

    VisualNode * createItem(ListView* list, const QString &type);

    void updateItem(ListView* list, VisualNode *listItem, const QString &type,
            const QVariantList &indexPath, const QVariant &data);
};

#endif /* SERVERLISTITEMFACTORY_H_ */
