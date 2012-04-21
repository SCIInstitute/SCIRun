/****************************************************************************
** Meta object code from reading C++ file 'NetworkEditorControllerGuiProxy.h'
**
** Created: Fri Apr 20 15:50:51 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/Interface/Application/NetworkEditorControllerGuiProxy.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NetworkEditorControllerGuiProxy.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SCIRun__Gui__NetworkEditorControllerGuiProxy[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      68,   46,   45,   45, 0x05,

 // slots: signature, parameters, type, tag, flags
     149,  138,   45,   45, 0x0a,
     175,  172,   45,   45, 0x0a,
     206,  201,   45,   45, 0x0a,
     269,  172,   45,   45, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_SCIRun__Gui__NetworkEditorControllerGuiProxy[] = {
    "SCIRun::Gui::NetworkEditorControllerGuiProxy\0"
    "\0name,portInfoProvider\0"
    "moduleAdded(std::string,SCIRun::Domain::Networks::ModuleInfoProvider)\0"
    "moduleName\0addModule(std::string)\0id\0"
    "removeModule(std::string)\0desc\0"
    "addConnection(SCIRun::Domain::Networks::ConnectionDescription)\0"
    "removeConnection(SCIRun::Domain::Networks::ConnectionId)\0"
};

const QMetaObject SCIRun::Gui::NetworkEditorControllerGuiProxy::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_SCIRun__Gui__NetworkEditorControllerGuiProxy,
      qt_meta_data_SCIRun__Gui__NetworkEditorControllerGuiProxy, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SCIRun::Gui::NetworkEditorControllerGuiProxy::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SCIRun::Gui::NetworkEditorControllerGuiProxy::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SCIRun::Gui::NetworkEditorControllerGuiProxy::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SCIRun__Gui__NetworkEditorControllerGuiProxy))
        return static_cast<void*>(const_cast< NetworkEditorControllerGuiProxy*>(this));
    return QObject::qt_metacast(_clname);
}

int SCIRun::Gui::NetworkEditorControllerGuiProxy::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: moduleAdded((*reinterpret_cast< const std::string(*)>(_a[1])),(*reinterpret_cast< const SCIRun::Domain::Networks::ModuleInfoProvider(*)>(_a[2]))); break;
        case 1: addModule((*reinterpret_cast< const std::string(*)>(_a[1]))); break;
        case 2: removeModule((*reinterpret_cast< const std::string(*)>(_a[1]))); break;
        case 3: addConnection((*reinterpret_cast< const SCIRun::Domain::Networks::ConnectionDescription(*)>(_a[1]))); break;
        case 4: removeConnection((*reinterpret_cast< const SCIRun::Domain::Networks::ConnectionId(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void SCIRun::Gui::NetworkEditorControllerGuiProxy::moduleAdded(const std::string & _t1, const SCIRun::Domain::Networks::ModuleInfoProvider & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
