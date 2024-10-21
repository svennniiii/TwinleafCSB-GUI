import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11

Item {
    id: root
    property var driver
    property bool isConnected: driver.isConnected
    property string currentDevice: driver.currentDevice

    onCurrentDeviceChanged: {
        comboBox.model = driver.getDeviceList()

        if (currentDevice === ""){
            return
        } else if (comboBox.indexOfValue(currentDevice) === -1){
            comboBox.model = [currentDevice]
            comboBox.currentIndex = 0
            return
        } else {
            comboBox.currentIndex = comboBox.indexOfValue(currentDevice)
            return
        }
    }

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.rightMargin: 5
        anchors.leftMargin: 5
        anchors.topMargin: 5

        Label {
            text: qsTr("Port")
            font.bold: true
            font.pointSize: 12
        }

        RowLayout {
            id: rowLayout
            Layout.fillWidth: true

            function updateDevices(){
                let currentDevice = comboBox.currentText
                comboBox.model = root.driver.getDeviceList()
                comboBox.currentIndex = comboBox.indexOfValue(currentDevice)
            }

            ComboBox {
                id: comboBox
                Layout.fillWidth: true
                model: root.driver.getDeviceList()
                enabled: !root.isConnected

                Connections{
                    target: root.driver
                    function devicesChanged() {rowLayout.updateDevices()}
                }
            }

            Button {
                id: button
                //width: 40
                text: qsTr("⟳")
                Layout.maximumWidth: 40
                font.bold: true
                font.pointSize: 16
                onClicked: rowLayout.updateDevices()
                enabled: !root.isConnected
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.fillHeight: false
            Layout.fillWidth: true

            Label{
                id: label
                text: root.isConnected ? "Connected." : "Not connected."
                Layout.fillWidth: true
                font.pointSize: 10
            }


            Button {
                text: qsTr("Reconnect")
                Layout.fillWidth: false
                font.pointSize: 10
                onClicked: {
                    root.driver.disconnectDevice()
                    root.driver.connectDevice(comboBox.currentText)
                }
            }

            Button {
                text: qsTr("Disconnect")
                Layout.fillWidth: false
                onClicked: root.driver.disconnectDevice()
                font.pointSize: 10
            }
        }

        RowLayout {
            Layout.fillWidth: true

            TextField {
                id: textField
                //width: 80
                //height: 20
                font.pixelSize: 12
                Layout.fillWidth: true
                selectByMouse: true
            }

            Button {
                text: qsTr("Send")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: root.driver.sendCommand(textField.text)
                font.pointSize: 10
            }
        }
        ScrollView{
            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true

            onContentHeightChanged: contentItem.contentY = contentHeight - height

            TextEdit {
                id: textEdit
                font.pixelSize: 12
                textFormat: Text.RichText
                text: ""

                property color responseColor: "RoyalBlue"
                property color commandColor: "DarkGreen"

                onTextChanged: {
                    if (lineCount > 100){
                        text = text.split(/\r\n|\r|\n/).slice(-100).reduce(function(a,b){ return a + b}, "")
                    }
                }

                Connections{
                    target: root.driver
                    function onLastResponseChanged(){
                        let message = Qt.formatTime(new Date(), "hh:mm:ss") + " "
                        message += `<font color=${textEdit.responseColor}>${root.driver.lastResponse}</font>`
                        textEdit.text += message
                    }

                    function onLastCommandChanged(){
                        let message =  Qt.formatTime(new Date(), "hh:mm:ss") + " "
                        message += `<font color=${textEdit.commandColor}>${root.driver.lastCommand}</font>`
                        textEdit.text += message
                    }
                }
            }
        }
    }
}
/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:0.9;height:480;width:640}
}
##^##*/
