/****************************************************************************
** Meta object code from reading C++ file 'Port.h'
**
** Created: Fri Apr 20 15:50:51 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/Interface/Application/Port.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Port.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SCIRun__Gui__PortWidget[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      30,   25,   24,   24, 0x05,
      97,   94,   24,   24, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_SCIRun__Gui__PortWidget[] = {
    "SCIRun::Gui::PortWidget\0\0desc\0"
    "connectionMade(SCIRun::Domain::Networks::ConnectionDescription)\0"
    "id\0connectionDeleted(SCIRun::Domain::Networks::ConnectionId)\0"
};

const QMetaObject SCIRun::Gui::PortWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_SCIRun__Gui__PortWidget,
      qt_meta_data_SCIRun__Gui__PortWidget, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SCIRun::Gui::PortWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SCIRun::Gui::PortWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SCIRun::Gui::PortWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SCIRun__Gui__PortWidget))
        return static_cast<void*>(const_cast< PortWidget*>(this));
    if (!strcmp(_clname, "NeedsScenePositionProvider"))
        return static_cast< NeedsScenePositionProvider*>(const_cast< PortWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int SCIRun::Gui::PortWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: connectionMade((*reinterpret_cast< const SCIRun::Domain::Networks::ConnectionDescription(*)>(_a[1]))); break;
        case 1: connectionDeleted((*reinterpret_cast< const SCIRun::Domain::Networks::ConnectionId(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void SCIRun::Gui::PortWidget::connectionMade(const SCIRun::Domain::Networks::ConnectionDescription & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SCIRun::Gui::PortWidget::connectionDeleted(const SCIRun::Domain::Networks::ConnectionId & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
