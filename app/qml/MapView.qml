import QtQuick 2.0
import QtQuick.Window 2.14
import QtLocation 5.6
import QtPositioning 5.6

Rectangle {
    Plugin {
        id: mapPlugin
        name: "mapboxgl" // "mapboxgl", "esri", ...
        // specify plugin parameters if necessary
        // PluginParameter {
        //     name:
        //     value:
        // }
    }

    Map {
        anchors.fill: parent
        plugin: mapPlugin
        zoomLevel: 2.5

        onZoomLevelChanged: console.log(zoomLevel)
    }

}
