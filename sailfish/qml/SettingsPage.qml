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
        contentHeight: column.height

        Column {
            id: column

            width: root.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: qsTr("Settings")
            }

            TextSwitch {
                automaticCheck: false
                checked: settings.rememberPlaylist
                text: qsTr("Start with last playlist")
                description: qsTr("When Jupii starts, the last " +
                                  "playlist will be automatically loaded.")
                onClicked: {
                    settings.rememberPlaylist = !settings.rememberPlaylist
                }
            }

            Slider {
                width: parent.width
                minimumValue: 1
                maximumValue: 60
                stepSize: 1
                handleVisible: true
                value: settings.forwardTime
                valueText: value + " s"
                label: qsTr("Forward/backward time-step interval")

                onValueChanged: {
                    settings.forwardTime = value
                }
            }

            TextSwitch {
                automaticCheck: false
                checked: settings.useHWVolume
                text: qsTr("Volume control with hardware keys")
                onClicked: {
                    settings.useHWVolume = !settings.useHWVolume
                }
            }

            Slider {
                visible: settings.useHWVolume
                width: parent.width
                minimumValue: 1
                maximumValue: 10
                stepSize: 1
                handleVisible: true
                value: settings.volStep
                valueText: value
                label: qsTr("Volume level step")

                onValueChanged: {
                    settings.volStep = value
                }
            }

            SectionHeader {
                text: qsTr("Experiments")
            }

            TextSwitch {
                automaticCheck: false
                checked: settings.imageSupported
                text: qsTr("Image content")
                description: qsTr("Playing images on UPnP devices doesn't work well right now. " +
                                  "There are few minor issues that have not been resolved yet. " +
                                  "This option forces %1 to play images despite the fact " +
                                  "it could cause some issues.").arg(APP_NAME)
                onClicked: {
                    settings.imageSupported = !settings.imageSupported
                }
            }

            TextSwitch {
                automaticCheck: false
                checked: settings.screenAudio
                text: qsTr("Screen capture with audio")
                onClicked: {
                    settings.screenAudio = !settings.screenAudio
                }
            }

            /*ComboBox {
                label: qsTr("Screen capture framerate")
                currentIndex: {
                    if (settings.screenFramerate < 15) {
                        return 0;
                    } else if (settings.screenFramerate < 30) {
                        return 1;
                    } else {
                        return 2;
                    }
                }
                menu: ContextMenu {
                    MenuItem { text: qsTr("5 fps (default)") }
                    MenuItem { text: qsTr("15 fps") }
                    MenuItem { text: qsTr("30 fps") }
                }

                onCurrentIndexChanged: {
                    switch (currentIndex) {
                    case 0:
                        settings.screenFramerate = 5; break;
                    case 1:
                        settings.screenFramerate = 15; break;
                    case 2:
                        settings.screenFramerate = 30; break;
                    default:
                        settings.screenFramerate = 5;
                    }
                }
            }*/

            /*ComboBox {
                // modes:
                // 0 - MP3 16-bit 44100 Hz stereo 128 kbps (default)
                // 1 - MPEG2-TS MP3 44100Hz 128 kbps
                label: qsTr("Audio capture format")
                currentIndex: settings.audioCaptureMode
                menu: ContextMenu {
                    MenuItem { text: qsTr("MP3 44100Hz 128 kbps (default)") }
                    MenuItem { text: qsTr("MPEG2-TS MP3 44100Hz 128 kbps") }
                }

                onCurrentIndexChanged: {
                    settings.audioCaptureMode = currentIndex
                }
            }*/

            TextSwitch {
                automaticCheck: false
                checked: settings.rec
                text: qsTr("Stream recorder")
                description: qsTr("Enables recording of tracks from SHOUTcast stream. " +
                                  "When stream provides information " +
                                  "about the title of the currently played track, " +
                                  "you can save this track to a file. To enable " +
                                  "recording use \"Record\" button located next to " +
                                  "\"Forward\" button on the bottom bar. " +
                                  "This button is visible only when recording is possible. " +
                                  "When the \"Record\" button is activated before " +
                                  "the end of the track, the whole recording " +
                                  "(from the begining to the end of the track) " +
                                  "will be saved as a file. " +
                                  "Currently AAC streams cannot be recorded.")
                onClicked: {
                    settings.rec = !settings.rec
                }
            }

            ListItem {
                visible: settings.rec
                contentHeight: visible ? recflow.height + 2*Theme.paddingLarge : 0

                Flow {
                    id: recflow
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: parent.left; right: parent.right
                        leftMargin: Theme.paddingLarge
                        rightMargin: Theme.paddingLarge
                    }
                    spacing: Theme.paddingMedium

                    Label {
                        text: qsTr("Directory for recordings")
                    }

                    Label {
                        color: Theme.highlightColor
                        text: utils.dirNameFromPath(settings.recDir)
                    }
                }

                onClicked: openMenu();

                menu: ContextMenu {
                    MenuItem {
                        text: qsTr("Change")
                        onClicked: {
                            var obj = pageStack.push(Qt.resolvedUrl("DirPage.qml"));
                            obj.accepted.connect(function() {
                                settings.recDir = obj.selectedPath
                            })
                        }
                    }
                    MenuItem {
                        text: qsTr("Set default")
                        onClicked: {
                            settings.recDir = ""
                        }
                    }
                }
            }

            TextSwitch {
                automaticCheck: false
                checked: settings.showAllDevices
                text: qsTr("All devices visible")
                description: qsTr("%1 supports only Media Renderer devices. With this option enabled, " +
                                  "all UPnP devices will be shown, including unsupported devices like " +
                                  "home routers or Media Servers. For unsupported devices %1 is able " +
                                  "to show only basic description information. " +
                                  "This option could be useful for auditing UPnP devices " +
                                  "in your local network.").arg(APP_NAME)
                onClicked: {
                    settings.showAllDevices = !settings.showAllDevices
                }
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
