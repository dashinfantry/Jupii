/* Copyright (C) 2017 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CONTENTSERVER_H
#define CONTENTSERVER_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QHash>
#include <QVector>
#include <QStringList>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>
#include <QMutex>
#include <QIODevice>
#include <QAudioInput>
#include <memory>

#include <qhttpserver.h>
#include <qhttprequest.h>
#include <qhttpresponse.h>

#include "taskexecutor.h"

#ifdef FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
}
#endif

class ContentServerWorker;
class MicDevice;

class ContentServer :
        public QThread
{
friend class ContentServerWorker;
    Q_OBJECT
public:
    enum Type {
        TypeUnknown = 0,
        TypeImage = 1,
        TypeMusic = 2,
        TypeVideo = 4,
        TypeDir = 128,
        TypePlaylist = 256
    };

    enum PlaylistType {
        PlaylistUnknown,
        PlaylistPLS,
        PlaylistM3U,
        PlaylistXSPF
    };

    struct ItemMeta {
        bool valid = false;
        QString trackerId;
        QUrl url;
        QString path;
        QString filename;
        QString title;
        QString mime;
        QString comment;
        QString album;
        QString albumArt;
        QString artist;
        Type type = TypeUnknown;
        bool local = true;
        bool seekSupported = true;
        int duration = 0;
        double bitrate = 0.0;
        double sampleRate = 0.0;
        int channels = 0;
        int64_t size = 0;
    };

    struct PlaylistItemMeta {
        QUrl url;
        QString title;
        int length = 0;
    };

    const static int micSampleRate = 44100;
    const static int micChannelCount = 1;
    const static int micSampleSize = 16;

    static ContentServer* instance(QObject *parent = nullptr);
    static Type typeFromMime(const QString &mime);
    static QUrl idUrlFromUrl(const QUrl &url, bool* ok = nullptr, bool* isFile = nullptr, bool *isArt = nullptr);
    static QString bestName(const ItemMeta &meta);
    static Type getContentTypeByExtension(const QString &path);
    static Type getContentTypeByExtension(const QUrl &url);
    static PlaylistType playlistTypeFromMime(const QString &mime);
    static PlaylistType playlistTypeFromExtension(const QString &path);
    static QList<PlaylistItemMeta> parsePls(const QByteArray &data, const QString context = QString());
    static QList<PlaylistItemMeta> parseM3u(const QByteArray &data, const QString context = QString());
    static QList<PlaylistItemMeta> parseXspf(const QByteArray &data, const QString context = QString());

    bool getContentUrl(const QString &id, QUrl &url, QString &meta, QString cUrl = "");
    Type getContentType(const QString &path);
    Type getContentType(const QUrl &url);
    QString getContentMime(const QString &path);
    QString getContentMime(const QUrl &url);
    Q_INVOKABLE QStringList getExtensions(int type) const;
    Q_INVOKABLE QString idFromUrl(const QUrl &url) const;
    Q_INVOKABLE QString pathFromUrl(const QUrl &url) const;
    Q_INVOKABLE QString urlFromUrl(const QUrl &url) const;
    const QHash<QUrl, ItemMeta>::const_iterator getMetaCacheIterator(const QUrl &url, bool createNew = true);
    const QHash<QUrl, ItemMeta>::const_iterator getMetaCacheIteratorForId(const QUrl &id, bool createNew = true);
    const QHash<QUrl, ItemMeta>::const_iterator metaCacheIteratorEnd();
    const ItemMeta* getMeta(const QUrl &url, bool createNew = true);
    const ItemMeta* getMetaForId(const QUrl &id, bool createNew = true);
    Q_INVOKABLE QString streamTitle(const QUrl &id) const;

signals:
    void streamTitleChanged(const QUrl &id, const QString &title);

private slots:
    void shoutcastMetadataHandler(const QUrl &id, const QByteArray &metadata);
    void proxyItemAddedHandler(const QUrl &id);
    void proxyItemRemovedHandler(const QUrl &id);

private:
    enum DLNA_ORG_FLAGS {
      DLNA_ORG_FLAG_NONE                       = 0,
      DLNA_ORG_FLAG_SENDER_PACED               = (1 << 31),
      DLNA_ORG_FLAG_TIME_BASED_SEEK            = (1 << 30),
      DLNA_ORG_FLAG_BYTE_BASED_SEEK            = (1 << 29),
      DLNA_ORG_FLAG_PLAY_CONTAINER             = (1 << 28),
      DLNA_ORG_FLAG_S0_INCREASE                = (1 << 27),
      DLNA_ORG_FLAG_SN_INCREASE                = (1 << 26),
      DLNA_ORG_FLAG_RTSP_PAUSE                 = (1 << 25),
      DLNA_ORG_FLAG_STREAMING_TRANSFER_MODE    = (1 << 24),
      DLNA_ORG_FLAG_INTERACTIVE_TRANSFERT_MODE = (1 << 23),
      DLNA_ORG_FLAG_BACKGROUND_TRANSFER_MODE   = (1 << 22),
      DLNA_ORG_FLAG_CONNECTION_STALL           = (1 << 21),
      DLNA_ORG_FLAG_DLNA_V15                   = (1 << 20)
    };

    struct AvData {
        QString path;
        QString mime;
        QString type;
        QString extension;
        int bitrate;
        int channels;
        int64_t size;
    };

    struct StreamData {
        QUrl id;
        QString title;
    };

    static ContentServer* m_instance;

    static const QHash<QString,QString> m_imgExtMap;
    static const QHash<QString,QString> m_musicExtMap;
    static const QHash<QString,QString> m_videoExtMap;
    static const QHash<QString,QString> m_playlistExtMap;
    static const QStringList m_m3u_mimes;
    static const QStringList m_pls_mimes;
    static const QStringList m_xspf_mimes;
    static const QString queryTemplate;
    static const QString dlnaOrgOpFlagsSeekBytes;
    static const QString dlnaOrgOpFlagsNoSeek;
    static const QString dlnaOrgCiFlags;
    static const QString audioItemClass;
    static const QString videoItemClass;
    static const QString imageItemClass;
    static const QString playlistItemClass;
    static const QString broadcastItemClass;
    static const QString defaultItemClass;
    static const QByteArray userAgent;
    static const QString artCookie;
    static const qint64 qlen = 100000;
    static const int threadWait = 1;
    static const int maxRedirections = 5;
    static const int httpTimeout = 10000;

    QHash<QUrl, ItemMeta> metaCache; // url => ItemMeta
    QHash<QUrl, StreamData> streams; // id => StreamData
    ContentServerWorker* worker;

    QMutex metaCacheMutex;

    static QByteArray encrypt(const QByteArray& data);
    static QByteArray decrypt(const QByteArray& data);
    static bool makeUrl(const QString& id, QUrl& url);
    static QString dlnaOrgFlagsForFile();
    static QString dlnaOrgFlagsForStreaming();
    static QString dlnaOrgPnFlags(const QString& mime);
    static QString dlnaContentFeaturesHeader(const QString& mime, bool seek = true, bool flags = true);
    static QString getContentMimeByExtension(const QString &path);
    static QString getContentMimeByExtension(const QUrl &url);
    static QString mimeFromDisposition(const QString &disposition);

    ContentServer(QObject *parent = nullptr);
    bool getContentMeta(const QString &id, const QUrl &url, QString &meta);
    void requestHandler(QHttpRequest *req, QHttpResponse *resp);
    const QHash<QUrl, ItemMeta>::const_iterator makeItemMeta(const QUrl &url);
    const QHash<QUrl, ItemMeta>::const_iterator makeMicItemMeta(const QUrl &url);
    const QHash<QUrl, ItemMeta>::const_iterator makeItemMetaUsingTracker(const QUrl &url);
    const QHash<QUrl, ItemMeta>::const_iterator makeItemMetaUsingTaglib(const QUrl &url);
    const QHash<QUrl, ItemMeta>::const_iterator makeItemMetaUsingHTTPRequest(const QUrl &url,
            std::shared_ptr<QNetworkAccessManager> nam = std::shared_ptr<QNetworkAccessManager>(),
            int counter = 0);
    //const QHash<QUrl, ItemMeta>::const_iterator makeItemMetaUsingExtension(const QUrl &url);
    ItemMeta *makeMetaUsingExtension(const QUrl &url);
    void fillCoverArt(ItemMeta& item);
    //QString makePlaylistForUrl(const QUrl &url);
    void run();
#ifdef FFMPEG
    static bool extractAudio(const QString& path, ContentServer::AvData& data);
    static bool fillAvDataFromCodec(const AVCodecParameters* codec, const QString &videoPath, AvData &data);
#endif
};

class ContentServerWorker :
        public QObject
{
    Q_OBJECT
    friend MicDevice;
public:
    QHttpServer* server;
    QNetworkAccessManager* nam;
    ContentServerWorker(QObject *parent = nullptr);

signals:
    void shoutcastMetadataReceived(const QUrl &id, const QByteArray &metadata);
    void proxyItemAdded(const QUrl &id);
    void proxyItemRemoved(const QUrl &id);

private slots:
    void proxyMetaDataChanged();
    void proxyRedirected(const QUrl &url);
    void proxyFinished();
    void proxyReadyRead();
    void startMic();
    void stopMic();
    void responseDone();

private:
    struct ProxyItem {
        QHttpRequest* req = nullptr;
        QHttpResponse* resp = nullptr;
        QNetworkReply* reply = nullptr;
        QUrl id;
        bool seek = false;
        int state = 0;
        bool meta = false; // shoutcast metadata requested by client
        int metaint = 0; // shoutcast metadata interval received from server
        int metacounter = 0; // bytes couter reseted every metaint
        //int metasize = 0; // shoutcast metadata size detected
        //QByteArray metadata; // shoutcast metadata readed so far
        QByteArray data;
    };

    struct MicItem {
        QHttpRequest* req = nullptr;
        QHttpResponse* resp = nullptr;
    };

    std::unique_ptr<QAudioInput> micInput;
    std::unique_ptr<MicDevice> micDev;

    QHash<QNetworkReply*, ProxyItem> proxyItems;
    QHash<QHttpResponse*, QNetworkReply*> responseToReplyMap;
    QList<MicItem> micItems;

    void streamFile(const QString& path, const QString &mime, QHttpRequest *req, QHttpResponse *resp);
    bool seqWriteData(QFile &file, qint64 size, QHttpResponse *resp);
    void requestHandler(QHttpRequest *req, QHttpResponse *resp);
    void requestForFileHandler(const QUrl &id, const ContentServer::ItemMeta *meta, QHttpRequest *req, QHttpResponse *resp);
    void requestForUrlHandler(const QUrl &id, const ContentServer::ItemMeta *meta, QHttpRequest *req, QHttpResponse *resp);
    void requestForMicHandler(const QUrl &id, const ContentServer::ItemMeta *meta, QHttpRequest *req, QHttpResponse *resp);
    void sendEmptyResponse(QHttpResponse *resp, int code);
    void sendResponse(QHttpResponse *resp, int code, const QByteArray &data = "");
    void sendRedirection(QHttpResponse *resp, const QString &location);
    void processShoutcastMetadata(QByteArray &data, ProxyItem &item);
};

class MicDevice : public QIODevice
{
    Q_OBJECT

public:
    MicDevice(ContentServerWorker* worker, QObject *parent = nullptr);
    void setActive(bool value);
    bool isActive();

protected:
    qint64 readData(char *data, qint64 maxSize);
    qint64 writeData(const char *data, qint64 maxSize);

private:
    bool active = false;
    ContentServerWorker *worker;
};

#endif // CONTENTSERVER_H
