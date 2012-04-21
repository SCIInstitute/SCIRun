/****************************************************************************
** Meta object code from reading C++ file 'Connection.h'
**
** Created: Fri Apr 20 15:50:51 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/Interface/Application/Connection.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Connection.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SCIRun__Gui__ConnectionLine[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      32,   29,   28,   28, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_SCIRun__Gui__ConnectionLine[] = {
    "SCIRun::Gui::ConnectionLine\0\0id\0"
    "deleted(SCIRun::Domain::Networks::ConnectionId)\0"
};

const QMetaObject SCIRun::Gui::ConnectionLine::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_SCIRun__Gui__ConnectionLine,
      qt_meta_data_SCIRun__Gui__ConnectionLine, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SCIRun::Gui::ConnectionLine::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SCIRun::Gui::ConnectionLine::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SCIRun::Gui::ConnectionLine::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SCIRun__Gui__ConnectionLine))
        return static_cast<void*>(const_cast< ConnectionLine*>(this));
    if (!strcmp(_clname, "QGraphicsLineItem"))
        return static_cast< QGraphicsLineItem*>(const_cast< ConnectionLine*>(this));
    return QObject::qt_metacast(_clname);
}

int SCIRun::Gui::ConnectionLine::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: deleted((*reinterpret_cast< const SCIRun::Domain::Networks::ConnectionId(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void SCIRun::Gui::ConnectionLine::deleted(const SCIRun::Domain::Networks::ConnectionId & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
