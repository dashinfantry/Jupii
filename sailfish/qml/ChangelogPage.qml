/* Copyright (C) 2017 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: root

    allowedOrientations: Orientation.All

    SilicaFlickable {
        id: flick
        anchors.fill: parent
        contentHeight: content.height

        Column {
            id: content

            width: root.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: qsTr("Changelog")
            }

            SectionHeader {
                text: qsTr("Version %1").arg("2.4.0 (beta)")
            }

            LogItem {
                title: "Screen capture"
                description: "Screen data is sent as a video stream to remote DLNA device. " +
                             "Be aware that due to buffering there is a delay " +
                             "between phone and the video displayed on the remote screen. " +
                             "Screen capturing requires Jupii to run as a privileged app, " +
                             "therefore this feature will be enabled only in OpenRepos package. " +
                             "The implementation is inspired and partially based on " +
                             "<a href=\"https://github.com/mer-qa/lipstick2vnc/\">lipstick2vnc</a> project."
            }

            LogItem {
                title: "Microphone and Audio capture improvements"
                description: "Audio capturing and Microphone features have been significantly improved. " +
                             "Audio streaming has lower delay and the overall performance, " +
                             "including impact on battery, is much better. " +
                             "Moreover, microphone data is now always encoded to MP3 which provides " +
                             "better compatibility with various DLNA devices."
            }

            LogItem {
                title: "Improved search on Album, Recordings and gPodder pages"
                description: "Searching takes into account many different fields like: " +
                             "album title, artist name, station name, podcast and episode title. " +
                             "There is also an option to sort search results."
            }

            SectionHeader {
                text: qsTr("Version %1").arg("2.3.1 (beta)")
            }

            LogItem {
                title: "Landscape mode"
                description: "Landscape orientation is now supported."
            }

            LogItem {
                title: "Translations update"
                description: "Following translations have been updated: " +
                             "Slovenian, Chinese, Russian, Polish and Swedish. " +
                             "Many thanks to all contributors."
            }

            SectionHeader {
                text: qsTr("Version %1").arg("2.3.0 (beta)")
            }

            LogItem {
                title: "Stream recorder"
                description: "Enables recording of tracks from SHOUTcast stream. " +
                             "When stream provides information " +
                             "about the title of the currently played track, " +
                             "you can save this track to a file. " +
                             "Stream recording feature can be turn on in Experiments " +
                             "section in the settings."
            }

            LogItem {
                title: "Better volume up/down hardware keys support"
                description: "You can change the volume level of a remote DLNA " +
                             "device using phone hardware " +
                             "keys without changing phone volume level. " +
                             "Support for hardware keys is enabled by default but it can be " +
                             "disabled in the settings"
            }

            LogItem {
                title: "Initial Yamaha XC API support - Toggle power option"
                description: "When remote device supports Yamaha XC API, " +
                             "option to toggle power is visible in the context menu."
            }

            LogItem {
                title: "DBus API improvements"
                description: "App is bring to foreground when addPathOnceAndPlay " +
                             "or addUrlOnceAndPlay is invoked."
            }

            LogItem {
                title: "SomaFM streaming improvements"
                description: "Streaming of SomaFM has been improved to be more " +
                             "compatible with various DLNA implementations. " +
                             "Especially it applies to Samsung TV devices. "
            }

            SectionHeader {
                text: qsTr("Version %1").arg("2.2.2")
            }

            LogItem {
                title: "Audio capture of any application"
                description: "This option enables capturing audio output of any application. " +
                             "It provides similar functionality to pulseaudio-dlna server. It means that " +
                             "Jupii can stream certain application's audio playback to a DLNA device. " +
                             "For instance, you can capture web browser audio playback to listen YouTube on a remote speaker. " +
                             "You can enable audio capturing by adding \"Audio capture\" from \"Add item\" menu. " +
                             "By default captured audio stream will be encoded to MP3 format. " +
                             "Encoding adds extra delay comparing to uncommpressed stream but it is " +
                             "much more efficient for overall performance. " +
                             "A stream format/quality can be changed in the settings (Experiments section). "
            }

            LogItem {
                title: "Option to update SomaFM channel list"
                description: "To download the latest list of SomaFM channels " +
                             "choose \"Refresh channel list\" from pull-down menu. " +
                             "If you enjoy SomaFM radio, please consider making a " +
                             "<a href=\"http://somafm.com/support/\">donation</a>."
            }

            LogItem {
                title: "Initial support for HLS URLs"
                description: "Very basic support for HTTP Live Streaming has been added. " +
                             "When Jupii detects that URL points to HLS playlist it will " +
                             "just pass it to a DLNA device without streaming relay. " +
                             "Your DLNA device has to support HLS to make it work."
            }

            LogItem {
                title: "Streaming improvements"
                description: "Streaming of local files has been improved to be more compatible with various DLNA implementations. " +
                             "Especially it applies to Samsung TV devices. "
            }

            LogItem {
                title: "Better support for playlist formats"
                description: "Playlists with relative URLs are now accepted."
            }

            LogItem {
                title: "DBus API extension"
                description: "Addional methods to DBus API have been added."
            }

            LogItem {
                title: "Slovenian and Chinese translations"
                description: "Many thanks to Rui Kon and Boštjan Štrumbelj for " +
                             "providing Chinese and Slovenian translations."
            }

            SectionHeader {
                text: qsTr("Version %1").arg("2.0.0")
            }

            LogItem {
                title: "Support for Internet streaming content"
                description: "In addition to local files, URL content " +
                             "(e.g. internet radio streams, remote media files) can be added to the playlist queue. " +
                             "An URL should point to direct stream or to a playlist file. " +
                             "Only HTTP URLs are supported right now."
            }

            LogItem {
                title: "SomaFM channels"
                description: "As a playlist item, you can add SomaFM radio channel. " +
                             "SomaFM is an independent Internet-only streaming service, " +
                             "supported entirely with donations from listeners. " +
                             "If you enjoy SomaFM radio, please consider making a " +
                             "<a href=\"http://somafm.com/support/\">donation</a>."
            }

            LogItem {
                title: "Icecast directory"
                description: "Icecast directory is a listing of broadcast streams. " +
                             "Jupii has built-in browser for searching and adding " +
                             "Icecast stations to its playlist queue."
            }

            LogItem {
                title: "Podcasts browser for gPodder"
                description: "A convenient built-in browser that allows you to add episodes " +
                             "previously downloaded with gPodder podcast player. " +
                             "Option is visible only if gPodder app is installed."
            }

            LogItem {
                title: "Microphone"
                description: "Use microphone as a source for audio stream that is played on your DLNA device. " +
                             "Add Item list contains additional item - Microphone."
            }

            LogItem {
                title: "Playlist queue polish"
                description: "Playlist queue has been polished. When track provides meta data, " +
                             "playlist item contains title, author and album art image."
            }

            LogItem {
                title: "Improved support for playlist files"
                description: "Interface for browsing and adding tracks from playlist files has been improved. " +
                             "Now it is possible to search and select individual tracks. " +
                             "Additionally, the file picker supports three major playlist formats (M3U, XSPF and PLS)."
            }

            LogItem {
                title: "SHOUTcast meta data support"
                description: "Many internet radio services use SHOUTcast streaming. " +
                             "Jupii is able to display stream title retrieved from meta data. " +
                             "It also removes in-stream meta data if DLNA device doesn't support SHOUTcast protocol."
            }

            LogItem {
                title: "Improved cover design"
                description: "Cover page has a new background image."
            }

            LogItem {
                title: "General performance and stability improvements"
                description: "Overall stability and performance have been improved."
            }

            SectionHeader {
                text: qsTr("Version %1").arg("1.0.0")
            }

            LogItem {
                title: "Spanish Swedish & German translations"
                description: "Many thanks to Carlos Gonzalez for " +
                             "providing Spanish translation."
            }

            SectionHeader {
                text: qsTr("Version %1").arg("0.9.6")
            }

            LogItem {
                title: "Swedish & German translations"
                description: "Many thanks to Åke Engelbrektson and drosjesjaafoer for " +
                             "providing Swedish and German translations."
            }

            LogItem {
                title: "Improved stability and bug fixes"
                description: "Resilience to crash failures and other errors " +
                             "has been significantly improved."
            }

            SectionHeader {
                text: qsTr("Version %1").arg("0.9.5")
            }

            LogItem {
                title: "Better support for various UPnP devices"
                description: "Support for diffrent UPnP devices has been improved."
            }

            LogItem {
                title: "Russian, Dutch & Polish translations"
                description: "Many thanks to Вячеслав Диконов and d9h20f for " +
                             "providing Russian and Dutch translations."
            }

            SectionHeader {
                text: qsTr("Version %1").arg("0.9.4")
            }

            LogItem {
                title: "Play audio stream extracted from video files"
                description: "Some DLNA devices support only audio content. " +
                             "In order to play video files on such devices, " +
                             "Jupii can extract an audio stream from a video file."
            }

            LogItem {
                title: "Music picker sorted by artists"
                description: "The music tracks can be browsed by artist name."
            }

            LogItem {
                title: "Playlist file picker"
                description: "The music tracks from a playlist file can be " +
                             "added to the current playlist."
            }

            LogItem {
                title: "Save current playlist to a file"
                description: "Current playlist can be saved as a playlist file."
            }

            LogItem {
                title: "Add device manually (experimental)"
                description: "If Jupii fails to discover a device " +
                             "(e.g. because it is in a different LAN), you can " +
                             "add it manually with IP address. Make sure " +
                             "that device is not behind a NAT or a firewall."
            }

            SectionHeader {
                text: qsTr("Version %1").arg("0.9.3")
            }

            LogItem {
                title: "Music album picker"
                description: "The music tracks can be browsed by album name. " +
                             "The whole album or individual tracks can be " +
                             "added to the current playlist."
            }

            SectionHeader {
                text: qsTr("Version %1").arg("0.9.2")
            }

            LogItem {
                title: "Repeat play mode"
                description: "Items on the playlist can be playbacked in " +
                             "Normal, Repeat-All or Repeat-One mode."
            }

            LogItem {
                title: "Multi-item pickers"
                description: "Music, Video, Image or File pickers support " +
                             "selection of multiple items."
            }

            LogItem {
                title: "Start with last playlist"
                description: "When Jupii connects to a device, the last saved " +
                             "playlist will be automatically loaded. " +
                             "If you don't like this feature it can be disabled " +
                             "in the settings."
            }

            LogItem {
                title: "Volume control with hardware keys"
                description: "Change volume level using phone hardware volume keys. " +
                             "The volume level of the media device will be " +
                             "set to be the same as the volume level of the ringing alert " +
                             "on the phone. Option can be disabled in the settings."
            }

            LogItem {
                title: "D-Bus API"
                description: "Jupii exposes simple D-Bus service. It can be use " +
                             "to make integration with other Sailfish OS applications. " +
                             "The example 'proof of concept' intergation with gPodder " +
                             "is available to download from Jupii GitHub page."
            }

            LogItem {
                title: "Improvements of the player UI"
                description: "Player bottom panel has more compact look. " +
                             "If needed, it can be also expanded to the full size."
            }

            SectionHeader {
                text: qsTr("Version %1").arg("0.9.1")
            }

            LogItem {
                title: "Initial release"
                description: "This is initial public release.";
            }

            Spacer {}
        }
    }

    VerticalScrollDecorator {
        flickable: flick
    }

    focus: true
    Keys.onVolumeUpPressed: {
        rc.volUpPressed()
    }
    Keys.onVolumeDownPressed: {
        rc.volDownPressed()
    }
}
