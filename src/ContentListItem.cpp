/*
 * ContentListItem.cpp
 *
 *  Created on: Feb 20, 2021
 *      Author: aranade
 */

#include <src/ContentListItem.h>

#include <bb/cascades/Container>
#include <bb/cascades/DockLayout>
#include <bb/cascades/StackLayout>
#include <bb/cascades/StackLayoutProperties>

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
                                                .spaceQuota(1))
                                        .background(Color::Green);

    imageView = ImageView::create()
                            .image(Image("asset:///ic_entry.amd"))
                            .scalingMethod(ScalingMethod::AspectFit)
                            .horizontal(HorizontalAlignment::Fill)
                            .vertical(VerticalAlignment::Fill);
    entryTypeContainer->add(imageView);

    Container *entryNameContainer = Container::create()
                                        .vertical(VerticalAlignment::Fill)
                                        .layout(DockLayout::create())
                                        .layoutProperties(StackLayoutProperties::create()
                                                .spaceQuota(4));
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

    this->contentName->setText(map["name"].toString());
}
