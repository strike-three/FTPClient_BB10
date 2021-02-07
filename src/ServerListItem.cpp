/*
 * ServerListItem.cpp
 *
 *  Created on: Feb 7, 2021
 *      Author: aranade
 */

#include <src/ServerListItem.h>

#include <bb/cascades/Container>
#include <bb/cascades/DockLayout>
#include <bb/cascades/StackLayout>
#include <bb/cascades/StackLayoutProperties>

using namespace bb::cascades;

ServerListItem::ServerListItem()
{
    Container *serverItemContainer = new Container();
    serverItemContainer->setHorizontalAlignment(HorizontalAlignment::Fill);
    serverItemContainer->setVerticalAlignment(VerticalAlignment::Fill);
    serverItemContainer->setLayout(StackLayout::create().
                                    orientation(LayoutOrientation::LeftToRight));

    Container *logoContainer = new Container();
    logoContainer->setVerticalAlignment(VerticalAlignment::Fill);
    logoContainer->setMinWidth(200);
    logoContainer->setBackground(Color::Green);
    logoContainer->setLayout(DockLayout::create());
    logoContainer->setLayoutProperties(StackLayoutProperties::create()
                                        .spaceQuota(0.49));

    Container *labelContainer = new Container();
    labelContainer->setVerticalAlignment(VerticalAlignment::Fill);
    labelContainer->setMinWidth(200);
    labelContainer->setBackground(Color::Red);
    labelContainer->setLayout(DockLayout::create());
    labelContainer->setLayoutProperties(StackLayoutProperties::create()
                                            .spaceQuota(0.5));

    this->label = new Label();

    this->label->setVerticalAlignment(VerticalAlignment::Center);
    this->label->setHorizontalAlignment(HorizontalAlignment::Center);
    labelContainer->add(this->label);
    serverItemContainer->add(logoContainer);
    serverItemContainer->add(labelContainer);
    setContent(serverItemContainer);
}

void ServerListItem::updateItem(const QString text)
{
    this->label->setText(text);
}
