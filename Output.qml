pragma ComponentBehavior: Bound

import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQml.Models 2.1

GridLayout {
    id: root
    rowSpacing: 10
    rows: 3
    columns: 1

    property var driver

    ListModel {
        id: channelsDescription
        ListElement {label: "X channel"; channelName: "x"}
        ListElement {label: "Y channel"; channelName: "y"}
        ListElement {label: "Z channel"; channelName: "z"}
    }

    Repeater {
        id: channels
        model: channelsDescription

        property var driver: root.driver

        delegate: Item{
            id: delegate

            implicitWidth:  channel.implicitWidth
            implicitHeight: channel.implicitHeight

            required property string label
            required property string channelName

            property var channelHandler: root.driver.getChannel(channelName)

            Channel {
                id: channel
                label: delegate.label

                onOffsetChanged:     delegate.channelHandler.offset    = offset
                onAmplitudeChanged:  delegate.channelHandler.amplitude = amplitude
                onFrequencyChanged:  delegate.channelHandler.frequency = frequency
            }

            Connections {
                id: connection
                target: delegate.channelHandler
                function onOffsetChanged()    {channel.offset    = delegate.channelHandler.offset}
                function onAmplitudeChanged() {channel.amplitude = delegate.channelHandler.amplitude}
                function onFrequencyChanged() {channel.frequency = delegate.channelHandler.frequency}
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.75}
}
##^##*/
