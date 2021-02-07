/*
 * ServerListItem.h
 *
 *  Created on: Feb 7, 2021
 *      Author: aranade
 */

#ifndef SERVERLISTITEM_H_
#define SERVERLISTITEM_H_

#include <bb/cascades/CustomListItem>
#include <bb/cascades/Label>

using namespace bb::cascades;

class ServerListItem: public bb::cascades::CustomListItem
{
public:
    ServerListItem();

    void updateItem(const QString text);

private:
    Label *label;
};

#endif /* SERVERLISTITEM_H_ */
