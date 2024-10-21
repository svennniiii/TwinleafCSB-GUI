import QtQuick 6.8
import QtQuick.Window 6.8
import QtQuick.Controls 6.8

Item {
    id: window
    implicitWidth: swipeView.implicitWidth
    implicitHeight: swipeView.implicitHeight + tabBar.implicitHeight
    property var driver

    SwipeView {
        id: swipeView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: tabBar.top
        currentIndex: tabBar.currentIndex

        implicitHeight: settings.implicitHeight + output.implicitHeight
        implicitWidth: output.implicitWidth

        Settings {
            id: settings
            driver: window.driver
        }

        Output {
            id: output
            driver: window.driver
        }
    }

    TabBar {
        id: tabBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 0
        anchors.leftMargin: 0
        currentIndex: swipeView.currentIndex

        TabButton {
            text: qsTr("Settings")
            font.pointSize: 10
        }

        TabButton {
            text: qsTr("Output")
            font.pointSize: 10
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

