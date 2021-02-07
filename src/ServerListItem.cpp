/*
 * ServerListItem.cpp
 *
 *  Created on: Feb 7, 2021
 *      Author: aranade
 */

#include <src/ServerListItem.h>

#include <bb/cascades/Container>
#include <bb/cascades/DockLayout>

using namespace bb::cascades;

ServerListItem::ServerListItem()
{
    Container *serverItemContainer = new Container();
    serverItemContainer->setBackground(Color::Green);
    serverItemContainer->setHorizontalAlignment(HorizontalAlignment::Fill);
    serverItemContainer->setVerticalAlignment(VerticalAlignment::Fill);

    serverItemContainer->setLayout(DockLayout::create());

    this->label = new Label();

    this->label->setVerticalAlignment(VerticalAlignment::Center);
    this->label->setHorizontalAlignment(HorizontalAlignment::Center);
    serverItemContainer->add(this->label);
    setContent(serverItemContainer);
}

void ServerListItem::updateItem(const QString text)
{
    this->label->setText(text);
}
