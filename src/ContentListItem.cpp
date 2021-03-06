/*
 * ContentListItem.cpp
 *
 *  Created on: Feb 20, 2021
 *      Author: aranade
 */

#include <src/ContentListItem.h>

#include <bb/cascades/Color>
#include <bb/cascades/Container>
#include <bb/cascades/DockLayout>
#include <bb/cascades/StackLayout>
#include <bb/cascades/StackLayoutProperties>
#include <bb/cascades/UIPalette>

using namespace bb::cascades;

ContentListItem::ContentListItem()
{
    Container *contentItemContainer = Container::create()
                                        .horizontal(HorizontalAlignment::Fill)
                                        .vertical(VerticalAlignment::Fill)
                                        .layout(StackLayout::create()
                                                .orientation(LayoutOrientation::LeftToRight));

    Container *entryTypeContainer = Container::create()
                                        .layout(DockLayout::create())
                                        .layoutProperties(StackLayoutProperties::create()
                                                .spaceQuota(1));

    contentIcon = ImageView::create()
                            .image(Image("asset:///ic_entry.amd"))
                            .scalingMethod(ScalingMethod::AspectFit)
                            .horizontal(HorizontalAlignment::Fill)
                            .vertical(VerticalAlignment::Fill)
                            .filterColor(contentIcon->ui()->palette()->primary());
    entryTypeContainer->add(contentIcon);

    Container *entryNameContainer = Container::create()
                                        .vertical(VerticalAlignment::Fill)
                                        .layout(DockLayout::create())
                                        .layoutProperties(StackLayoutProperties::create()
                                                .spaceQuota(4.5));
//                                        .background(Color::Green);

    contentName = Label::create("File name")
                         .horizontal(HorizontalAlignment::Left)
                         .vertical(VerticalAlignment::Center);

    entryNameContainer->add(contentName);

    contentItemContainer->add(entryTypeContainer);
    contentItemContainer->add(entryNameContainer);
    setContent(contentItemContainer);
}

void ContentListItem::updateListItem(const QVariant &data)
{
    QVariantMap map = data.toMap();

    if(map["type"].toString().compare("File") == 0)
    {
        this->contentIcon->setImage("asset:///ic_doctype_generic.amd");
    }
    else
    {
        this->contentIcon->setImage("asset:///ic_folder.amd");
    }
    this->contentName->setText(map["name"].toString());
}
