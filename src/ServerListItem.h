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
#include <bb/cascades/ImageView>

using namespace bb::cascades;

class ServerListItem: public bb::cascades::CustomListItem
{
public:
    ServerListItem();

    void updateListItem(const QVariant &data);

private:
    Label *serverName;
    Label *serverUrl;
    ImageView *statusImage;
};

#endif /* SERVERLISTITEM_H_ */
