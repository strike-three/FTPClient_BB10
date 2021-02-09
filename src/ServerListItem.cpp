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
#include <bb/cascades/ImagePaint>
#include <bb/cascades/UIConfig>

using namespace bb::cascades;

ServerListItem::ServerListItem()
{
    UIConfig *ui;

    Container *serverItemContainer = new Container();
    serverItemContainer->setHorizontalAlignment(HorizontalAlignment::Fill);
    serverItemContainer->setVerticalAlignment(VerticalAlignment::Fill);
    serverItemContainer->setLayout(StackLayout::create().
                                    orientation(LayoutOrientation::LeftToRight));
    serverItemContainer->setBackground(
            ImagePaint(QUrl("asset:///listBackground.png"), RepeatPattern::Fill));

    ui = serverItemContainer->ui();

    serverItemContainer->setTopPadding(ui->du(0.9));
    serverItemContainer->setBottomPadding(ui->du(1.7));
    Container *logoContainer = new Container();
    logoContainer->setVerticalAlignment(VerticalAlignment::Fill);
//    logoContainer->setMinWidth(200);
//    logoContainer->setBackground(Color::Green);
    logoContainer->setLayout(DockLayout::create());
    logoContainer->setLayoutProperties(StackLayoutProperties::create()
                                        .spaceQuota(1));

    ImageView *image = new ImageView();
    image->setImage(Image("asset:///server.png"));
    image->setHorizontalAlignment(HorizontalAlignment::Center);
    image->setScalingMethod(ScalingMethod::AspectFit);
    logoContainer->add(image);

    Container *labelContainer = new Container();
    labelContainer->setVerticalAlignment(VerticalAlignment::Fill);
//    labelContainer->setMinWidth(200);
//    labelContainer->setBackground(Color::Red);
    labelContainer->setLayout(DockLayout::create());
    labelContainer->setLayoutProperties(StackLayoutProperties::create()
                                            .spaceQuota(3));

    ui = labelContainer->ui();
    labelContainer->setBottomPadding(ui->du(0.6));
    labelContainer->setLeftPadding(ui->du(2));
    labelContainer->setTopPadding(ui->du(0.4));
    this->serverName = new Label();

    this->serverName->setVerticalAlignment(VerticalAlignment::Top);
    this->serverName->setHorizontalAlignment(HorizontalAlignment::Left);
    this->serverName->textStyle()->setFontFamily("Arial");
    this->serverName->textStyle()->setFontSize(FontSize::Default);
    this->serverName->textStyle()->setFontWeight(FontWeight::W500);
    this->serverUrl = new Label();
    this->serverUrl->setVerticalAlignment(VerticalAlignment::Bottom);
    this->serverUrl->setHorizontalAlignment(HorizontalAlignment::Left);
    this->serverUrl->textStyle()->setFontFamily("Arial");
    this->serverUrl->textStyle()->setFontSize(FontSize::XSmall);
    this->serverUrl->textStyle()->setFontWeight(FontWeight::W200);

    Container *connStatusContainer = new Container();
    connStatusContainer->setVerticalAlignment(VerticalAlignment::Fill);
    connStatusContainer->setLayout(DockLayout::create());
    connStatusContainer->setLayoutProperties(StackLayoutProperties::create()
                                            .spaceQuota(1));
//    connStatusContainer->setBackground(Color::Yellow);

    this->statusImage = new ImageView();
    this->statusImage->setImage(Image("asset:///ic_done.amd"));
    this->statusImage->setHorizontalAlignment(HorizontalAlignment::Center);
    this->statusImage->setScalingMethod(ScalingMethod::AspectFit);
    connStatusContainer->add(this->statusImage);

    labelContainer->add(this->serverName);
    labelContainer->add(this->serverUrl);
    serverItemContainer->add(logoContainer);
    serverItemContainer->add(labelContainer);
    serverItemContainer->add(connStatusContainer);
    setContent(serverItemContainer);
}

void ServerListItem::updateListItem(const QVariant &data)
{
    QVariantMap map = data.value<QVariantMap>();
    this->serverName->setText(map["servername"].toString());
    this->serverUrl->setText(map["url"].toString());

    if(map["connstatus"].toBool())
    {
        this->statusImage->setImage("asset:///ic_done.amd");
        this->statusImage->setFilterColor(Color::Green);
    }
    else
    {
        this->statusImage->setImage("asset:///ic_incomplete.amd");
        this->statusImage->setFilterColor(Color::Red);
    }
}
