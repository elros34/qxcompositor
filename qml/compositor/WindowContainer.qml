/****************************************************************************
** Copyright (C) 2019 elros34
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.6
import Sailfish.Silica 1.0
import QtQuick.Window 2.0
import QtCompositor 1.0
import QXCompositor 1.0

Item {
    id: container
    width: parent.width
    height: parent.height

    property Item surfaceItem: null // qwaylandsurfaceitem
    property bool isXwaylandWindow: false

    Connections {
        target: root
        onIsPortraitChanged: fillScreen()
    }

    function fillScreen() {
        if (isXwaylandWindow)
            return
        surfaceItem.width = isPortrait ? Screen.width : Screen.height
        surfaceItem.height = isPortrait ? Screen.height : Screen.width
    }

    Component.onCompleted: {
        compositor.fullscreenSurface = surfaceItem.surface

        if (compositor.sshUserOption && compositor.sshPortOption) {
            console.log("create xclipboard")
            xclipboard.createObject(container)
        }
    }

    visible: true
    onVisibleChanged: {
        if (surfaceItem)
            surfaceItem.surface.clientRenderingEnabled = visible
    }

    onSurfaceItemChanged: {
        if (surfaceItem) {
            surfaceItem.parent = container
            delayTimer.start()
        }
    }

    Connections {
        target: container.surfaceItem ? container.surfaceItem.surface : null

        onUnmapped: {
            root.removeWindow(container)
        }
    }
    Connections {
        target: container.surfaceItem ? container.surfaceItem : null
        onSurfaceDestroyed: {
            root.removeWindow(container)
        }
    }

    property bool delayed: false
    Timer { // FIXME
        id: delayTimer
        interval: 2000
        onTriggered: delayed = true
    }

    Component {
        id: xclipboard
        XClipboard {
            xwaylandWindowReady: delayed && container.surfaceItem !== null
            compositorWindowActive: appWindow.applicationActive
            sshUser: compositor.sshUserOption
            sshPort: compositor.sshPortOption
        }
    }


    property Item subSurfaceItem1: null
    property var subSurfaceItems: []

    function addSubSurfaceItem(subSurfaceItem) {
        subSurfaceItems.push(subSurfaceItem)
        container.subSurfaceItem1 = subSurfaceItem
        var subContainer = subContainerComponent.createObject(container,
                                                              {
                                                                  "subSurfaceItem": subSurfaceItem
                                                              })
    }

    function removeSubSurfaceItem(subSurfaceItem) {
        var i = subSurfaceItems.indexOf(subSurfaceItem)
        if (i >= 0) {
            console.log("removeSubSurfaceItem " + i + ": " + subSurfaceItems)
            subSurfaceItems.splice(i, 1)
        }
    }


    Component {
        id: subContainerComponent
        MouseArea {
            id: subContainer
            width: subSurfaceItem ? (Math.max(30, subSurfaceItem.width)) + 2 : 0
            height: subSurfaceItem ? (Math.max(10, subSurfaceItem.height + closeButton.height)) + 1 : 0
            //x: Math.max(0, (container.width - width)/2)
            //y: Math.max(0, (container.height - height)/2)
            // changing position creates random mouse pos offsets and crashes..

            visible: subSurfaceItem
            enabled: visible

            drag {
                target: subContainer
                axis: Drag.XAndYAxis
            }

            property Item subSurfaceItem: null

            onSubSurfaceItemChanged: {
                if (subSurfaceItem) {
                    subSurfaceItem.parent = subContainer
                    // ignore x,y changes
                    subSurfaceItem.anchors.bottom = subContainer.bottom
                    subSurfaceItem.anchors.left = subContainer.left
                    subSurfaceItem.anchors.bottomMargin = 1
                    subSurfaceItem.anchors.leftMargin = 1
                    subSurfaceItem.takeFocus()
                }
            }

            Connections {
                target: subSurfaceItem
                onSurfaceDestroyed: close()
            }

            function close() {
                container.removeSubSurfaceItem(subSurfaceItem)
                subSurfaceItem.destroy()
                subSurfaceItem = null
                subContainer.destroy()
            }

            Rectangle {
                anchors.fill: parent
                color: "transparent"
                border.color: "black"
                border.width: 4
            }

            Rectangle {
                color: Theme.rgba(Theme.highlightDimmerColor, 0.7)
                width: parent.width
                height: Theme.itemSizeExtraSmall*2/3
            }

            IconButton {
                id: closeButton
                width: Theme.itemSizeSmall
                height: Theme.itemSizeExtraSmall*2/3
                icon.source: "image://theme/icon-m-cancel"
                anchors {
                    right: parent.right
                    top: parent.top
                }

                onClicked: subContainer.close()
            }
        }
    }
}
