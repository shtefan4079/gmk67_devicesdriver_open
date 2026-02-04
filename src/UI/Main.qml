import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material
import gmk67_ui.backend 1.0

ApplicationWindow {
    id: window
    width: 1280
    height: 800
    visible: true
    title: "GMK67 Ultimate Control"
    
    Material.theme: Material.Dark
    Material.accent: Material.DeepPurple
    Material.primary: Material.Grey

    ProtocolController {
        id: backend
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TabBar {
            id: mainTabBar
            Layout.fillWidth: true
            
            TabButton {
                text: "LIGHTING"
                width: implicitWidth + 50
                font.bold: true
            }
            TabButton {
                text: "KEY MAPPING"
                width: implicitWidth + 50
                font.bold: true
            }
        }

        StackLayout {
            currentIndex: mainTabBar.currentIndex
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            LightingView {
                backend: backend
            }
            
            KeymapView {
                backend: backend
            }
        }
        
        Rectangle {
            Layout.fillWidth: true
            height: 30
            color: "#1a1a1a"
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                
                Label {
                    text: backend.isConnected ? "🟢 DEVICE CONNECTED" : "🔴 DEVICE DISCONNECTED"
                    color: backend.isConnected ? "#00ff00" : "#ff0000"
                    font.bold: true
                }
                
                Item { Layout.fillWidth: true }
                
                Button {
                    text: "⟳ REFRESH"
                    flat: true
                    height: 25
                    font.pixelSize: 10
                    onClicked: backend.connectToDevice()
                }
            }
        }
    }
}
