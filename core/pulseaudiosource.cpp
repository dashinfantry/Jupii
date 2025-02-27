/* Copyright (C) 2019 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pulseaudiosource.h"

#include <QDebug>
#include <cstddef>

#include "info.h"
#include "contentserver.h"

int PulseAudioSource::nullDataSize = 0;
bool PulseAudioSource::started = false;
bool PulseAudioSource::shutdown = false;
const int PulseAudioSource::timerDelta = 1000/25; // msec, must be divided by 4
bool PulseAudioSource::timerActive = false;
bool PulseAudioSource::muted = false;
pa_sample_spec PulseAudioSource::sampleSpec = {PA_SAMPLE_S16LE, 44100u, 2};
pa_stream* PulseAudioSource::stream = nullptr;
uint32_t PulseAudioSource::connectedSinkInput = PA_INVALID_INDEX;
pa_mainloop* PulseAudioSource::ml = nullptr;
pa_mainloop_api* PulseAudioSource::mla = nullptr;
pa_context* PulseAudioSource::ctx = nullptr;
QHash<uint32_t, PulseAudioSource::Client> PulseAudioSource::clients = QHash<uint32_t, PulseAudioSource::Client>();
QHash<uint32_t, PulseAudioSource::SinkInput> PulseAudioSource::sinkInputs = QHash<uint32_t, PulseAudioSource::SinkInput>();

PulseAudioSource::PulseAudioSource(QObject *parent) : QObject(parent)
{
    iterationTimer.setInterval(PulseAudioSource::timerDelta);
    iterationTimer.setSingleShot(true);
    connect(&iterationTimer, &QTimer::timeout,
            this, &PulseAudioSource::doPulseIteration, Qt::QueuedConnection);
}

PulseAudioSource::~PulseAudioSource()
{
    deinit();
}

QString PulseAudioSource::subscriptionEventToStr(pa_subscription_event_type_t t)
{
    auto facility = t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;
    auto type = t & PA_SUBSCRIPTION_EVENT_TYPE_MASK;

    QString facility_str("UNKNOWN");

    switch (facility) {
    case PA_SUBSCRIPTION_EVENT_SINK: facility_str = "SINK"; break;
    case PA_SUBSCRIPTION_EVENT_SOURCE: facility_str = "SOURCE"; break;
    case PA_SUBSCRIPTION_EVENT_SINK_INPUT: facility_str = "SINK_INPUT"; break;
    case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT: facility_str = "SOURCE_OUTPUT"; break;
    case PA_SUBSCRIPTION_EVENT_MODULE: facility_str = "SOURCE_OUTPUT"; break;
    case PA_SUBSCRIPTION_EVENT_CLIENT: facility_str = "CLIENT"; break;
    case PA_SUBSCRIPTION_EVENT_SAMPLE_CACHE: facility_str = "SAMPLE_CACHE"; break;
    case PA_SUBSCRIPTION_EVENT_SERVER: facility_str = "SERVER"; break;
    case PA_SUBSCRIPTION_EVENT_AUTOLOAD: facility_str = "AUTOLOAD"; break;
    case PA_SUBSCRIPTION_EVENT_CARD: facility_str = "CARD"; break;
    }

    QString type_str("UNKNOWN");

    switch (type) {
    case PA_SUBSCRIPTION_EVENT_NEW: type_str = "NEW"; break;
    case PA_SUBSCRIPTION_EVENT_CHANGE: type_str = "CHANGE"; break;
    case PA_SUBSCRIPTION_EVENT_REMOVE: type_str = "REMOVE"; break;
    }

    return facility_str + " " + type_str;
}

void PulseAudioSource::subscriptionCallback(pa_context *ctx, pa_subscription_event_type_t t, uint32_t idx, void *userdata)
{
    Q_ASSERT(ctx);
    Q_UNUSED(userdata);

    qDebug() << "Pulse-audio subscriptionCallback:" << subscriptionEventToStr(t) << idx;

    auto facility = t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;
    auto type = t & PA_SUBSCRIPTION_EVENT_TYPE_MASK;

    if (facility == PA_SUBSCRIPTION_EVENT_SINK_INPUT) {
        if (type == PA_SUBSCRIPTION_EVENT_NEW || type == PA_SUBSCRIPTION_EVENT_CHANGE) {
            pa_operation_unref(pa_context_get_sink_input_info(ctx, idx, sinkInputInfoCallback, nullptr));
        } else if (type == PA_SUBSCRIPTION_EVENT_REMOVE) {
            qDebug() << "Removing pulse-audio sink input:" << idx;
            sinkInputs.remove(idx);
            discoverStream();
        }
    } else if (facility == PA_SUBSCRIPTION_EVENT_CLIENT) {
        if (type == PA_SUBSCRIPTION_EVENT_NEW || type == PA_SUBSCRIPTION_EVENT_CHANGE) {
            pa_operation_unref(pa_context_get_client_info(ctx, idx, clientInfoCallback, nullptr));
        } else if (type == PA_SUBSCRIPTION_EVENT_REMOVE) {
            qDebug() << "Removing pulse-audio client:" << idx;
            clients.remove(idx);
        }
    }
}

void PulseAudioSource::successSubscribeCallback(pa_context *ctx, int success, void *userdata)
{
    Q_ASSERT(ctx);
    Q_UNUSED(userdata);

    if (success) {
         pa_operation_unref(pa_context_get_client_info_list(ctx, clientInfoCallback, nullptr));
         pa_operation_unref(pa_context_get_sink_input_info_list(ctx, sinkInputInfoCallback, nullptr));
    }
}

void PulseAudioSource::stateCallback(pa_context *ctx, void *userdata)
{
    Q_ASSERT(ctx);
    Q_UNUSED(userdata);

    switch (pa_context_get_state(ctx)) {
        case PA_CONTEXT_CONNECTING:
            qDebug() << "Pulse-audio connecting";
            break;
        case PA_CONTEXT_AUTHORIZING:
            qDebug() << "Pulse-audio authorizing";
            break;
        case PA_CONTEXT_SETTING_NAME:
            qDebug() << "Pulse-audio setting name";
            break;
        case PA_CONTEXT_READY: {
            qDebug() << "Pulse-audio ready";
            auto mask = static_cast<pa_subscription_mask_t>(
                        PA_SUBSCRIPTION_MASK_SINK_INPUT|
                        PA_SUBSCRIPTION_MASK_CLIENT);
            pa_operation_unref(pa_context_subscribe(
                                   ctx, mask, successSubscribeCallback, nullptr));
            break;
        }
        case PA_CONTEXT_TERMINATED:
            qDebug() << "Pulse-audio terminated";
            break;
        case PA_CONTEXT_FAILED:
            qDebug() << "Pulse-audio failed";
            break;
        default:
            qDebug() << "Pulse-audio connection failure: " << pa_strerror(pa_context_errno(ctx));
    }
}

void PulseAudioSource::streamRequestCallback(pa_stream *stream, size_t nbytes, void *userdata)
{
    Q_ASSERT(stream);
    Q_UNUSED(userdata);

    if (nbytes <= 0) {
        qWarning() << "Pulse-audio stream nbytes <= 0";
        return;
    }

    const void *data;
    if (pa_stream_peek(stream, &data, &nbytes) < 0) {
        qWarning() << "Pulse-audio stream peek failed";
        return;
    }

    if (!data) {
        qWarning() << "Pulse-audio stream peek data is null";
        return;
    }

    if (nbytes <= 0) {
        qWarning() << "Pulse-audio stream peeked nbytes <= 0";
        return;
    }

    if (PulseAudioSource::stream) { // stream is connected
        auto worker = ContentServerWorker::instance();
        worker->dispatchPulseData(data, static_cast<int>(nbytes));
    }

    pa_stream_drop(stream);
}

void PulseAudioSource::stopRecordStream()
{
    if (stream) {
        unmuteConnectedSinkInput();
        qDebug() << "Disconnecting pulse-audio stream";
        pa_stream_disconnect(stream);
        pa_stream_unref(stream);
        stream = nullptr;
        connectedSinkInput = PA_INVALID_INDEX;
    }
}

void PulseAudioSource::contextSuccessCallback(pa_context *ctx, int success, void *userdata)
{
    Q_UNUSED(ctx)
    Q_UNUSED(userdata)
    qDebug() << "contextSuccessCallback:" << success;
}

void PulseAudioSource::muteConnectedSinkInput()
{
#ifdef SAILFISH
    if (!muted && connectedSinkInput != PA_INVALID_INDEX) {
        qDebug() << "Muting sink input by moving it to null sink";
        auto o = pa_context_move_sink_input_by_name(
                    ctx, connectedSinkInput, "sink.null",
                    contextSuccessCallback, nullptr);
        //qDebug() << "o:" << o;
        if (o)
            pa_operation_unref(o);
        muted = true;
    } else {
        qDebug() << "Cannot mute";
    }
#endif
}

void PulseAudioSource::unmuteConnectedSinkInput()
{
#ifdef SAILFISH
    if (connectedSinkInput != PA_INVALID_INDEX &&
            sinkInputs.contains(connectedSinkInput)) {
        qDebug() << "Unmuting sink input by moving it to primary sink";
        auto o = pa_context_move_sink_input_by_name(
                   ctx, connectedSinkInput, "sink.primary",
                   contextSuccessCallback, nullptr);
        //qDebug() << "o:" << o;
        if (o)
            pa_operation_unref(o);
    } else {
        qDebug() << "Cannot unmute";
    }
    muted = false;
#endif
}

bool PulseAudioSource::startRecordStream(pa_context *ctx, uint32_t si)
{
    stopRecordStream();

    qDebug() << "Creating new pulse-audio stream connected to sink input";
    stream = pa_stream_new(ctx, Jupii::APP_NAME, &sampleSpec, nullptr);
    pa_stream_set_read_callback(stream, streamRequestCallback, nullptr);
    connectedSinkInput = si;

    muteConnectedSinkInput();

    if (pa_stream_set_monitor_stream(stream, si) < 0) {
        qWarning() << "Pulse-audio stream set monitor error";
    } else if (pa_stream_connect_record(stream, nullptr, nullptr, PA_STREAM_NOFLAGS) < 0) {
        qWarning() << "Pulse-audio stream connect record error";
    } else {
        qDebug() << "Sink input successfully connected";
        return true;
    }

    // something went wrong, so reseting stream
    pa_stream_disconnect(stream);
    pa_stream_unref(stream);
    unmuteConnectedSinkInput();
    stream = nullptr;
    connectedSinkInput = PA_INVALID_INDEX;

    return false;
}

void PulseAudioSource::sinkInputInfoCallback(pa_context *ctx, const pa_sink_input_info *i, int eol, void *userdata)
{
    Q_ASSERT(ctx);
    Q_UNUSED(userdata);

    if (!eol) {
        qDebug() << "sinkInputInfoCallback:";
        qDebug() << "  index:" << i->index;
        qDebug() << "  name:" << i->name;
        qDebug() << "  client:" << (i->client == PA_INVALID_INDEX ? 0 : i->client);
        qDebug() << "  has_volume:" << i->has_volume;
        qDebug() << "  mute:" << i->mute;
        qDebug() << "  volume.channels:" << i->volume.channels;
        qDebug() << "  volume.values[0]:" << i->volume.values[0];
        qDebug() << "  corked:" << i->corked;
        qDebug() << "  sink:" << i->sink;
        qDebug() << "  sample_spec:" << pa_sample_format_to_string(i->sample_spec.format) << " "
             << i->sample_spec.rate << " "
             << static_cast<uint>(i->sample_spec.channels);
        auto props = pa_proplist_to_string(i->proplist);
        qDebug() << "  props:\n" << props;
        pa_xfree(props);

        auto& si = sinkInputs[i->index];
        si.idx = i->index;
        si.clientIdx = i->client;
        si.name = i->name;
        si.corked = i->corked;
    } else {
        discoverStream();
    }
}

void PulseAudioSource::discoverStream()
{
    if (inited()) {
        auto worker = ContentServerWorker::instance();
        QHash<uint32_t, SinkInput>::const_iterator i;
        for (i = sinkInputs.begin(); i != sinkInputs.end(); ++i) {
            const auto si = i.value();
            if (!si.corked && clients.contains(si.clientIdx)) {
                const auto client = clients.value(si.clientIdx);
                bool needUpdate = false;
                if (connectedSinkInput != si.idx) {
                    qDebug() << "Starting recording for:";
                    qDebug() << "  sink input:" << si.idx << si.name;
                    qDebug() << "  client:" << client.idx << client.name;
                    if (startRecordStream(ctx, si.idx))
                        needUpdate = true;
                } else {
                    qDebug() << "Sink is already connected";
                    needUpdate = true;
                }

                if (needUpdate) {
                    qDebug() << "Updating stream name to name of sink input's client:" << client.name;
                    worker->updatePulseStreamName(client.name);
                } else {
                    qDebug() << "Reseting stream name";
                    worker->updatePulseStreamName(QString());
                }

                return;
            }
        }

        qDebug() << "No proper pulse-audio sink found";
        worker->updatePulseStreamName(QString());
        stopRecordStream();
    } else {
        qWarning() << "Pulse-audio is not inited";
    }
}

QList<PulseAudioSource::Client> PulseAudioSource::activeClients()
{
    QList<PulseAudioSource::Client> list;

    for (auto ci : clients.keys()) {
        for (auto& s : sinkInputs.values()) {
            if (s.clientIdx == ci) {
                list << clients[ci];
                break;
            }
        }
    }

    return list;
}

bool PulseAudioSource::isBlacklisted(const char* name)
{
#ifdef SAILFISH
    if (!strcmp(name, "ngfd") ||
        !strcmp(name, "feedback-event") ||
        !strcmp(name, "keyboard_0") ||
        !strcmp(name, "keyboard_1") ||
        !strcmp(name, "ngf-tonegen-plugin") ||
        !strcmp(name, "jolla keyboard")) {
        return true;
    }
#else // SAILFISH
    if (!strcmp(name, "speech-dispatcher-dummy")
#ifdef QT_DEBUG
        || !strcmp(name, "Kodi")
#endif
    ) {
        return true;
    }
#endif // SAILFISH
    return false;
}

void PulseAudioSource::correctClientName(Client &client)
{
#ifdef SAILFISH
    if (client.name == "CubebUtils" && !client.binary.isEmpty()) {
        client.name = client.binary;
    } else if (client.name == "aliendalvik_audio_glue") {
        client.name = "Android";
    }
#else
    Q_UNUSED(client)
#endif
}

void PulseAudioSource::clientInfoCallback(pa_context *ctx, const pa_client_info *i,
                                          int eol, void *userdata)
{
    Q_ASSERT(ctx);
    Q_UNUSED(userdata);

    if (!eol) {
        qDebug() << "clientInfoCallback:";
        qDebug() << "  index:" << i->index;
        qDebug() << "  name:" << i->name;
        auto props = pa_proplist_to_string(i->proplist);
        qDebug() << "  props:\n" << props;
        pa_xfree(props);

        if (!isBlacklisted(i->name)) {
            auto& client = clients[i->index];
            client.idx = i->index;

            client.name = QString::fromLatin1(i->name);

            if (pa_proplist_contains(i->proplist, PA_PROP_APPLICATION_PROCESS_BINARY)) {
                const void* data; size_t size;
                if (pa_proplist_get(i->proplist, PA_PROP_APPLICATION_PROCESS_BINARY, &data, &size) >= 0) {
                    client.binary = QString::fromUtf8(static_cast<const char*>(data),
                                                      static_cast<int>(size) - 1);
                }
            }
            if (pa_proplist_contains(i->proplist, PA_PROP_APPLICATION_ICON_NAME)) {
                const void* data; size_t size;
                if (pa_proplist_get(i->proplist, PA_PROP_APPLICATION_ICON_NAME, &data, &size) >= 0) {
                    client.icon = QString::fromUtf8(static_cast<const char*>(data),
                                                    static_cast<int>(size) - 1);
                }
            }
            correctClientName(client);
        } else {
            qDebug() << "Client blacklisted";
            clients.remove(i->index);
        }
    }
}

bool PulseAudioSource::inited()
{
    return ml && mla && ctx;
}

void PulseAudioSource::deinit()
{
    qDebug() << "Deiniting pulse audio client";
    started = false;
    if (ctx) {
        pa_context_disconnect(ctx);
        pa_context_unref(ctx);
        ctx = nullptr;
    }
    if (ml) {
        pa_signal_done();
        pa_mainloop_free(ml);
        ml = nullptr;
        mla = nullptr;
    }
}

bool PulseAudioSource::init()
{
    shutdown = false;
    nullDataSize = int(double(timerDelta)/1000) * 2 * int(sampleSpec.rate) * int(sampleSpec.channels);
    qDebug() << "null data size:" << nullDataSize;

    ml = pa_mainloop_new();
    mla = pa_mainloop_get_api(ml);

    ctx = pa_context_new(mla, Jupii::APP_NAME);
    if (!ctx) {
        qWarning() << "New pulse-audio context failed";
    } else {
        pa_context_set_state_callback(ctx, stateCallback, nullptr);
        pa_context_set_subscribe_callback(ctx, subscriptionCallback, nullptr);

        if (pa_context_connect(ctx, nullptr, PA_CONTEXT_NOFLAGS, nullptr) < 0) {
            qWarning() << "Cannot connect pulse-audio context:"
                       << pa_strerror(pa_context_errno(ctx));
        } else {
            return true;
        }

        pa_context_unref(ctx);
        ctx = nullptr;
    }

    pa_mainloop_free(ml);
    ml = nullptr;
    mla = nullptr;

    qWarning() << "Pulse-audio init error";
    return false;
}

void PulseAudioSource::doPulseIteration()
{
    //qDebug() << "doPulseIteration";
    auto worker = ContentServerWorker::instance();
    if (worker->screenCaptureItems.isEmpty() &&
            worker->audioCaptureItems.isEmpty()) {
        qDebug() << "No clients for audio capture connected, "
                    "so ending audio capturing";
        stop();
    }

    int ret = 0;
    while (pa_mainloop_iterate(PulseAudioSource::ml, 0, &ret) > 0)
        continue;
    //if (ret == 0) {
    if (ret == 0 && !shutdown) {
        if (!stream) {
            // sending null data (silence) to all connected devices
            // because there is no valid source of audio (no valid sink input)
            //qDebug() << "sending null data:" << nullDataSize;
            worker->dispatchPulseData(nullptr, nullDataSize);
        }
        iterationTimer.start();
    } else {
        qDebug() << "Pulse-audio loop quit";
        deinit();
    }
}

void PulseAudioSource::stop()
{
    shutdown = true;

    if (inited()) {
        qDebug() << "Requesting to stop pulse-audio";
        stopRecordStream();
        //mla->quit(mla, 10); // 10 = stop request
    } else {
        qWarning() << "Cannot stop because pulse-audio is not inited";
    }
}

bool PulseAudioSource::start()
{
    if (!inited()) {
        if (!init()) {
            qWarning() << "Pluse-audio loop cannot be inited";
            return false;
        }
    }

    shutdown = false;

    //discoverStream();

    if (!iterationTimer.isActive()) {
        iterationTimer.start();
    } else {
        qDebug() << "Pluse-audio loop already started";
    }

    return true;
}

