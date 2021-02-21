/*
 * ContentListItem.h
 *
 *  Created on: Feb 20, 2021
 *      Author: aranade
 */

#ifndef CONTENTLISTITEM_H_
#define CONTENTLISTITEM_H_

#include <bb/cascades/CustomListItem>

#include <bb/cascades/Label>
#include <bb/cascades/ImageView>

using namespace bb::cascades;

class ContentListItem: public bb::cascades::CustomListItem
{
public:
    ContentListItem();

    void updateListItem(const QVariant &data);

private:
    ImageView *contentIcon;
    Label *contentName;
};

#endif /* CONTENTLISTITEM_H_ */
