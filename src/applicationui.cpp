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
#include <bb/cascades/ScrollView>
#include <bb/cascades/Divider>
#include <bb/cascades/DeleteActionItem>
#include <bb/cascades/ActionSet>
#include <bb/cascades/Button>
#include <bb/cascades/StackLayoutProperties>

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
    Application::instance()->setScene(this->navigationPane);

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

    /* Read the accounts information and populate the listdatamodel*/
    this->listViewDataModel.clear();
    this->readAccountInfo();

    switch(this->invokemanager->startupMode())
    {
        case bb::system::ApplicationStartupMode::LaunchApplication:
        case bb::system::ApplicationStartupMode::InvokeApplication:
            this->rootPage->setObjectName("appLaunchPage");
            this->label->setText("Launch");
            renderServerListPage(this->rootPage, false);
            break;

        case bb::system::ApplicationStartupMode::InvokeCard:
            this->rootPage->setObjectName("cardLaunchPage");
            this->label->setText("Card");
            renderServerListPage(this->rootPage, true);
            break;

        default:
            this->rootPage->setObjectName("appLaunchPage");
            renderServerListPage(this->rootPage, false);
            break;
    }
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

void ApplicationUI::saveAccountInfo()
{
    QString filePath = QDir::currentPath() + "/app/native/assets/json/accounts.json";
    QFile accountFile(filePath);
    bb::data::JsonDataAccess data;
    QVariantList accountData;

    accountFile.resize(0);

    for(int i = 0; i < this->listViewDataModel.size(); i++)
    {
        accountData.append(this->listViewDataModel.value(i));
    }
    data.save(accountData, filePath);
}

void ApplicationUI::addServerPressed()
{
    Page *addServerPage = Page::create().objectName("addServerPage");


    this->navigationPane->push(addServerPage);
}

void ApplicationUI::pushFinished(bb::cascades::Page *page)
{

    /* No action on push finish signal on application launch */

    if(page->objectName().compare("addServerPage") == 0)
    {
        this->renderAddServerPage(page, false, 0);
    }

    if(page->objectName().compare("serverEntryEditPage") == 0)
    {
        qDebug()<<this->list->selected().at(0).toInt();
        this->renderAddServerPage(page, true, this->list->selected().at(0).toInt());
        this->list->clearSelection();
    }

}

void ApplicationUI::popFinished(bb::cascades::Page *page)
{
    if(page->objectName().compare("addServerPage") == 0)
    {
        qDebug()<<"Remove page "<<page->objectName();
        page->deleteLater();
    }

    if(page->objectName().compare("serverEntryEditPage") == 0)
    {
        qDebug()<<"Remove page "<<page->objectName();
        page->deleteLater();
    }

    this->label->setText("Pop finished");
}

void ApplicationUI::renderServerListPage(bb::cascades::Page *page, bool card)
{
    ServerListItemFactory *serverListItemFactory = new ServerListItemFactory();

    Container *listContainer = new Container();
//    listContainer->setBackground(Color::Yellow);
    listContainer->setLayout(DockLayout::create());
    listContainer->setHorizontalAlignment(HorizontalAlignment::Fill);
    UIConfig *ui = listContainer->ui();
    listContainer->setLeftPadding(ui->du(1));
    listContainer->setRightPadding(ui->du(1));

    this->list = new ListView();
    this->list->setDataModel(&this->listViewDataModel);
    this->list->setListItemProvider(serverListItemFactory);
    this->list->setObjectName("serverList");
    listContainer->add(list);

    if(!card)
    {
        ActionSet *listItemActions = ActionSet::create().title("Actions");

        ActionItem *editEntry = ActionItem::create()
                                            .title("Edit")
                                            .image(Image("asset:///ic_edit.png"))
                                            .onTriggered(this, SLOT(onServerEntryEdit()));

        DeleteActionItem *delServerEntry = DeleteActionItem::create()
                                                .onTriggered(this, SLOT(onServerEntryDelete()));

        listItemActions->add(editEntry);
        listItemActions->add(delServerEntry);

        this->list->addActionSet(listItemActions);

        ActionItem *addServerAction = ActionItem::create()
                                                .title("Add")
                                                .image(Image("asset:///ic_add.png"))
                                                .onTriggered(this, SLOT(addServerPressed()));

        page->addAction(addServerAction, ActionBarPlacement::OnBar);
    }
    page->setContent(listContainer);
    this->navigationPane->push(page);
}

void ApplicationUI::renderAddServerPage(bb::cascades::Page *page, bool prefill, int index)
{
    ScrollView *scrollview = ScrollView::create().objectName("Scroll View");
    Container *addServerContainer = Container::create()
                                   .objectName("addServerContainer")
                                   .horizontal(HorizontalAlignment::Fill)
                                   .vertical(VerticalAlignment::Fill)
                                   .layout(StackLayout::create()
                                           .orientation(LayoutOrientation::TopToBottom));
    UIConfig *ui = addServerContainer->ui();
    addServerContainer->setLeftPadding(ui->du(2));
    addServerContainer->setRightPadding(ui->du(2));

    Label *serverNameLabel = Label::create("Display Name")
                                   .objectName("serverNameLabel")
                                   .horizontal(HorizontalAlignment::Fill)
                                   .bottomMargin(ui->du(1));
    serverNameLabel->textStyle()->setFontSize(FontSize::Small);
    addServerContainer->add(serverNameLabel);

    TextField *serverName = TextField::create()
                           .objectName("serverName")
                           .hintText("Work FTP")
                           .bottomMargin(ui->du(2))
                           .backgroundVisible(true)
                           .clearButtonVisible(true);
    serverName->input()->setSubmitKey(SubmitKey::Done);
    serverName->input()->setSubmitKeyFocusBehavior(SubmitKeyFocusBehavior::Next);

    addServerContainer->add(serverName);
    addServerContainer->add(Divider::create().horizontal(HorizontalAlignment::Fill));

    Label *serverCredentials = Label::create("Server Credentials")
                                   .objectName("serverCredentials")
                                   .horizontal(HorizontalAlignment::Fill)
                                   .bottomMargin(ui->du(1));
    serverCredentials->textStyle()->setFontSize(FontSize::Small);

    TextField *serverUrl = TextField::create()
                           .objectName("serverUrl")
                           .hintText("URL")
                           .bottomMargin(ui->du(1))
                           .backgroundVisible(true)
                           .clearButtonVisible(true);
    serverUrl->input()->setSubmitKey(SubmitKey::Done);
    serverUrl->input()->setSubmitKeyFocusBehavior(SubmitKeyFocusBehavior::Next);

    TextField *userName = TextField::create()
                           .objectName("userName")
                           .hintText("User name")
                           .bottomMargin(ui->du(1))
                           .backgroundVisible(true)
                           .clearButtonVisible(true);
    userName->input()->setSubmitKey(SubmitKey::Done);
    userName->input()->setSubmitKeyFocusBehavior(SubmitKeyFocusBehavior::Next);

    TextField *password = TextField::create()
                           .objectName("password")
                           .hintText("Password")
                           .bottomMargin(ui->du(1))
                           .backgroundVisible(true)
                           .clearButtonVisible(true)
                           .inputMode(TextFieldInputMode::Password);
    password->input()->setSubmitKey(SubmitKey::Done);
    password->input()->setSubmitKeyFocusBehavior(SubmitKeyFocusBehavior::Next);

    addServerContainer->add(serverCredentials);
    addServerContainer->add(serverUrl);
    addServerContainer->add(userName);
    addServerContainer->add(password);
    addServerContainer->add(Divider::create().horizontal(HorizontalAlignment::Fill));

    Label *protocolLabel = Label::create("Protocol")
                           .objectName("protocolLabel")
                           .horizontal(HorizontalAlignment::Fill)
                           .bottomMargin(ui->du(1));
    protocolLabel->textStyle()->setFontSize(FontSize::Small);

    DropDown *protocol = DropDown::create()
                               .objectName("protocol")
                               .title("Protocol")
                               .bottomMargin(ui->du(2));
    protocol->add(Option::create().text("FTP"));
    protocol->add(Option::create().text("SFTP"));
    protocol->setSelectedIndex(0);

    addServerContainer->add(protocolLabel);
    addServerContainer->add(protocol);

    addServerContainer->add(Divider::create().horizontal(HorizontalAlignment::Fill));

    Label *protocolPort = Label::create("Port")
                           .objectName("protocolPort")
                           .horizontal(HorizontalAlignment::Fill)
                           .bottomMargin(ui->du(1));
    protocolPort->textStyle()->setFontSize(FontSize::Small);

    TextField *port = TextField::create()
                           .objectName("port")
                           .hintText("Port")
                           .bottomMargin(ui->du(1))
                           .backgroundVisible(true)
                           .clearButtonVisible(true)
                           .inputMode(TextFieldInputMode::NumbersAndPunctuation);
    port->input()->setSubmitKey(SubmitKey::Done);
    port->input()->setSubmitKeyFocusBehavior(SubmitKeyFocusBehavior::Lose);

    addServerContainer->add(protocolPort);
    addServerContainer->add(port);

    addServerContainer->add(Divider::create().horizontal(HorizontalAlignment::Fill));

    Container *buttonContainer = Container::create()
                                    .objectName("buttonContainer")
                                    .layout(StackLayout::create()
                                            .orientation(LayoutOrientation::LeftToRight))
                                    .horizontal(HorizontalAlignment::Fill);

    Button *saveButton = Button::create("Save")
                            .objectName("saveButton")
                            .rightMargin(ui->du(2))
                            .layoutProperties(StackLayoutProperties::create().spaceQuota(1));
    bool res = QObject::connect(saveButton, SIGNAL(clicked()),
                                this, SLOT(onServerSave()));
    Q_ASSERT(res);

    Button *testButton = Button::create("Test Connection")
                            .objectName("testButton")
                            .rightMargin(ui->du(2))
                            .layoutProperties(StackLayoutProperties::create().spaceQuota(1));

    res = QObject::connect(testButton, SIGNAL(clicked()),
                                this, SLOT(onServerConnTest()));
    Q_ASSERT(res);

    buttonContainer->add(saveButton);
    buttonContainer->add(testButton);

    TextField *operation = TextField::create()
                            .objectName("operation")
                            .visible(false)
                            .text("appendEntry");

    TextField *entryindex = TextField::create()
                        .objectName("entryIndex")
                        .visible(false);

    entryindex->setText(QString::number(this->listViewDataModel.size()));

    addServerContainer->add(buttonContainer);
    addServerContainer->add(operation);
    addServerContainer->add(entryindex);

    scrollview->setContent(addServerContainer);
    page->setContent(scrollview);

    if(prefill)
    {
        QVariantMap map = this->listViewDataModel.value(index).toMap();
        serverName->setText(map["name"].toString());
        serverUrl->setText(map["url"].toString());
        userName->setText(map["uname"].toString());
        password->setText(map["password"].toString());
        if(map["protocol"].toString().compare("FTP") == 0)
        {
            protocol->setSelectedIndex(FTP_PROTOCOL);
        }
        else
        {
            protocol->setSelectedIndex(SFTP_PROTOCOL);
        }
        port->setText(map["port"].toString());

        operation->setText("editEntry");
        entryindex->setText(QString::number(index));
    }
}

void ApplicationUI::onServerEntryEdit()
{
    Page *serverEntryEditPage = Page::create()
        .objectName("serverEntryEditPage");

    this->navigationPane->push(serverEntryEditPage);
}

void ApplicationUI::onServerEntryDelete()
{
    qDebug()<<"Delete entry "<<this->navigationPane->at(0)->findChild<ListView *>("serverList")->selected();
    this->list->clearSelection();
}

void ApplicationUI::onServerSave()
{
    QVariantMap map;
    map["name"] = this->navigationPane->at(1)->findChild<TextField *>("serverName")->text();
    map["url"] = this->navigationPane->at(1)->findChild<TextField *>("serverUrl")->text();
    map["uname"] = this->navigationPane->at(1)->findChild<TextField *>("userName")->text();
    map["password"] = this->navigationPane->at(1)->findChild<TextField *>("password")->text();
    map["protocol"] = this->navigationPane->at(1)->findChild<DropDown *>("protocol")->selectedOption()->text();
    map["port"] = this->navigationPane->at(1)->findChild<TextField *>("port")->text().toUInt();
    map["connstatus"] = true;

    if(this->navigationPane->at(1)->findChild<TextField *>("operation")->text()
            .compare("editEntry") == 0)
    {
        qDebug()<<"removing entry at "<<this->navigationPane->at(1)->findChild<TextField *>("entryIndex")->text().toInt();
        this->listViewDataModel.removeAt(
        this->navigationPane->at(1)->findChild<TextField *>("entryIndex")->text().toInt());

        qDebug()<<this->listViewDataModel.size();
    }

    qDebug()<<"inserting entry at "<<this->navigationPane->at(1)->findChild<TextField *>("entryIndex")->text().toInt();
    this->listViewDataModel.insert(this->navigationPane->at(1)->findChild<TextField *>("entryIndex")->text().toInt(),
                                    (QVariant)map);

    this->saveAccountInfo();
    this->listViewDataModel.clear();
    this->readAccountInfo();

    this->navigationPane->at(1)->findChild<Button *>("saveButton")->setEnabled(false);
}

void ApplicationUI::onServerConnTest()
{
    this->ftpInterface = new Ftp_interface();

    bool res = QObject::connect(ftpInterface, SIGNAL(verificationDone(QVariant)),
            this, SLOT(onVerificationDone(QVariant)));

    Q_ASSERT(res);

    ftpInterface->verifyServerConnection(this->listViewDataModel.value(0));
}

void ApplicationUI::onVerificationDone(QVariant verificationResult)
{
    QVariantMap map = verificationResult.toMap();
    qDebug()<<"Verified" << map["result"] << " "<<map["reason"];
    delete this->ftpInterface;
}

void ApplicationUI::onInvoke(const bb::system::InvokeRequest& data)
{
    Q_UNUSED(data);
    this->rootPage->setObjectName("appLaunchPage");
    this->navigationPane->push(this->rootPage);
}
