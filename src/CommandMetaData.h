/*
 * CommandMetaData.h
 *
 *  Created on: Feb 27, 2021
 *      Author: aranade
 */

#ifndef COMMANDMETADATA_H_
#define COMMANDMETADATA_H_

#include <stdint.h>

#include <QStringList>
#include <QFile>

#define ACTION_CONNECT              (0x00000001U)
#define ACTION_LOGIN                (0x00000002U)
#define ACTION_DISCONNECT           (0x00000004U)
#define ACTION_LIST_FOLDER          (0x00000010U)
#define ACTION_DOWNLOAD_FILE        (0x00000020U)
#define ACTION_UPLOAD_FILE          (0x00000040U)
#define ACTION_CD_WORKING_DIR       (0x00000080U)
#define ACTION_CLOSE_IO_DEV         (0x00000100U)
#define ACTION_DELETE_DIR           (0x00000200U)
#define ACTION_DELETE_FILE          (0x00000400U)

class CommandMetaData
{
public:
    CommandMetaData();
    virtual ~CommandMetaData();

    enum SEQUENCE_ID
        {
        SEQUENCE_NONE,
        SEQUENCE_VERIFY,
        SEQUENCE_LIST_FOLDER,
        SEQUENCE_UPLOAD_FILE,
        SEQUENCE_DOWNLOAD_FILE,
        SEQUENCE_CLOSE
        };

    void initCommandMetaData();

    void setUrl(QString);
    QString getUrl();

    void setUname(QString);
    QString getUName();

    void setPassword(QString);
    QString getPassword();

    void setPort(int32_t);
    int32_t getPort();

    void appendToListPath(QString);
    void removeFromListPath();
    QString getLastFromListPath();
    QString getListPath();
    bool listingRootFolder();

    void addActiontoSequence(uint32_t);
    void removeActionFromSequence(uint32_t);
    bool isActionSet(uint32_t);
    bool isSequenceEmpty();

    void setSequenceId(SEQUENCE_ID);

    bool openDestFile(QString path);
    bool openSourceFile(QString  path);
    QFile *getIoDevice();
    void closeIoDevice();

    void setFileName(QString);
    QString getFileName();
private:
    /* Specific to the server
     * This values shall be initialised when logging in to the server
     * ie. change from server list page to content list page */
    QString url;
    QString uname;
    QString password;
    int32_t port;

    /* Values for ftp commands
     * Shall be set each time before a command is started */
    SEQUENCE_ID sequenceId;
    uint32_t sequence;
    bool error;
    QString errorString;
    QStringList listPath;

    /* Value for the get command */
    QFile *ioDevice;
    QString fileName;
};

#endif /* COMMANDMETADATA_H_ */
