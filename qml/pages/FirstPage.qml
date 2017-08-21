import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: root
    objectName: "firstPage"

    Label {
        id: hintLabel
        anchors.centerIn: parent
        text: "No Xwayland window"
        font.pixelSize: Theme.fontSizeLarge
    }

    property variant selectedWindow: null
    property bool hasFullscreenWindow: typeof compositor != "undefined" && compositor.fullscreenSurface !== null


    function windowAdded(window) {
        var windowContainerComponent = Qt.createComponent("../compositor/XWaylandContainer.qml");
        if (windowContainerComponent.status != Component.Ready) {
            console.warn("Error loading WindowContainer.qml: " +  windowContainerComponent.errorString());
            return;
        }

        var windowContainer = windowContainerComponent.createObject(root, {
                                                                    "child": compositor.item(window)
                                                                    });
        windowContainer.objectName = "windowContainer"
        windowContainer.child.resizeSurfaceToItem = true
        windowContainer.child.parent = windowContainer;
        windowContainer.child.touchEventsEnabled = true;

        hintLabel.visible = false

        root.selectedWindow = windowContainer.child
        windowContainer.child.takeFocus()
    }

    function windowResized(window) {
        window.width = window.surface.size.width;
        window.height = window.surface.size.height;
    }

    function removeWindow(window) {
        window.destroy();
        hintLabel.visible = true
    }
}

