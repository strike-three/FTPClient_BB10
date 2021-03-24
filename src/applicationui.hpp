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

#ifndef ApplicationUI_HPP_
#define ApplicationUI_HPP_

#include <stdint.h>

#include <QObject>

#include <QtNetwork/QFtp>

#include <bb/cascades/Page>
#include <bb/cascades/NavigationPane>

#include <bb/cascades/QListDataModel>
#include <bb/cascades/GroupDataModel>
#include <bb/cascades/ListView>
#include <bb/cascades/Label>
#include <bb/cascades/Menu>
#include <bb/cascades/ActionItem>

#include <bb/system/InvokeManager>
#include <bb/system/InvokeRequest>
#include <bb/system/SystemPrompt>
#include <bb/system/SystemUiResult>
#include <bb/system/SystemToast>
#include <bb/system/SystemProgressDialog>

#include <bb/device/DisplayInfo>

#include <bb/system/SystemDialog>

#include <src/CommandMetaData.h>

using namespace bb::cascades;

#define ACCOUNT_INFO_FILE_PATH      "/data/accounts.json"
#define FIRST_TIME_FLAG_PATH        "/data/firstTime"
#define DARK_THEME_FLAG_PATH        "/data/darkTheme"

#define FTP_PROTOCOL    0
#define SFTP_PROTOCOL   1

#define SEQUENCE_VERIFY             0
#define SEQUENCE_LIST_FOLDER        10
#define SEQUENCE_UPLOAD_FILE        11
#define SEQUENCE_DOWNLOAD_FILE      12
#define SEQUENCE_CLOSE              20
#define SEQUENCE_DELETE_FILE        31
#define SEQUENCE_DELETE_DIR         32
#define SEQUENCE_RENAME             33
#define SEQUENCE_MKDIR              34


struct command_meta_data_t{
    uint32_t sequenceId;
    bool error;
    QString errorString;
};

/*!
 * @brief Application UI object
 *
 * Use this object to create and init app UI, to create context objects, to register the new meta types etc.
 */
class ApplicationUI : public QObject
{
    Q_OBJECT
public:
    ApplicationUI();
    ~ApplicationUI();

public slots:
    void onInvoke(const bb::system::InvokeRequest&);
private slots:
    void onToggleTheme();
    void addServerPressed();
    void pushFinished(bb::cascades::Page*);
    void popFinished(bb::cascades::Page*);
    void onServerEntryEdit();
    void onServerEntryDelete();
    void onServerSave();
    void onServerConnTest();
    void onFtpStateChanged(int);
    void onFtpCommandStarted(int);
    void onFtpCommandFinished(int, bool);
    void onServerConnTestFinished();
    void onServerTriggered(QVariantList);
    void onProtocolSelected(int);
    void onListInfo(const QUrlInfo&);
    void onContentItemTriggered(QVariantList);
    void onCustomBackButton();
    void onItemDownload();
    void onDataTransferProgress(qint64 done, qint64 total);
    void onDownloaDestSelected(const QStringList&);
    void onDownloadCanceled();
    void onItemUpload();
    void onUploadFileSelected(const QStringList&);
    void onUploadCanceled();
    void onFolderRefresh();
    void onCardItemUpload();
    void onCardCancel();
    void onSelectionContentChanged(QVariantList, bool);
    void onContentItemDelete();
    void onItemRename();
    void onRenamePromtFinished(bb::system::SystemUiResult::Type);
    void onSysProgressDialogFinished(bb::system::SystemUiResult::Type);
    void onAddFolder();
    void onAddFolderPromtFinished(bb::system::SystemUiResult::Type);
    void onServerPageTextChanged(QString);
    void onRawCommandReply(int, QString);

signals:
    void verificationFinished();
    void folderListingFinished();

private:
void renderServerListPage(bb::cascades::Page*);
void renderAddServerPage(bb::cascades::Page*, bool, int);
void renderContentsPage(bb::cascades::Page*);
void renderActiveFrame();
int32_t readAccountInfo();
void saveAccountInfo();
void createFtpInstance();
void startCommand();
void initCommandMetaData();
void initAppTheme();
struct command_meta_data_t command_meta_data;
CommandMetaData *commandMetaData;

//ArrayDataModel contentsData;
QVariantListDataModel listViewDataModel;
NavigationPane* navigationPane;
Page *rootPage;
ListView *list;
Label *label;
QFtp *ftp;
bool card;
QUrl invokeuri;
QVariantList selectedIndex;
Menu *appMenu;
ActionItem *changeTheme;
bb::system::InvokeManager *invokemanager;
bb::device::DisplayInfo *displayInfo;
bb::system::SystemDialog *sysDialog;
bb::system::SystemPrompt *renamePromt;
bb::system::SystemPrompt *addFolderPromt;
bb::system::SystemToast *sysToast;
bb::system::SystemProgressDialog *sysProgressDialog;
};

#endif /* ApplicationUI_HPP_ */
