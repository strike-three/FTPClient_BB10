/*
 * ContentListItemFactory.h
 *
 *  Created on: Feb 20, 2021
 *      Author: aranade
 */

#ifndef CONTENTLISTITEMFACTORY_H_
#define CONTENTLISTITEMFACTORY_H_

#include <bb/cascades/ListItemProvider>

using namespace bb::cascades;

class ContentListItemFactory : public bb::cascades::ListItemProvider
{
public:
    ContentListItemFactory();

    VisualNode * createItem(ListView* list, const QString &type);

    void updateItem(ListView* list, VisualNode *listItem, const QString &type,
            const QVariantList &indexPath, const QVariant &data);
};

#endif /* CONTENTLISTITEMFACTORY_H_ */
