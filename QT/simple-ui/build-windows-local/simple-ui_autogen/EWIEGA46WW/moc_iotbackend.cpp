/****************************************************************************
** Meta object code from reading C++ file 'iotbackend.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../iotbackend.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'iotbackend.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN10IoTBackendE_t {};
} // unnamed namespace

template <> constexpr inline auto IoTBackend::qt_create_metaobjectdata<qt_meta_tag_ZN10IoTBackendE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "IoTBackend",
        "telemetryChanged",
        "",
        "relay1Changed",
        "state",
        "relay2Changed",
        "fanSpeedChanged",
        "speed",
        "brightnessChanged",
        "val",
        "connectionChanged",
        "deviceNameChanged",
        "notification",
        "title",
        "message",
        "setRelay1",
        "on",
        "setRelay2",
        "setFanSpeed",
        "setBrightness",
        "toggleRelay1",
        "toggleRelay2",
        "rebootDevice",
        "syncData",
        "toggleConnection",
        "onSimulateTelemetry",
        "temperature",
        "humidity",
        "voltage",
        "rssi",
        "relay1",
        "relay2",
        "fanSpeed",
        "brightness",
        "connected",
        "connectionStatus",
        "deviceName",
        "lastUpdated"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'telemetryChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'relay1Changed'
        QtMocHelpers::SignalData<void(bool)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 4 },
        }}),
        // Signal 'relay2Changed'
        QtMocHelpers::SignalData<void(bool)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 4 },
        }}),
        // Signal 'fanSpeedChanged'
        QtMocHelpers::SignalData<void(int)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 7 },
        }}),
        // Signal 'brightnessChanged'
        QtMocHelpers::SignalData<void(int)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 9 },
        }}),
        // Signal 'connectionChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'deviceNameChanged'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'notification'
        QtMocHelpers::SignalData<void(const QString &, const QString &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 13 }, { QMetaType::QString, 14 },
        }}),
        // Slot 'setRelay1'
        QtMocHelpers::SlotData<void(bool)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 16 },
        }}),
        // Slot 'setRelay2'
        QtMocHelpers::SlotData<void(bool)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 16 },
        }}),
        // Slot 'setFanSpeed'
        QtMocHelpers::SlotData<void(int)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 7 },
        }}),
        // Slot 'setBrightness'
        QtMocHelpers::SlotData<void(int)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 9 },
        }}),
        // Slot 'toggleRelay1'
        QtMocHelpers::SlotData<void()>(20, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'toggleRelay2'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'rebootDevice'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'syncData'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'toggleConnection'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onSimulateTelemetry'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'temperature'
        QtMocHelpers::PropertyData<double>(26, QMetaType::Double, QMC::DefaultPropertyFlags, 0),
        // property 'humidity'
        QtMocHelpers::PropertyData<double>(27, QMetaType::Double, QMC::DefaultPropertyFlags, 0),
        // property 'voltage'
        QtMocHelpers::PropertyData<double>(28, QMetaType::Double, QMC::DefaultPropertyFlags, 0),
        // property 'rssi'
        QtMocHelpers::PropertyData<int>(29, QMetaType::Int, QMC::DefaultPropertyFlags, 0),
        // property 'relay1'
        QtMocHelpers::PropertyData<bool>(30, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 1),
        // property 'relay2'
        QtMocHelpers::PropertyData<bool>(31, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 2),
        // property 'fanSpeed'
        QtMocHelpers::PropertyData<int>(32, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 3),
        // property 'brightness'
        QtMocHelpers::PropertyData<int>(33, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 4),
        // property 'connected'
        QtMocHelpers::PropertyData<bool>(34, QMetaType::Bool, QMC::DefaultPropertyFlags, 5),
        // property 'connectionStatus'
        QtMocHelpers::PropertyData<QString>(35, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'deviceName'
        QtMocHelpers::PropertyData<QString>(36, QMetaType::QString, QMC::DefaultPropertyFlags, 6),
        // property 'lastUpdated'
        QtMocHelpers::PropertyData<QString>(37, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<IoTBackend, qt_meta_tag_ZN10IoTBackendE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject IoTBackend::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10IoTBackendE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10IoTBackendE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10IoTBackendE_t>.metaTypes,
    nullptr
} };

void IoTBackend::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<IoTBackend *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->telemetryChanged(); break;
        case 1: _t->relay1Changed((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 2: _t->relay2Changed((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 3: _t->fanSpeedChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->brightnessChanged((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->connectionChanged(); break;
        case 6: _t->deviceNameChanged(); break;
        case 7: _t->notification((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 8: _t->setRelay1((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 9: _t->setRelay2((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 10: _t->setFanSpeed((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 11: _t->setBrightness((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 12: _t->toggleRelay1(); break;
        case 13: _t->toggleRelay2(); break;
        case 14: _t->rebootDevice(); break;
        case 15: _t->syncData(); break;
        case 16: _t->toggleConnection(); break;
        case 17: _t->onSimulateTelemetry(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (IoTBackend::*)()>(_a, &IoTBackend::telemetryChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (IoTBackend::*)(bool )>(_a, &IoTBackend::relay1Changed, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (IoTBackend::*)(bool )>(_a, &IoTBackend::relay2Changed, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (IoTBackend::*)(int )>(_a, &IoTBackend::fanSpeedChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (IoTBackend::*)(int )>(_a, &IoTBackend::brightnessChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (IoTBackend::*)()>(_a, &IoTBackend::connectionChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (IoTBackend::*)()>(_a, &IoTBackend::deviceNameChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (IoTBackend::*)(const QString & , const QString & )>(_a, &IoTBackend::notification, 7))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<double*>(_v) = _t->temperature(); break;
        case 1: *reinterpret_cast<double*>(_v) = _t->humidity(); break;
        case 2: *reinterpret_cast<double*>(_v) = _t->voltage(); break;
        case 3: *reinterpret_cast<int*>(_v) = _t->rssi(); break;
        case 4: *reinterpret_cast<bool*>(_v) = _t->relay1(); break;
        case 5: *reinterpret_cast<bool*>(_v) = _t->relay2(); break;
        case 6: *reinterpret_cast<int*>(_v) = _t->fanSpeed(); break;
        case 7: *reinterpret_cast<int*>(_v) = _t->brightness(); break;
        case 8: *reinterpret_cast<bool*>(_v) = _t->isConnected(); break;
        case 9: *reinterpret_cast<QString*>(_v) = _t->connectionStatus(); break;
        case 10: *reinterpret_cast<QString*>(_v) = _t->deviceName(); break;
        case 11: *reinterpret_cast<QString*>(_v) = _t->lastUpdated(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 4: _t->setRelay1(*reinterpret_cast<bool*>(_v)); break;
        case 5: _t->setRelay2(*reinterpret_cast<bool*>(_v)); break;
        case 6: _t->setFanSpeed(*reinterpret_cast<int*>(_v)); break;
        case 7: _t->setBrightness(*reinterpret_cast<int*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *IoTBackend::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *IoTBackend::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10IoTBackendE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int IoTBackend::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 18)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 18;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void IoTBackend::telemetryChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void IoTBackend::relay1Changed(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void IoTBackend::relay2Changed(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void IoTBackend::fanSpeedChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void IoTBackend::brightnessChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void IoTBackend::connectionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void IoTBackend::deviceNameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void IoTBackend::notification(const QString & _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1, _t2);
}
QT_WARNING_POP
