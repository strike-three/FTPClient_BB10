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
#include <src/ContentListItemFactory.h>

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

void ApplicationUI::onInvoke(const bb::system::InvokeRequest& data)
{
    Q_UNUSED(data);
    this->rootPage->setObjectName("appLaunchPage");
    this->navigationPane->push(this->rootPage);
}

/*****************************************************************************
 *                  Read / Save Account info
 * ***************************************************************************/
int32_t ApplicationUI::readAccountInfo()
{
    QString filePath = QDir::currentPath() + ACCOUNT_INFO_FILE_PATH;
    QFile accountFile(filePath);
    bb::data::JsonDataAccess data;
    int32_t retval = 0;
    int32_t i;
    QVariant accountInfo;

    accountFile.open(QIODevice::ReadWrite);

    /* Load account data in List model */
    accountInfo = data.load(&accountFile);
    accountFile.close();
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

    return 0;
}

void ApplicationUI::saveAccountInfo()
{
    QString filePath = QDir::currentPath() + ACCOUNT_INFO_FILE_PATH;
    QFile accountFile(filePath);
    bb::data::JsonDataAccess data;
    QVariantList accountData;

    accountFile.open(QIODevice::ReadWrite);

    for(int i = 0; i < this->listViewDataModel.size(); i++)
    {
        accountData.append(this->listViewDataModel.value(i));
    }
    data.save(accountData, &accountFile);

    accountFile.close();
}

/*****************************************************************************
 *                  Push / Pop signals from Navigation pane
 * ***************************************************************************/

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

    if(page->objectName().compare("listContentsPage") == 0)
    {
        this->renderContentsPage(page);
    }

}

void ApplicationUI::popFinished(bb::cascades::Page *page)
{
    if(page->objectName().compare("addServerPage") == 0)
    {
        qDebug()<<"Remove page "<<page->objectName();
        this->initCommandMetaData();
        page->deleteLater();
    }

    if(page->objectName().compare("serverEntryEditPage") == 0)
    {
        qDebug()<<"Remove page "<<page->objectName();
        this->initCommandMetaData();
        page->deleteLater();
    }

    if(page->objectName().compare("listContentsPage") == 0)
    {
        qDebug()<<"Remove page "<<page->objectName();
        this->initCommandMetaData();
//        page->deleteLater();
    }
    this->label->setText("Pop finished");
}

/*****************************************************************************
 *                  Rendering pages
 * ***************************************************************************/

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

    bool res = QObject::connect(this->list, SIGNAL(triggered(QVariantList)),
                                        this, SLOT(onServerTriggered(QVariantList)));

    Q_ASSERT(res);

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
    serverName->input()->setSubmitKey(SubmitKey::Next);
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
    serverUrl->input()->setSubmitKey(SubmitKey::Next);
    serverUrl->input()->setSubmitKeyFocusBehavior(SubmitKeyFocusBehavior::Next);

    TextField *userName = TextField::create()
                           .objectName("userName")
                           .hintText("User name")
                           .bottomMargin(ui->du(1))
                           .backgroundVisible(true)
                           .clearButtonVisible(true);
    userName->input()->setSubmitKey(SubmitKey::Next);
    userName->input()->setSubmitKeyFocusBehavior(SubmitKeyFocusBehavior::Next);

    TextField *password = TextField::create()
                           .objectName("password")
                           .hintText("Password")
                           .bottomMargin(ui->du(1))
                           .backgroundVisible(true)
                           .clearButtonVisible(true)
                           .inputMode(TextFieldInputMode::Password);
    password->input()->setSubmitKey(SubmitKey::Next);
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

    bool res = QObject::connect(protocol, SIGNAL(selectedIndexChanged(int)),
                                this, SLOT(onProtocolSelected(int)));

    Q_ASSERT(res);

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
    port->input()->setSubmitKey(SubmitKey::Next);
    port->input()->setSubmitKeyFocusBehavior(SubmitKeyFocusBehavior::Next);

    addServerContainer->add(protocolPort);
    addServerContainer->add(port);

    addServerContainer->add(Divider::create().horizontal(HorizontalAlignment::Fill));

    Label *startPath = Label::create("Start Path")
                           .objectName("startPath")
                           .horizontal(HorizontalAlignment::Fill)
                           .bottomMargin(ui->du(1));
    protocolPort->textStyle()->setFontSize(FontSize::Small);

    TextField *startPathText = TextField::create()
                           .objectName("startPathText")
                           .hintText("/")
                           .bottomMargin(ui->du(1))
                           .backgroundVisible(true)
                           .clearButtonVisible(true)
                           .inputMode(TextFieldInputMode::Default);
    startPathText->input()->setSubmitKey(SubmitKey::Done);
    startPathText->input()->setSubmitKeyFocusBehavior(SubmitKeyFocusBehavior::Lose);

    addServerContainer->add(startPath);
    addServerContainer->add(startPathText);

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
    res = QObject::connect(saveButton, SIGNAL(clicked()),
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

        startPathText->setText(map["startPath"].toString());

        operation->setText("editEntry");
        entryindex->setText(QString::number(index));
    }
}

void ApplicationUI::renderContentsPage(bb::cascades::Page *page)
{
    ContentListItemFactory *contentListItemFactory = new ContentListItemFactory();

    QVariantMap map = this->listViewDataModel.value(this->label->text().toInt()).toMap();
    qDebug()<<"Listing for server "<<map["name"].toString();

    Container *contentsListContainer = new Container();
//    listContainer->setBackground(Color::Yellow);
    contentsListContainer->setLayout(DockLayout::create());
    contentsListContainer->setHorizontalAlignment(HorizontalAlignment::Fill);
    UIConfig *ui = contentsListContainer->ui();
    contentsListContainer->setLeftPadding(ui->du(1));
    contentsListContainer->setRightPadding(ui->du(1));

    ListView *contentsList = new ListView();
    contentsList->setListItemProvider(contentListItemFactory);
    GroupDataModel *contentsData = new GroupDataModel(QStringList() << "type" << "name");
    contentsData->setObjectName("contentsData");
    contentsData->setGrouping(ItemGrouping::None);
    contentsList->setDataModel(contentsData);
    contentsList->setObjectName("contentsList");
    contentsListContainer->add(contentsList);

    bool res = QObject::connect(contentsList, SIGNAL(triggered(QVariantList)),
                                this, SLOT(onContentItemTriggered(QVariantList)));

    Q_ASSERT(res);

    page->setContent(contentsListContainer);

    this->createFtpInstance();

    this->command_meta_data.sequenceId = SEQUENCE_LIST_FOLDER;
    /* Enter command meta data */
    this->command_meta_data.sequence = ACTION_CONNECT | ACTION_LOGIN | ACTION_LIST_FOLDER | ACTION_DISCONNECT;
    this->command_meta_data.url = map["url"].toString();
    this->command_meta_data.uname = map["uname"].toString();
    this->command_meta_data.password = map["password"].toString();
    this->command_meta_data.port = map["port"].toInt();
    this->command_meta_data.path.append(map["startPath"].toString());

    this->startCommand();

}
/*****************************************************************************
 *                  Signals / slots
 * ***************************************************************************/

void ApplicationUI::addServerPressed()
{
    Page *addServerPage = Page::create().objectName("addServerPage");


    this->navigationPane->push(addServerPage);
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

    this->listViewDataModel.removeAt(
            this->list->selected().at(0).toInt());
    this->list->clearSelection();

    this->saveAccountInfo();
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
    map["startPath"] = this->navigationPane->at(1)->findChild<TextField *>("startPathText")->text();

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
    this->createFtpInstance();
    this->initCommandMetaData();
    this->command_meta_data.sequenceId = SEQUENCE_VERIFY;
    /* Enter command meta data */
    this->command_meta_data.sequence = ACTION_CONNECT | ACTION_LOGIN | ACTION_DISCONNECT;
    this->command_meta_data.url = this->navigationPane->top()->findChild<TextField *>("serverUrl")->text();
    this->command_meta_data.uname = this->navigationPane->top()->findChild<TextField *>("userName")->text();
    this->command_meta_data.password = this->navigationPane->top()->findChild<TextField *>("password")->text();
    this->command_meta_data.port = this->navigationPane->top()->findChild<TextField *>("port")->text().toInt();

    bool res = QObject::connect(this, SIGNAL(verificationFinished()), this, SLOT(serverConnTestFinished()));
    Q_ASSERT(res);

    this->startCommand();
}

void ApplicationUI::serverConnTestFinished()
{
    if(this->command_meta_data.error)
    {
        qDebug()<<"Verification failed :" << this->command_meta_data.errorString;
    }
    else
    {
        qDebug()<<"Verification success";
    }
    this->ftp->deleteLater();
}

void ApplicationUI::onServerTriggered(QVariantList index)
{
    Page *listContentsPage = Page::create()
        .objectName("listContentsPage");
    this->label->setText(index.at(0).toString());
    this->navigationPane->push(listContentsPage);
}

void ApplicationUI::onProtocolSelected(int index)
{
    this->navigationPane->top()->findChild<TextField *>("port")->setLocallyFocused(true);
    if(index == FTP_PROTOCOL)
    {
        this->navigationPane->top()->findChild<TextField *>("port")->setText("21");
    }
    else
    {
        this->navigationPane->top()->findChild<TextField *>("port")->setText("452");
    }

}

void ApplicationUI::onListInfo(const QUrlInfo& contentInfo)
{
    QString info;
    QVariantMap map;
    if(contentInfo.isValid())
    {
        if(contentInfo.isDir())
        {
            map["type"] = "Directory";
            map["name"] = contentInfo.name();
            this->navigationPane->top()->findChild<GroupDataModel *>("contentsData")->insert(map);
        }

        if(contentInfo.isExecutable())
        {
            map["type"] = "Application";
            map["name"] = contentInfo.name();
            this->navigationPane->top()->findChild<GroupDataModel *>("contentsData")->insert(map);
        }

        if(contentInfo.isFile())
        {
            map["type"] = "File";
            map["name"] = contentInfo.name();
            this->navigationPane->top()->findChild<GroupDataModel *>("contentsData")->insert(map);
        }
    }

}

void ApplicationUI::onContentItemTriggered(QVariantList index)
{
    QVariantMap item = this->navigationPane->top()->findChild<GroupDataModel *>("contentsData")->data(index).toMap();

    if(item["type"].toString().compare("Directory") == 0)
    {
        this->command_meta_data.path.append(item["name"].toString());
        qDebug()<<"setting pane properties";
        /* Set custom back button for the navigation pane */
        ActionItem *customBack = ActionItem::create()
                                    .title("Up")
                                    .imageSource(QUrl("asset:///ic_previous.png"))
                                    .onTriggered(this, SLOT(onCustomBackButton()));

        NavigationPaneProperties *navigationPaneProperties = new NavigationPaneProperties();
        navigationPaneProperties->setBackButton(customBack);
        this->navigationPane->top()->setPaneProperties(navigationPaneProperties);

        this->command_meta_data.sequence = ACTION_CONNECT | ACTION_LOGIN | ACTION_LIST_FOLDER | ACTION_DISCONNECT;
        this->startCommand();

    }
}

void ApplicationUI::onCustomBackButton()
{

    if(this->command_meta_data.path.size() > 1)
    {
        this ->command_meta_data.path.removeLast();
    }

    qDebug()<<"Custom back button"  << this->command_meta_data.path.join("/");

    if(this->command_meta_data.path.size() == 1)
    {
        /* listing for the previous folder */

        this->navigationPane->top()->resetPaneProperties();
    }

    this->command_meta_data.sequence = ACTION_CONNECT | ACTION_LOGIN | ACTION_LIST_FOLDER | ACTION_DISCONNECT;
    this->startCommand();
}
/*****************************************************************************
 *                  FTP methods
 * ***************************************************************************/

void ApplicationUI::initCommandMetaData()
{
    this->command_meta_data.sequenceId = 0;
    this->command_meta_data.sequence = 0;
    this->command_meta_data.url.clear();
    this->command_meta_data.uname.clear();
    this->command_meta_data.password.clear();
    this->command_meta_data.port = 0;
    this->command_meta_data.path.clear();
    this->command_meta_data.error = false;
    this->command_meta_data.errorString.clear();
}



void ApplicationUI::createFtpInstance()
{
    this->ftp = new QFtp();

    bool res = QObject::connect(ftp, SIGNAL(stateChanged(int)),
                                this, SLOT(onFtpStateChanged(int)));
    Q_ASSERT(res);

    res = QObject::connect(ftp, SIGNAL(commandStarted(int)),
                                this, SLOT(onFtpCommandStarted(int)));
    Q_ASSERT(res);

    res = QObject::connect(ftp, SIGNAL(commandFinished(int, bool)),
                                this, SLOT(onFtpCommandFinished(int, bool)));
    Q_ASSERT(res);

    res = QObject::connect(ftp, SIGNAL(listInfo(const QUrlInfo&)),
                                this, SLOT(onListInfo(const QUrlInfo&)));
    Q_ASSERT(res);

}

void ApplicationUI::startCommand()
{

    if(this->command_meta_data.sequence & ACTION_CONNECT)
    {
        this->command_meta_data.sequence = (this->command_meta_data.sequence & ~ACTION_CONNECT);
        this->ftp->connectToHost(this->command_meta_data.url, this->command_meta_data.port);
    }
    else if(this->command_meta_data.sequence & ACTION_LOGIN)
    {
        this->command_meta_data.sequence = (this->command_meta_data.sequence & ~ACTION_LOGIN);
        this->ftp->login(this->command_meta_data.uname, this->command_meta_data.password);
    }
    else if(this->command_meta_data.sequence & ACTION_LIST_FOLDER)
    {
        this->command_meta_data.sequence = (this->command_meta_data.sequence & ~ACTION_LIST_FOLDER);

        this->navigationPane->top()->findChild<GroupDataModel *>("contentsData")->clear();

        this->ftp->list(this->command_meta_data.path.join("/"));
    }
    else if(this->command_meta_data.sequence & ACTION_DISCONNECT)
    {
        this->command_meta_data.sequence = (this->command_meta_data.sequence & ~ACTION_DISCONNECT);
        this->ftp->close();
    }

}


void ApplicationUI::onFtpStateChanged(int state)
{
    if(this->navigationPane->top()->objectName().compare("serverEntryEditPage") == 0)
    {
//        qDebug()<<"State "<<state;
    }
}

void ApplicationUI::onFtpCommandStarted(int cmdId)
{
//    qDebug()<<" Command started "<<cmdId;
}

void ApplicationUI::onFtpCommandFinished(int cmdId, bool error)
{
//    qDebug()<<"Command finished "<<cmdId<<" error "<< error;

    if(error)
    {
        this->command_meta_data.sequence = 0;

        this->command_meta_data.error = error;
        this->command_meta_data.errorString = this->ftp->errorString();
    }

    if(this->command_meta_data.sequence)
    {

        this->startCommand();
    }
    else
    {
        if(this->command_meta_data.sequenceId == SEQUENCE_VERIFY)
        {
            emit this->verificationFinished();
        }

        if(this->command_meta_data.sequenceId == SEQUENCE_LIST_FOLDER)
        {
            emit this->folderListingFinished();
        }
    }
}

