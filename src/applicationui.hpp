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
#include <bb/cascades/ListView>
#include <bb/cascades/Label>

#include <bb/system/InvokeManager>
#include <bb/system/InvokeRequest>

#include <bb/device/DisplayInfo>


using namespace bb::cascades;

#define ACCOUNT_INFO_FILE_PATH      "/app/native/assets/json/accounts.json"
#define FTP_PROTOCOL    0
#define SFTP_PROTOCOL   1

#define SEQUENCE_VERIFY             0
#define SEQUENCE_LIST_FOLDER        10
#define SEQUENCE_UPLOAD_FILE        11
#define SEQUENCE_DOWNLOAD_FILE      12

#define ACTION_CONNECT              (0x00000001U)
#define ACTION_LOGIN                (0x00000002U)
#define ACTION_DISCONNECT           (0x00000004U)


struct command_meta_data_t{
    uint32_t sequenceId;
    uint32_t sequence;
    QString url;
    QString uname;
    QString password;
    int32_t port;
    QString path;
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
    virtual ~ApplicationUI() {}

public slots:
    void onInvoke(const bb::system::InvokeRequest&);
private slots:
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
    void serverConnTestFinished();
    void onServerTriggered(QVariantList);
    void onProtocolSelected(int);

signals:
    void verificationFinished();

private:
void renderServerListPage(bb::cascades::Page*, bool);
void renderAddServerPage(bb::cascades::Page*, bool, int);
void renderContentsPage(bb::cascades::Page*);
int32_t readAccountInfo();
void saveAccountInfo();
void createFtpInstance();
void queueFtpCommands(int);
void startCommand();
void initCommandMetaData();
struct command_meta_data_t command_meta_data;

QVariantListDataModel listViewDataModel;
NavigationPane* navigationPane;
Page *rootPage;
ListView *list;
Label *label;
QFtp *ftp;
bb::system::InvokeManager *invokemanager;
bb::device::DisplayInfo *displayInfo;
};

#endif /* ApplicationUI_HPP_ */
