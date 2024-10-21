import QtQuick 6.8
import QtQuick.Controls 6.8
import QtQuick.Layouts 6.8

ColumnLayout {
    id: root
    spacing: 10

    property string label: qsTr("Channel x")

    property double offset: 0.0
    property double offsetMin: -10.0
    property double offsetMax: 10.0

    property double amplitude: 0.0
    property double amplitudeMin: 0.0
    property double amplitudeMax: 10.0

    property double frequency: 0.0
    property double frequencyMin: 0.0
    property double frequencyMax: 1000.0

    function validateValue(value, min, max) {
        return Math.max(min, Math.min(max, value))
    }

    function updateValue(propertyName, newValue) {
        root[propertyName] = validateValue(newValue, root[propertyName + "Min"], root[propertyName + "Max"])
    }

    Label {
        text: root.label
        font.pointSize: 12
        font.bold: true
        Layout.fillWidth: true
        Layout.margins: 5
    }

    GridLayout {
        columns: 5
        columnSpacing: 5
        rowSpacing: 5
        Layout.fillWidth: true
        Layout.margins: 5

        // Offset
        Label { text: qsTr("Offset"); Layout.alignment: Qt.AlignRight }
        Slider {
            from: root.offsetMin
            to: root.offsetMax
            value: root.offset
            onMoved: root.updateValue("offset", value)
            Layout.fillWidth: true
        }
        Button {
            text: "↺"
            font.bold: true
            font.pointSize: 16
            implicitWidth: 40
            implicitHeight: 40
            onClicked: root.updateValue("offset", 0.0)
        }
        SpinBox {
            id: offsetSpinBox
            from: root.offsetMin * 1000000
            to: root.offsetMax * 1000000
            value: root.offset * 1000000
            stepSize: 10000
            editable: true
            onValueModified: root.updateValue("offset", value / 1000000)

            textFromValue: (value, locale) => (value / 1000000).toLocaleString(locale, 'f', 6)
            valueFromText: (text, locale) => Number.fromLocaleString(locale, text) * 1000000

            validator: DoubleValidator {
                bottom: offsetSpinBox.from / 1000000
                top: offsetSpinBox.to / 1000000
                decimals: 6
                notation: DoubleValidator.StandardNotation
            }
        }
        Label { text: qsTr("mA") }

        // Frequency
        Label { text: qsTr("Mod. Frequency"); Layout.alignment: Qt.AlignRight }
        Slider {
            from: root.frequencyMin
            to: root.frequencyMax
            value: root.frequency
            onMoved: root.updateValue("frequency", value)
            Layout.fillWidth: true
        }
        Button {
            text: "↺"
            font.bold: true
            font.pointSize: 16
            implicitWidth: 40
            implicitHeight: 40
            onClicked: root.updateValue("frequency", 0.0)
        }
        SpinBox {
            id: frequencySpinBox
            from: root.frequencyMin * 100
            to: root.frequencyMax * 100
            value: root.frequency * 100
            stepSize: 10
            editable: true
            onValueModified: root.updateValue("frequency", value / 100)

            textFromValue: (value, locale) => (value / 100).toLocaleString(locale, 'f', 2)
            valueFromText: (text, locale) => Number.fromLocaleString(locale, text) * 100

            validator: DoubleValidator {
                bottom: frequencySpinBox.from / 100
                top: frequencySpinBox.to / 100
                decimals: 2
                notation: DoubleValidator.StandardNotation
            }
        }
        Label { text: qsTr("Hz") }

        // Amplitude
        Label { text: qsTr("Mod. Amplitude"); Layout.alignment: Qt.AlignRight }
        Slider {
            from: root.amplitudeMin
            to: root.amplitudeMax
            value: root.amplitude
            onMoved: root.updateValue("amplitude", value)
            Layout.fillWidth: true
        }
        Button {
            text: "↺"
            font.bold: true
            font.pointSize: 16
            implicitWidth: 40
            implicitHeight: 40
            onClicked: root.updateValue("amplitude", 0.0)
        }
        SpinBox {
            id: amplitudeSpinBox
            from: root.amplitudeMin * 1000000
            to: root.amplitudeMax * 1000000
            value: root.amplitude * 1000000
            stepSize: 10000
            editable: true
            onValueModified: root.updateValue("amplitude", value / 1000000)

            textFromValue: (value, locale) => (value / 1000000).toLocaleString(locale, 'f', 6)
            valueFromText: (text, locale) => Number.fromLocaleString(locale, text) * 1000000

            validator: DoubleValidator {
                bottom: amplitudeSpinBox.from / 1000000
                top: amplitudeSpinBox.to / 1000000
                decimals: 6
                notation: DoubleValidator.StandardNotation
            }
        }
        Label { text: qsTr("mA") }
    }
}
