//
//  ConsoleScriptingInterface.cpp
//  libraries/script-engine/src
//
//  Created by NeetBhagat on 6/1/17.
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  ConsoleScriptingInterface is responsible for following functionality
//  Printing logs with various tags and grouping on debug Window and Logs/log file.
//  Debugging functionalities like Timer start-end, assertion, trace.
//  To use these functionalities, use "console" object in Javascript files.
//  For examples please refer "scripts/developer/tests/consoleObjectTest.js"
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "ConsoleScriptingInterface.h"

#include <QtCore/QDateTime>

#include "ScriptContext.h"
#include "ScriptEngine.h"
#include "ScriptManager.h"
#include "ScriptValue.h"

#define INDENTATION 4 // 1 Tab - 4 spaces
const QString LINE_SEPARATOR = "\n    ";
const QString SPACE_SEPARATOR = " ";
const QString STACK_TRACE_FORMAT = "\n[Stacktrace]%1%2";
QList<QString> ConsoleScriptingInterface::_groupDetails = QList<QString>();

ScriptValue ConsoleScriptingInterface::info(ScriptContext* context, ScriptEngine* engine) {
    if (ScriptManager* scriptManager = engine->manager()) {
        scriptManager->scriptInfoMessage(appendArguments(context), context->currentFileName(), context->currentLineNumber());
    }
    return engine->nullValue();
}

ScriptValue ConsoleScriptingInterface::log(ScriptContext* context, ScriptEngine* engine) {
    QString message = appendArguments(context);
    if (_groupDetails.count() == 0) {
        if (ScriptManager* scriptManager = engine->manager()) {
            scriptManager->scriptPrintedMessage(message, context->currentFileName(), context->currentLineNumber());
        }
    } else {
        logGroupMessage(message, engine);
    }
    return engine->nullValue();
}

ScriptValue ConsoleScriptingInterface::debug(ScriptContext* context, ScriptEngine* engine) {
    if (ScriptManager* scriptManager = engine->manager()) {
        scriptManager->scriptPrintedMessage(appendArguments(context), context->currentFileName(), context->currentLineNumber());
    }
    return engine->nullValue();
}

ScriptValue ConsoleScriptingInterface::warn(ScriptContext* context, ScriptEngine* engine) {
    if (ScriptManager* scriptManager = engine->manager()) {
        scriptManager->scriptWarningMessage(appendArguments(context), context->currentFileName(), context->currentLineNumber());
    }
    return engine->nullValue();
}

ScriptValue ConsoleScriptingInterface::error(ScriptContext* context, ScriptEngine* engine) {
    if (ScriptManager* scriptManager = engine->manager()) {
        scriptManager->scriptErrorMessage(appendArguments(context), context->currentFileName(), context->currentLineNumber());
    }
    return engine->nullValue();
}

ScriptValue ConsoleScriptingInterface::exception(ScriptContext* context, ScriptEngine* engine) {
    if (ScriptManager* scriptManager = engine->manager()) {
        scriptManager->scriptErrorMessage(appendArguments(context), context->currentFileName(), context->currentLineNumber());
    }
    return engine->nullValue();
}

void ConsoleScriptingInterface::time(QString labelName) {
    _timerDetails.insert(labelName, QDateTime::currentDateTime().toUTC());
    QString message = QString("%1: Timer started").arg(labelName);
    Q_ASSERT(engine);
    if (ScriptManager* scriptManager = engine()->manager()) {
        scriptManager->scriptPrintedMessage(message, context()->currentFileName(), context()->currentLineNumber());
    }
}

void ConsoleScriptingInterface::timeEnd(QString labelName) {
    Q_ASSERT(engine);
    if (ScriptManager* scriptManager = engine()->manager()) {
        if (!_timerDetails.contains(labelName)) {
            scriptManager->scriptErrorMessage("No such label found " + labelName, context()->currentFileName(), context()->currentLineNumber());
            return;
        }

        if (_timerDetails.value(labelName).isNull()) {
            _timerDetails.remove(labelName);
            scriptManager->scriptErrorMessage("Invalid start time for " + labelName, context()->currentFileName(), context()->currentLineNumber());
            return;
        }
        QDateTime _startTime = _timerDetails.value(labelName);
        QDateTime _endTime = QDateTime::currentDateTime().toUTC();
        qint64 diffInMS = _startTime.msecsTo(_endTime);

        QString message = QString("%1: %2ms").arg(labelName).arg(QString::number(diffInMS));
        _timerDetails.remove(labelName);

        scriptManager->scriptPrintedMessage(message, context()->currentFileName(), context()->currentLineNumber());
    }
}

ScriptValue ConsoleScriptingInterface::assertion(ScriptContext* context, ScriptEngine* engine) {
    QString message;
    bool condition = false;
    for (int i = 0; i < context->argumentCount(); i++) {
        if (i == 0) {
            condition = context->argument(i).toBool(); // accept first value as condition
        } else {
            message += SPACE_SEPARATOR + context->argument(i).toString(); // accept other parameters as message
        }
    }

    QString assertionResult;
    if (!condition) {
        if (message.isEmpty()) {
            assertionResult = "Assertion failed";
        } else {
            assertionResult = QString("Assertion failed : %1").arg(message);
        }
        if (ScriptManager* scriptManager = engine->manager()) {
            scriptManager->scriptErrorMessage(assertionResult, context->currentFileName(), context->currentLineNumber());
        }
    }
    return engine->nullValue();
}

void ConsoleScriptingInterface::trace() {
    Q_ASSERT(engine);
    ScriptEnginePointer scriptEngine = engine();
    if (ScriptManager* scriptManager = scriptEngine->manager()) {
        scriptManager->scriptPrintedMessage
            (QString(STACK_TRACE_FORMAT).arg(LINE_SEPARATOR,
            scriptEngine->currentContext()->backtrace().join(LINE_SEPARATOR)), context()->currentFileName(), context()->currentLineNumber());
    }
}

void ConsoleScriptingInterface::clear() {
    Q_ASSERT(engine);
    if (ScriptManager* scriptManager = engine()->manager()) {
        scriptManager->clearDebugLogWindow();
    }
}

ScriptValue ConsoleScriptingInterface::group(ScriptContext* context, ScriptEngine* engine) {
    logGroupMessage(context->argument(0).toString(), engine); // accept first parameter as label
    _groupDetails.push_back(context->argument(0).toString());
    return engine->nullValue();
}

ScriptValue ConsoleScriptingInterface::groupCollapsed(ScriptContext* context, ScriptEngine* engine) {
    logGroupMessage(context->argument(0).toString(), engine); // accept first parameter as label
    _groupDetails.push_back(context->argument(0).toString());
    return engine->nullValue();
}

ScriptValue ConsoleScriptingInterface::groupEnd(ScriptContext* context, ScriptEngine* engine) {
    ConsoleScriptingInterface::_groupDetails.removeLast();
    return engine->nullValue();
}

QString ConsoleScriptingInterface::appendArguments(ScriptContext* context) {
    QString message;
    for (int i = 0; i < context->argumentCount(); i++) {
        if (i > 0) {
            message += SPACE_SEPARATOR;
        }
        message += context->argument(i).toString();
    }
    return message;
}

void ConsoleScriptingInterface::logGroupMessage(QString message, ScriptEngine* engine) {
    int _addSpaces = _groupDetails.count() * INDENTATION;
    QString logMessage;
    for (int i = 0; i < _addSpaces; i++) {
        logMessage.append(SPACE_SEPARATOR);
    }
    logMessage.append(message);
    if (ScriptManager* scriptManager = engine->manager()) {
        scriptManager->scriptPrintedMessage(logMessage, context()->currentFileName(), context()->currentLineNumber());
    }
}
