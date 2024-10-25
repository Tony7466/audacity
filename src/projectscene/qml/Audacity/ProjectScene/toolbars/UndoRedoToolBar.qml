/*
* Audacity: A Digital Audio Editor
*/
import QtQuick

import Muse.UiComponents

import Audacity.ProjectScene

StyledToolBarView {
    navigationPanel.name: "UndoRedoToolBar"
    navigationPanel.accessible.name: qsTrc("projectscene", "Undo/redo toolbar")

    spacing: 0
    //! TODO AU4: apply padding when StyledToolBarView will have padding property added
    // padding: 6

    model: UndoRedoToolBarModel { }

    sourceComponentCallback: function(type) {
        switch(type) {
        case ToolBarItemType.ACTION: return controlComp
        }

        return null
    }

    Component {
        id: controlComp

        StyledToolBarItem {
            width: 30
            height: width

            navigation.panel: root.navigationPanel
            navigation.order: model.index
        }
    }
}

