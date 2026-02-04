import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import "Layout.js" as KeyboardData

Page {
    id: lightingPage
    property var backend
    
    background: Rectangle { color: "#121212" }

    property var selectedKeyIds: []
    property color activeColor: "#ff0000"
    property var savedColors: ({})

    ColorDialog {
        id: colorPicker
        onAccepted: activeColor = selectedColor
    }

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
                text: "Settings" 
                font.pixelSize: 24 
                font.bold: true
                color: "white"
            }
            
            Rectangle { width: parent.width; height: 1; color: "#333" }

            Label { text: "Effect:"; color: "#888" }
            ComboBox {
                width: parent.width
                model: ["Static", "Breathing", "Spectrum", "Raindrops", "Ripple", "Wave", "Starry Night", "Rolling"]
                id: effectSelector
            }

            Label { text: "Color:"; color: "#888"; topPadding: 10 }
            Rectangle {
                width: parent.width
                height: 50
                color: activeColor
                radius: 8
                border.color: "white"
                
                MouseArea {
                    anchors.fill: parent
                    onClicked: colorPicker.open()
                }
            }

            Item { height: 20; width: 1 }

            Button {
                text: "Select All"
                width: parent.width
                onClicked: {
                    var arr = []
                    if(KeyboardData.keys) for(var i=0; i<KeyboardData.keys.length; i++) arr.push(KeyboardData.keys[i].id)
                    selectedKeyIds = arr
                }
            }
            Button {
                text: "Reset Selection"
                width: parent.width
                onClicked: selectedKeyIds = []
            }
            
            Item { height: 20; width: 1 }

            Button {
                text: "APPLY COLOR"
                width: parent.width
                height: 60
                font.bold: true
                onClicked: {
                    for(var i=0; i<selectedKeyIds.length; i++) {
                         savedColors[selectedKeyIds[i].toString()] = activeColor.toString()
                    }
                    
                    if(backend) backend.applyLighting(effectSelector.currentIndex + 1, savedColors)
                    
                    savedColors = savedColors 
                }
            }
        }
    }

    Rectangle {
        id: visualizer
        anchors.left: sidebar.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.margins: 10
        
        color: "#080808"
        radius: 16
        border.color: "#333"
        clip: true

        Text {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 20
            text: "Area: " + Math.round(visualizer.width) + "x" + Math.round(visualizer.height) + " Scale: " + zoomSlider.value.toFixed(2)
            color: "yellow"
            font.pixelSize: 14
            z: 10
        }

        Rectangle {
            id: bottomPanel
            height: 60
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            color: "#111"
            radius: 16
            
            Row {
                anchors.centerIn: parent
                spacing: 20
                Label { text: "ZOOM:"; color: "white"; anchors.verticalCenter: parent.verticalCenter }
                Slider {
                    id: zoomSlider
                    width: 300
                    from: 0.3
                    to: 2.0
                    value: 0.9
                    anchors.verticalCenter: parent.verticalCenter
                }
                Button {
                    text: "Reset Zoom"
                    onClicked: zoomSlider.value = 0.9
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        Item {
            id: contentArea
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: bottomPanel.top 
            clip: true

            Item {
                id: kbd
                width: 17 * 60
                height: 6 * 60
                
                anchors.centerIn: parent
                scale: zoomSlider.value

                Repeater {
                    model: KeyboardData.keys
                    delegate: Rectangle {
                        property var kData: modelData
                        property bool isSelected: selectedKeyIds.indexOf(kData.id) !== -1
                        property color keyColor: isSelected ? activeColor : (savedColors[kData.id.toString()] || "#333")

                        x: kData.x * 60
                        y: kData.y * 60
                        width: kData.w * 60 - 6
                        height: 60 - 6
                        radius: 6
                        
                        color: keyColor
                        border.color: isSelected ? "white" : "#555"
                        border.width: isSelected ? 2 : 1

                        Text {
                            anchors.centerIn: parent
                            text: kData.label
                            color: "white"
                            font.family: "Roboto"
                            font.pixelSize: 14 / zoomSlider.value
                            font.bold: true
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: parent.border.color = "#aaa"
                            onExited:  parent.border.color = isSelected ? "white" : "#555"
                            onClicked: {
                                var idx = selectedKeyIds.indexOf(kData.id)
                                if (idx === -1) {
                                    var n = selectedKeyIds.slice(); n.push(kData.id); selectedKeyIds = n;
                                } else {
                                    var n = selectedKeyIds.slice(); n.splice(idx, 1); selectedKeyIds = n;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
