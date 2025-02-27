/* Copyright (C) 2019 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SCREENCASTER_H
#define SCREENCASTER_H

#include <QObject>
#include <QByteArray>
#include <QTimer>
#include <QImage>
#include <QEvent>
#include <QSize>
#include <memory>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#ifdef SAILFISH
#include "recorder.h"
#endif

class ScreenCaster : public QObject
{
    Q_OBJECT
public:
    ScreenCaster(QObject *parent = nullptr);
    ~ScreenCaster();
    bool init();
    void start();
    bool audioEnabled();
    bool writeAudioData(const QByteArray& data);

signals:
#ifdef DESKTOP
    void readNextVideoFrame();
#endif
    void frameError();

private slots:
#ifdef DESKTOP
    void readVideoFrame();
#endif
#ifdef SAILFISH
    void writeVideoData();
    void repaint();
#endif

private:
#ifdef SAILFISH
    QTimer frameTimer;
    QTimer repaintTimer;
    std::unique_ptr<Recorder> recorder;
    QImage bgImg;
    QImage currImg;
    uint32_t currImgTimestamp;
    void saveCurrImg(QEvent *e);
    bool event(QEvent *e);
#endif
    AVPacket in_pkt;
    AVPacket out_pkt;
    AVFormatContext* in_video_format_ctx = nullptr;
    AVCodecContext* in_video_codec_ctx = nullptr;
    AVCodecContext* out_video_codec_ctx = nullptr;
    AVFormatContext* out_format_ctx = nullptr;
    AVFrame* in_frame = nullptr;
    AVFrame* in_frame_s = nullptr;
    SwsContext* video_sws_ctx = nullptr;
    uint8_t* video_outbuf = nullptr;
    //static int read_packet_callback(void *opaque, uint8_t *buf, int buf_size);
    static int write_packet_callback(void *opaque, uint8_t *buf, int buf_size);
    int audio_frame_size = 0; // 0 => audio disabled for screen casting
    int video_framerate = 0;
    QSize video_size;
    AVCodecContext* in_audio_codec_ctx = nullptr;
    AVCodecContext* out_audio_codec_ctx = nullptr;
    SwrContext* audio_swr_ctx = nullptr;
    QByteArray audio_outbuf; // pulse audio data buf
};

#endif // SCREENCASTER_H
