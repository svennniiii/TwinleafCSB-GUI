import QtQuick 2.4
import QtQuick.Window 2.12
import TwinleafCSB 1.0

Window {    
    id: window
    visible: true
    flags: {
          Qt.Window
        | Qt.CustomizeWindowHint
        | Qt.WindowSystemMenuHint
        | Qt.WindowMinimizeButtonHint
        | Qt.WindowCloseButtonHint
        | Qt.WindowTitleHint
    }

    Component.onCompleted: {
        window.width = mainForm.implicitWidth
        window.height =  mainForm.implicitHeight

        window.maximumHeight = height
        window.minimumHeight = height

        window.maximumWidth = width
        window.minimumWidth = width

        twinleafCSB.loadSettings()
    }

    MainForm{
        id: mainForm
        driver: twinleafCSB
    }

    TwinleafCSB{
        id: twinleafCSB
    }
}
