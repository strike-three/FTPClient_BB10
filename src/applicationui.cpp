/*
 * Copyright (c) 2011-2015 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "applicationui.hpp"

#include <QDebug>
#include <bb/cascades/Application>
#include <bb/cascades/Page>
#include <bb/cascades/Container>
#include <bb/cascades/Color>
#include <bb/cascades/DockLayout>
#include <bb/cascades/StackLayout>
#include <bb/cascades/ActionItem>
#include <bb/cascades/TextField>
#include <bb/cascades/DropDown>

#include <bb/cascades/NavigationPaneProperties>
#include <bb/data/JsonDataAccess>

#include <src/ServerListItemFactory.h>

using namespace bb::cascades;

ApplicationUI::ApplicationUI() :
        QObject()
{
    this->rootPage = new Page();

    this->displayInfo = new bb::device::DisplayInfo();
    this->invokemanager = new bb::system::InvokeManager();

    this->list = 0;
    this->label = new Label();
    bool res = QObject::connect(this->invokemanager,
                                SIGNAL(invoked(const bb::system::InvokeRequest&)),
                                this,
                                SLOT(onInvoke(const bb::system::InvokeRequest&)));

    Q_ASSERT(res);

    this->navigationPane = new NavigationPane();

    res = QObject::connect(this->navigationPane,
                            SIGNAL(pushTransitionEnded(bb::cascades::Page*)),
                            this,
                            SLOT(pushFinished(bb::cascades::Page*)));
    Q_ASSERT(res);

    res = QObject::connect(this->navigationPane,
                            SIGNAL(popTransitionEnded(bb::cascades::Page*)),
                            this,
                            SLOT(popFinished(bb::cascades::Page*)));
    Q_ASSERT(res);


    switch(this->invokemanager->startupMode())
    {
        case bb::system::ApplicationStartupMode::LaunchApplication:
        case bb::system::ApplicationStartupMode::InvokeApplication:
            this->initAppUI();
            break;

        case bb::system::ApplicationStartupMode::InvokeCard:
            this->initCardUI();
            break;

        default:
            this->initAppUI();
            break;
    }
}


void ApplicationUI::initAppUI()
{
    QVariantMap map;
    ServerListItemFactory *serverListItemFactory = new ServerListItemFactory();


    Container *listContainer = new Container();
    listContainer->setBackground(Color::Yellow);
    listContainer->setLayout(DockLayout::create());
    listContainer->setHorizontalAlignment(HorizontalAlignment::Fill);
    listContainer->setMinHeight(this->displayInfo->physicalSize().height() * 0.9);

    this->label = new Label();
    this->label->setHorizontalAlignment(HorizontalAlignment::Fill);

    this->list = new ListView();

    listContainer->add(list);
    this->list->setDataModel(&this->listViewDataModel);
    this->list->setListItemProvider(serverListItemFactory);
    this->listViewDataModel.clear();
    this->readAccountInfo();


    Container *appContainer = new Container();
    appContainer->setLayout(StackLayout::create()
                            .orientation(LayoutOrientation::TopToBottom));
    appContainer->setHorizontalAlignment(HorizontalAlignment::Fill);
    appContainer->setVerticalAlignment(VerticalAlignment::Fill);
    appContainer->setBackground(Color::LightGray);
    UIConfig *ui = appContainer->ui();
    appContainer->setLeftPadding(ui->du(1));
    appContainer->setRightPadding(ui->du(1));


    appContainer->add(listContainer);
    appContainer->add(this->label);
    this->rootPage->setContent(appContainer);

    ActionItem *addServerAction = ActionItem::create()
                                            .title("Add")
                                            .image(Image("asset:///ic_add.png"))
                                            .onTriggered(this, SLOT(addServerPage()));


    this->rootPage->addAction(addServerAction, ActionBarPlacement::OnBar);

    this->navigationPane->push(this->rootPage);
    Application::instance()->setScene(this->navigationPane);

}

void ApplicationUI::initCardUI()
{
    QVariantMap map;
    ServerListItemFactory *serverListItemFactory = new ServerListItemFactory();


    Container *listContainer = new Container();
    listContainer->setBackground(Color::Yellow);
    listContainer->setLayout(DockLayout::create());
    listContainer->setHorizontalAlignment(HorizontalAlignment::Fill);
    listContainer->setMinHeight(this->displayInfo->physicalSize().height() * 0.9);

    this->label = new Label();
    this->label->setHorizontalAlignment(HorizontalAlignment::Fill);

    this->list = new ListView();

    listContainer->add(list);
    this->list->setDataModel(&this->listViewDataModel);
    this->list->setListItemProvider(serverListItemFactory);
    this->listViewDataModel.clear();
    this->readAccountInfo();


    Container *appContainer = new Container();
    appContainer->setLayout(StackLayout::create()
                            .orientation(LayoutOrientation::TopToBottom));
    appContainer->setHorizontalAlignment(HorizontalAlignment::Fill);
    appContainer->setVerticalAlignment(VerticalAlignment::Fill);
    appContainer->setBackground(Color::LightGray);
    UIConfig *ui = appContainer->ui();
    appContainer->setLeftPadding(ui->du(1));
    appContainer->setRightPadding(ui->du(1));


    appContainer->add(listContainer);
    appContainer->add(this->label);
    this->rootPage->setContent(appContainer);
    this->navigationPane->push(this->rootPage);
    Application::instance()->setScene(this->navigationPane);

}

int32_t ApplicationUI::readAccountInfo()
{
    QString filePath = QDir::currentPath() + "/app/native/assets/json/accounts.json";
    QFile accountFile(filePath);
    bb::data::JsonDataAccess data;
    int32_t retval = 0;
    int32_t i;
    QVariant accountInfo;

    if(accountFile.exists())
    {
        /* Load account data in List model */
        accountInfo = data.load(filePath);

        if(data.hasError())
        {
            this->label->setText(data.error().errorMessage());
            qDebug()<<data.error();
            retval = -1;
        }

        if(retval == 0)
        {
            QVariantList list = accountInfo.value<QVariantList>();
            this->label->setText(QString::number(list.size()) );
            for(i = 0; i < list.size(); i = i + 1)
            {
                this->listViewDataModel << list.at(i).value<QVariantMap>();
            }
        }

    }

    return 0;
}

void ApplicationUI::addServerPage()
{
    Page *addServerPage = Page::create().objectName("addServerPage");

    ActionItem *backAction = ActionItem::create()
                                .title("Back")
                                .imageSource(QUrl("asset:///ic_previous.amd"))
                                .onTriggered(this->navigationPane, SLOT(pop()));
    addServerPage->setPaneProperties(NavigationPaneProperties::create()
                                        .backButton(backAction));

    this->navigationPane->push(addServerPage);
}

void ApplicationUI::pushFinished(bb::cascades::Page *page)
{
    if(page->objectName().compare("addServerPage") == 0)
    {
    Container *addServerContainer = Container::create()
                                    .horizontal(HorizontalAlignment::Fill)
                                    .vertical(VerticalAlignment::Fill)
                                    .layout(StackLayout::create()
                                            .orientation(LayoutOrientation::TopToBottom));
    UIConfig *ui = addServerContainer->ui();
    addServerContainer->setLeftPadding(ui->du(2));
    addServerContainer->setRightPadding(ui->du(2));

    Label *serverNameLabel = Label::create("Display Name")
                                    .horizontal(HorizontalAlignment::Fill)
                                    .bottomMargin(ui->du(1));
    serverNameLabel->textStyle()->setFontSize(FontSize::Small);
    addServerContainer->add(serverNameLabel);

    TextField *serverName = TextField::create()
                            .hintText("Work FTP")
                            .bottomMargin(ui->du(2))
                            .backgroundVisible(true)
                            .clearButtonVisible(true);

    addServerContainer->add(serverName);

    Label *serverCredentials = Label::create("Server Credentials")
                                    .horizontal(HorizontalAlignment::Fill)
                                    .bottomMargin(ui->du(1));
    serverCredentials->textStyle()->setFontSize(FontSize::Small);

    TextField *serverUrl = TextField::create()
                            .hintText("URL")
                            .bottomMargin(ui->du(1))
                            .backgroundVisible(true)
                            .clearButtonVisible(true);

    TextField *userName = TextField::create()
                            .hintText("User name")
                            .bottomMargin(ui->du(1))
                            .backgroundVisible(true)
                            .clearButtonVisible(true);

    TextField *password = TextField::create()
                            .hintText("Password")
                            .bottomMargin(ui->du(1))
                            .backgroundVisible(true)
                            .clearButtonVisible(true)
                            .inputMode(TextFieldInputMode::Password);


    addServerContainer->add(serverCredentials);
    addServerContainer->add(serverUrl);
    addServerContainer->add(userName);
    addServerContainer->add(password);

//    Container protocolContainer = Container::create()
//                                    .horizontal(HorizontalAlignment::Fill)
//                                    .layout(StackLayout::create()
//                                            .orientation(LayoutOrientation::LeftToRight))
//                                    .top(ui->du(1));
    Label *protocolLabel = Label::create("Protocol")
                            .horizontal(HorizontalAlignment::Fill)
                            .bottomMargin(ui->du(1));
    protocolLabel->textStyle()->setFontSize(FontSize::Small);

    addServerContainer->add(protocolLabel);

    DropDown *protocol = DropDown::create()
                                .title("Protocol")
                                .bottomMargin(ui->du(2));
    protocol->add(Option::create().text("FTP"));
    protocol->add(Option::create().text("SFTP"));

    addServerContainer->add(protocol);

    Label *protocolPort = Label::create("Port")
                            .horizontal(HorizontalAlignment::Fill)
                            .bottomMargin(ui->du(1));
    protocolPort->textStyle()->setFontSize(FontSize::Small);

    TextField *port = TextField::create()
                            .hintText("Port")
                            .bottomMargin(ui->du(1))
                            .backgroundVisible(true)
                            .clearButtonVisible(true)
                            .inputMode(TextFieldInputMode::NumbersAndPunctuation);

    addServerContainer->add(protocolPort);
    addServerContainer->add(port);

    page->setContent(addServerContainer);
    }
}

void ApplicationUI::popFinished(bb::cascades::Page *page)
{
    delete page;
    this->label->setText("Pop finished");
}

void ApplicationUI::onInvoke(const bb::system::InvokeRequest& data)
{
    Q_UNUSED(data);
    this->initCardUI();
}
