import QtQuick 2.12
import QtLocation 5.12

Rectangle {
    id: mapView
    property alias mapCenter : map.center

    Plugin {
        id: mapPlugin
        name: "mapboxgl"
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapPlugin
        activeMapType: supportedMapTypes[6]
        zoomLevel: 2.5
    }
}
