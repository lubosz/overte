//
//  MenuItemProperties.cpp
//  libraries/script-engine/src
//
//  Created by Brad Hefta-Gaub on 1/28/14.
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "MenuItemProperties.h"

#include <QDebug>
#include <RegisteredMetaTypes.h>

#include "ScriptEngine.h"
#include "ScriptEngineCast.h"
#include "ScriptValue.h"
#include "ScriptManager.h"

STATIC_SCRIPT_TYPES_INITIALIZER((+[](ScriptManager* manager){
    auto scriptEngine = manager->engine().get();
    scriptRegisterMetaType<MenuItemProperties, menuItemPropertiesToScriptValue, menuItemPropertiesFromScriptValue>(scriptEngine, "MenuItemProperties");
}));

MenuItemProperties::MenuItemProperties(const QString& menuName, const QString& menuItemName,
                        const QString& shortcutKey, bool checkable, bool checked, bool separator) :
    menuName(menuName),
    menuItemName(menuItemName),
    shortcutKey(shortcutKey),
    shortcutKeyEvent(),
    shortcutKeySequence(shortcutKey),
    isCheckable(checkable),
    isChecked(checked),
    isSeparator(separator)
{
}

MenuItemProperties::MenuItemProperties(const QString& menuName, const QString& menuItemName,
                        const KeyEvent& shortcutKeyEvent, bool checkable, bool checked, bool separator) :
    menuName(menuName),
    menuItemName(menuItemName),
    shortcutKeyEvent(shortcutKeyEvent),
    shortcutKeySequence(shortcutKeyEvent),
    isCheckable(checkable),
    isChecked(checked),
    isSeparator(separator)
{
}

ScriptValue menuItemPropertiesToScriptValue(ScriptEngine* engine, const MenuItemProperties& properties) {
    ScriptValue obj = engine->newObject();
    // not supported
    return obj;
}

/*@jsdoc
 * A set of properties that can be passed to {@link Menu.addMenuItem} to create a new menu item.
 *
 * If none of the properties, <code>position</code>, <code>beforeItem</code>, <code>afterItem</code>, or <code>grouping</code> 
 * are specified, the menu item will be placed at the end of the menu.
 *
 * @typedef {object} Menu.MenuItemProperties
 * @property {string} menuName - Name of the menu. Nested menus can be described using the ">" character.
 * @property {string} menuItemName - Name of the menu item.
 * @property {boolean} [isCheckable=false] - Whether or not the menu item is checkable.
 * @property {boolean} [isChecked=false] - Whether or not the menu item is checked.
 * @property {boolean} [isSeparator=false] - Whether or not the menu item is a separator.
 * @property {string} [shortcutKey] - A shortcut key that triggers the menu item.
 * @property {KeyEvent} [shortcutKeyEvent] - A {@link KeyEvent} that specifies a key that triggers the menu item.
 * @property {number} [position] - The position to place the new menu item. An integer number with <code>0</code> being the first
 *     menu item.
 * @property {string} [beforeItem] - The name of the menu item to place this menu item before.
 * @property {string} [afterItem] - The name of the menu item to place this menu item after.
 * @property {string} [grouping] - The name of grouping to add this menu item to.
 */
bool menuItemPropertiesFromScriptValue(const ScriptValue& object, MenuItemProperties& properties) {
    if (!object.hasProperty("menuName")) {
        return false;
    }
    if (!object.hasProperty("menuItemName")) {
        return false;
    }
    properties.menuName = object.property("menuName").toVariant().toString();
    properties.menuItemName = object.property("menuItemName").toVariant().toString();
    properties.isCheckable = object.property("isCheckable").toVariant().toBool();
    properties.isChecked = object.property("isChecked").toVariant().toBool();
    properties.isSeparator = object.property("isSeparator").toVariant().toBool();

    // handle the shortcut key options in order...
    ScriptValue shortcutKeyValue = object.property("shortcutKey");
    if (shortcutKeyValue.isValid()) {
        properties.shortcutKey = shortcutKeyValue.toVariant().toString();
        properties.shortcutKeySequence = properties.shortcutKey;
    } else {
        ScriptValue shortcutKeyEventValue = object.property("shortcutKeyEvent");
        if (shortcutKeyEventValue.isValid()) {
            KeyEvent::fromScriptValue(shortcutKeyEventValue, properties.shortcutKeyEvent);
            properties.shortcutKeySequence = properties.shortcutKeyEvent;
        }
    }

    if (object.property("position").isValid()) {
        properties.position = object.property("position").toVariant().toInt();
    }
    properties.beforeItem = object.property("beforeItem").toVariant().toString();
    properties.afterItem = object.property("afterItem").toVariant().toString();
    properties.grouping = object.property("grouping").toVariant().toString();
    return true;
}


