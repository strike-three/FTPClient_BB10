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

#include <bb/cascades/pickers/FilePicker>

#include <src/ServerListItemFactory.h>
#include <src/ContentListItemFactory.h>

using namespace bb::cascades;

ApplicationUI::ApplicationUI() :
        QObject()
{
    this->rootPage = new Page();
    /* Create and initialise the command meta data */
    this->commandMetaData = new CommandMetaData();

    this->displayInfo = new bb::device::DisplayInfo();
    this->invokemanager = new bb::system::InvokeManager();
    this->sysDialog = new bb::system::SystemDialog(NULL, "Cancel");
    this->sysDialog->setActivityIndicatorVisible(true);
    this->sysDialog->setButtonAreaLimit(1);

    this->ftp = new QFtp();

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

ApplicationUI::~ApplicationUI()
{
    delete this->ftp;
    delete this->sysDialog;
    delete this->label;
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
    qDebug()<<"Save path" <<filePath;
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
        page->deleteLater();
    }

    if(page->objectName().compare("serverEntryEditPage") == 0)
    {
        qDebug()<<"Remove page "<<page->objectName();
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

    ActionSet *contentsListActions = ActionSet::create().title("Actions");

    ActionItem *downloadItem = ActionItem::create()
                                        .title("Download")
                                        .image(Image("asset:///ic_download.amd"))
                                        .onTriggered(this, SLOT(onItemDownload()));

    contentsListActions->add(downloadItem);

    contentsList->addActionSet(contentsListActions);

    /* Set custom back button for the navigation pane */
    ActionItem *customBack = ActionItem::create()
                                .title("Up")
                                .imageSource(QUrl("asset:///ic_previous.png"))
                                .onTriggered(this, SLOT(onCustomBackButton()));

    NavigationPaneProperties *navigationPaneProperties = new NavigationPaneProperties();
    navigationPaneProperties->setBackButton(customBack);
    this->navigationPane->top()->setPaneProperties(navigationPaneProperties);

    page->setContent(contentsListContainer);

    this->createFtpInstance();

    /* Enter command meta data */

    this->commandMetaData->initCommandMetaData();

    this->commandMetaData->setUrl(map["url"].toString());
    this->commandMetaData->setUname(map["uname"].toString());
    this->commandMetaData->setPassword(map["password"].toString());
    this->commandMetaData->setPort(map["port"].toInt());

    this->commandMetaData->appendToListPath(map["startPath"].toString());

    this->command_meta_data.sequenceId = SEQUENCE_LIST_FOLDER;
    this->commandMetaData->addActiontoSequence(ACTION_CONNECT);
    this->commandMetaData->addActiontoSequence(ACTION_LOGIN);
    this->commandMetaData->addActiontoSequence(ACTION_LIST_FOLDER);

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

    /* Enter command meta data */

    this->commandMetaData->initCommandMetaData();

    this->commandMetaData->setUrl(this->navigationPane->top()->findChild<TextField *>("serverUrl")->text());
    this->commandMetaData->setUname(this->navigationPane->top()->findChild<TextField *>("userName")->text());
    this->commandMetaData->setPassword(this->navigationPane->top()->findChild<TextField *>("password")->text());
    this->commandMetaData->setPort(this->navigationPane->top()->findChild<TextField *>("port")->text().toInt());

    this->command_meta_data.sequenceId = SEQUENCE_VERIFY;
    this->commandMetaData->addActiontoSequence(ACTION_CONNECT);
    this->commandMetaData->addActiontoSequence(ACTION_LOGIN);
    this->commandMetaData->addActiontoSequence(ACTION_DISCONNECT);

    bool res = QObject::connect(this, SIGNAL(verificationFinished()), this, SLOT(onServerConnTestFinished()));
    Q_ASSERT(res);

    this->startCommand();
}

void ApplicationUI::onServerConnTestFinished()
{
    /* Clean the command meta data object */
    this->commandMetaData->initCommandMetaData();

    if(this->command_meta_data.error)
    {
        qDebug()<<"Verification failed :" << this->command_meta_data.errorString;
    }
    else
    {
        qDebug()<<"Verification success";
    }
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
//        qDebug()<<contentInfo.isDir()<<" "<<contentInfo.name();
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
        this->commandMetaData->appendToListPath(item["name"].toString());
        this->command_meta_data.sequenceId = SEQUENCE_LIST_FOLDER;
        if(this->ftp->state() < QFtp::LoggedIn)
        {
            this->commandMetaData->addActiontoSequence(ACTION_CONNECT);
            this->commandMetaData->addActiontoSequence(ACTION_LOGIN);
            this->commandMetaData->addActiontoSequence(ACTION_LIST_FOLDER);
        }
        else
        {
            this->commandMetaData->addActiontoSequence(ACTION_LIST_FOLDER);
        }

        this->startCommand();

    }
}

void ApplicationUI::onCustomBackButton()
{

    if(!this->commandMetaData->listingRootFolder())
    {
        this ->commandMetaData->removeFromListPath();
        this->command_meta_data.sequenceId = SEQUENCE_LIST_FOLDER;
        if(this->ftp->state() < QFtp::LoggedIn)
        {
            this->commandMetaData->addActiontoSequence(ACTION_CONNECT);
            this->commandMetaData->addActiontoSequence(ACTION_LOGIN);
            this->commandMetaData->addActiontoSequence(ACTION_LIST_FOLDER);
        }
        else
        {
            this->commandMetaData->addActiontoSequence(ACTION_LIST_FOLDER);
        }
        this->startCommand();
    }
    else
    {
        this->command_meta_data.sequenceId = SEQUENCE_CLOSE;
        this->commandMetaData->addActiontoSequence(ACTION_DISCONNECT);
        this->startCommand();
    }
}

void ApplicationUI::onDataTransferProgress(qint64 done, qint64 total)
{
    qDebug()<<"Transfered "<<done<<" of "<<total;
}

void ApplicationUI::onItemDownload()
{
    QVariantList index = this->navigationPane->top()->findChild<ListView *>("contentsList")->selected();
    QVariantMap map = this->navigationPane->top()->findChild<GroupDataModel *>("contentsData")->data(index).toMap();

    if(map["type"].toString().compare("File") == 0)
    {
        this->commandMetaData->appendToListPath(map["name"].toString());
        this->command_meta_data.sequenceId = SEQUENCE_DOWNLOAD_FILE;
        if(this->ftp->state() < QFtp::LoggedIn)
        {
            this->commandMetaData->addActiontoSequence(ACTION_CONNECT);
            this->commandMetaData->addActiontoSequence(ACTION_LOGIN);
            this->commandMetaData->addActiontoSequence(ACTION_DOWNLOAD_FILE);
        }
        else
        {
            this->commandMetaData->addActiontoSequence(ACTION_DOWNLOAD_FILE);
        }

        bb::cascades::pickers::FilePicker *filePicker = new bb::cascades::pickers::FilePicker(
                                                            bb::cascades::pickers::FileType::Document,
                                                        0, QStringList(), QStringList(), QStringList(map["name"].toString()));
        filePicker->setMode(bb::cascades::pickers::FilePickerMode::Saver);
        filePicker->open();

        bool res = QObject::connect(filePicker, SIGNAL(fileSelected(const QStringList&)),
                                    this, SLOT(onDownloaDestSelected(const QStringList&)));

        Q_ASSERT(res);

        res = QObject::connect(filePicker, SIGNAL(canceled()),
                                this, SLOT(onDownloadCanceled()));

        Q_ASSERT(res);
    }

}

void ApplicationUI::onDownloadCanceled()
{
    this->commandMetaData->removeFromListPath();
}

void ApplicationUI::onDownloaDestSelected(const QStringList& dest)
{
    if(this->commandMetaData->openDestFile(dest.at(0)))
    {
        this->startCommand();
    }
    else
    {
        /* Error opening file */
        this->commandMetaData->removeFromListPath();
    }

}
/*****************************************************************************
 *                  FTP methods
 * ***************************************************************************/

void ApplicationUI::initCommandMetaData()
{
    this->command_meta_data.sequenceId = 0;
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

    res = QObject::connect(ftp, SIGNAL(dataTransferProgress(qint64, qint64)),
                                this, SLOT(onDataTransferProgress(qint64, qint64)));
    Q_ASSERT(res);

}

void ApplicationUI::startCommand()
{

    if(this->commandMetaData->isActionSet(ACTION_CONNECT))
    {
        this->commandMetaData->removeActionFromSequence(ACTION_CONNECT);
        this->ftp->connectToHost(this->commandMetaData->getUrl(), this->commandMetaData->getPort());
    }
    else if(this->commandMetaData->isActionSet(ACTION_LOGIN))
    {
        this->commandMetaData->removeActionFromSequence(ACTION_LOGIN);
        this->ftp->login(this->commandMetaData->getUName(), this->commandMetaData->getPassword());
    }
    else if(this->commandMetaData->isActionSet(ACTION_LIST_FOLDER))
    {
        this->commandMetaData->removeActionFromSequence(ACTION_LIST_FOLDER);

        this->navigationPane->top()->findChild<GroupDataModel *>("contentsData")->clear();

        this->ftp->list(this->commandMetaData->getListPath());
    }
    else if(this->commandMetaData->isActionSet(ACTION_DOWNLOAD_FILE))
    {
        this->commandMetaData->removeActionFromSequence(ACTION_DOWNLOAD_FILE);

        this->ftp->get(this->commandMetaData->getListPath(),
                        this->commandMetaData->getIoDevice());
    }
    else if(this->commandMetaData->isActionSet(ACTION_DISCONNECT))
    {
        this->commandMetaData->removeActionFromSequence(ACTION_DISCONNECT);
        this->ftp->close();
    }

}


void ApplicationUI::onFtpStateChanged(int state)
{
    Q_UNUSED(state);
    if(this->navigationPane->top()->objectName().compare("serverEntryEditPage") == 0)
    {
//        qDebug()<<"State "<<state;
    }
}

void ApplicationUI::onFtpCommandStarted(int cmdId)
{
    Q_UNUSED(cmdId);
    qDebug()<<"Command " << this->ftp->currentCommand();
    switch(this->ftp->currentCommand())
    {
        case QFtp::ConnectToHost:
            this->sysDialog->setBody("Connect to host");
            break;

        case QFtp::Login:
            this->sysDialog->setBody("Logging in..");
            break;

        case QFtp::Close:
            this->sysDialog->setBody("Logging out..");
            break;

        case QFtp::List:
            this->sysDialog->setBody("Getting folder contents..");
            break;

        case QFtp::Get:
            this->sysDialog->setBody("Downloading..");
            break;

        default:
            this->sysDialog->setBody("Unknown command");
            break;
    }
    this->sysDialog->show();
}

void ApplicationUI::onFtpCommandFinished(int cmdId, bool error)
{
    Q_UNUSED(cmdId);
    Q_UNUSED(error);
//    qDebug()<<"Command finished "<<cmdId<<" error "<< error;

    if(error)
    {

        this->command_meta_data.error = error;
        this->command_meta_data.errorString = this->ftp->errorString();
    }

    if(!this->commandMetaData->isSequenceEmpty())
    {

        this->startCommand();
    }
    else
    {
        this->sysDialog->cancel();

        if(this->command_meta_data.sequenceId == SEQUENCE_VERIFY)
        {
            emit this->verificationFinished();
        }

        if(this->command_meta_data.sequenceId == SEQUENCE_LIST_FOLDER)
        {
            emit this->folderListingFinished();
        }

        if(this->command_meta_data.sequenceId == SEQUENCE_CLOSE)
        {
            this->navigationPane->pop();
        }

        if(this->command_meta_data.sequenceId == SEQUENCE_DOWNLOAD_FILE)
        {
            this->commandMetaData->removeFromListPath();
            this->ftp->currentDevice()->close();
        }
    }
}

