import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Layout.js" as KeyboardData
import "FullLayout.js" as SourceData

Page {
    id: keymapView
    property var backend
    
    background: Rectangle { color: "#121212" }

    property var selectedTargetKeyId: null
    property var selectedSourceKeyName: null
    property var keyMappings: ({})

    Rectangle {
        id: sidebar
        width: 320
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 10
        color: "transparent"

        Column {
            anchors.fill: parent
            spacing: 15

            Label { 
                text: "Key Mapping" 
                font.pixelSize: 24 
                font.bold: true
                color: "white"
            }
            Rectangle { width: parent.width; height: 1; color: "#333" }

            Label { 
                text: "1. Select a key on Top Keyboard\n2. Select a function (Bottom)\n3. Click 'ASSIGN'\n4. Click 'APPLY' to save"
                color: "#aaa"
                wrapMode: Text.WordWrap
                width: parent.width
            }

            Rectangle {
                width: parent.width
                height: 80
                color: "#222"
                radius: 8
                border.color: "#444"
                
                Column {
                    anchors.centerIn: parent
                    spacing: 5
                    Text { text: selectedTargetKeyId ? "Key ID: " + selectedTargetKeyId : "Select a Key"; color: "white"; font.bold: true; anchors.horizontalCenter: parent.horizontalCenter }
                    Text { text: "⬇️"; color: "#666"; anchors.horizontalCenter: parent.horizontalCenter }
                    Text { text: selectedSourceKeyName ? selectedSourceKeyName : "..."; color: "#4caf50"; font.bold: true; anchors.horizontalCenter: parent.horizontalCenter; font.pixelSize: 16 }
                }
            }

            Button {
                text: "ASSIGN"
                width: parent.width
                height: 50
                enabled: selectedTargetKeyId !== null && selectedSourceKeyName !== null
                onClicked: {
                    keyMappings[selectedTargetKeyId.toString()] = selectedSourceKeyName
                    keyMappings = keyMappings 
                    console.log("Assigned " + selectedSourceKeyName + " to " + selectedTargetKeyId)
                }
            }

            Button {
                text: "UNBIND KEY"
                width: parent.width
                height: 40
                enabled: selectedTargetKeyId !== null
                onClicked: {
                     var temp = keyMappings
                     delete temp[selectedTargetKeyId.toString()]
                     keyMappings = temp
                     keyMappings = keyMappings
                }
            }

            Item { height: 20; width: 1 }

            Button {
                text: "APPLY"
                width: parent.width
                height: 60
                font.bold: true
                onClicked: {
                    if(backend) backend.applyKeymap(keyMappings)
                }
            }
        }
    }

    Rectangle {
        id: mainArea
        anchors.left: sidebar.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 10
        color: "transparent"

        Rectangle {
            id: targetContainer
            height: parent.height * 0.45
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            color: "#080808"
            radius: 12
            border.color: "#333"
            clip: true

            Label { text: "TARGET KEYBOARD"; color: "#444"; font.bold: true; anchors.top: parent.top; anchors.left: parent.left; anchors.margins: 10 }
            
            Row {
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.margins: 10
                spacing: 10
                Label { text: "Zoom:"; color: "#666" }
                Slider { id: targetZoom; from: 0.5; to: 1.5; value: 0.8; width: 100 }
            }

            Item {
                width: 17 * 60
                height: 6 * 60
                scale: targetZoom.value
                anchors.centerIn: parent
                
                Repeater {
                    model: KeyboardData.keys
                    delegate: Rectangle {
                        property bool isSel: selectedTargetKeyId === modelData.id
                        property string mappedName: keyMappings[modelData.id.toString()] || ""
                        
                        x: modelData.x * 60; y: modelData.y * 60
                        width: modelData.w * 60 - 6; height: 60 - 6
                        radius: 6
                        color: isSel ? "#2196f3" : (mappedName !== "" ? "#3f51b5" : "#333")
                        border.color: isSel ? "white" : (mappedName !== "" ? "#88aaff" : "#555")
                        border.width: isSel ? 2 : 1
                        
                        Column {
                            anchors.centerIn: parent
                            Text { text: modelData.label; color: "white"; font.pixelSize: 12/targetZoom.value; anchors.horizontalCenter: parent.horizontalCenter }
                            Text { 
                                text: mappedName
                                color: "#ffecb3" 
                                visible: mappedName !== ""
                                font.pixelSize: 10/targetZoom.value 
                                font.bold: true
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }
                        
                        MouseArea { anchors.fill: parent; onClicked: selectedTargetKeyId = modelData.id }
                    }
                }
            }
        }

        Rectangle {
            id: sourceContainer
            anchors.top: targetContainer.bottom
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 10
            color: "#080808"
            radius: 12
            border.color: "#333"
            clip: true

            Label { text: "SOURCE FUNCTION"; color: "#444"; font.bold: true; anchors.top: parent.top; anchors.left: parent.left; anchors.margins: 10 }

            Row {
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.margins: 10
                spacing: 10
                Label { text: "Zoom:"; color: "#666" }
                Slider { id: sourceZoom; from: 0.3; to: 1.2; value: 0.6; width: 100 }
            }

            Item {
                width: 23 * 60 
                height: 7 * 60
                scale: sourceZoom.value
                anchors.centerIn: parent
                
                Repeater {
                    model: SourceData.keys
                    delegate: Rectangle {
                        property bool isSel: selectedSourceKeyName === modelData.code
                        x: modelData.x * 60; y: modelData.y * 60
                        width: modelData.w * 60 - 4; height: 60 - 4
                        radius: 4
                        color: isSel ? "#4caf50" : "#222"
                        border.color: isSel ? "white" : "#444"
                        
                        Text { anchors.centerIn: parent; text: modelData.label; color: "white"; font.pixelSize: 10/sourceZoom.value; font.bold: true }
                        MouseArea { anchors.fill: parent; onClicked: selectedSourceKeyName = modelData.code }
                    }
                }
            }
        }
    }
}
