/****************************************************************************
** Meta object code from reading C++ file 'Module.h'
**
** Created: Fri Apr 20 15:50:51 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/Interface/Application/Module.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Module.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SCIRun__Gui__ModuleWidget[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      36,   27,   26,   26, 0x05,
      67,   62,   26,   26, 0x05,
     133,  130,   26,   26, 0x05,

 // slots: signature, parameters, type, tag, flags
     191,   26,   26,   26, 0x0a,
     201,   26,   26,   26, 0x0a,
     234,  221,   26,   26, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_SCIRun__Gui__ModuleWidget[] = {
    "SCIRun::Gui::ModuleWidget\0\0moduleId\0"
    "removeModule(std::string)\0desc\0"
    "addConnection(SCIRun::Domain::Networks::ConnectionDescription)\0"
    "id\0connectionDeleted(SCIRun::Domain::Networks::ConnectionId)\0"
    "execute()\0openOptionsDialog()\0"
    "milliseconds\0setExecutionTime(int)\0"
};

const QMetaObject SCIRun::Gui::ModuleWidget::staticMetaObject = {
    { &QFrame::staticMetaObject, qt_meta_stringdata_SCIRun__Gui__ModuleWidget,
      qt_meta_data_SCIRun__Gui__ModuleWidget, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SCIRun::Gui::ModuleWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SCIRun::Gui::ModuleWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SCIRun::Gui::ModuleWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SCIRun__Gui__ModuleWidget))
        return static_cast<void*>(const_cast< ModuleWidget*>(this));
    if (!strcmp(_clname, "NeedsScenePositionProvider"))
        return static_cast< NeedsScenePositionProvider*>(const_cast< ModuleWidget*>(this));
    if (!strcmp(_clname, "Ui::Module"))
        return static_cast< Ui::Module*>(const_cast< ModuleWidget*>(this));
    return QFrame::qt_metacast(_clname);
}

int SCIRun::Gui::ModuleWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: removeModule((*reinterpret_cast< const std::string(*)>(_a[1]))); break;
        case 1: addConnection((*reinterpret_cast< const SCIRun::Domain::Networks::ConnectionDescription(*)>(_a[1]))); break;
        case 2: connectionDeleted((*reinterpret_cast< const SCIRun::Domain::Networks::ConnectionId(*)>(_a[1]))); break;
        case 3: execute(); break;
        case 4: openOptionsDialog(); break;
        case 5: setExecutionTime((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void SCIRun::Gui::ModuleWidget::removeModule(const std::string & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SCIRun::Gui::ModuleWidget::addConnection(const SCIRun::Domain::Networks::ConnectionDescription & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void SCIRun::Gui::ModuleWidget::connectionDeleted(const SCIRun::Domain::Networks::ConnectionId & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
