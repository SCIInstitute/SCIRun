/****************************************************************************
** Meta object code from reading C++ file 'NetworkEditor.h'
**
** Created: Fri Apr 20 15:50:51 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/Interface/Application/NetworkEditor.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'NetworkEditor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SCIRun__Gui__NetworkEditor[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      28,   27,   27,   27, 0x05,
      94,   91,   27,   27, 0x05,

 // slots: signature, parameters, type, tag, flags
     174,  152,   27,   27, 0x0a,
     242,   27,   27,   27, 0x08,
     248,   27,   27,   27, 0x08,
     254,   27,   27,   27, 0x08,
     261,   27,   27,   27, 0x08,
     269,   27,   27,   27, 0x08,
     284,   27,   27,   27, 0x08,
     297,   27,   27,   27, 0x08,
     310,   27,   27,   27, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SCIRun__Gui__NetworkEditor[] = {
    "SCIRun::Gui::NetworkEditor\0\0"
    "addConnection(SCIRun::Domain::Networks::ConnectionDescription)\0"
    "id\0connectionDeleted(SCIRun::Domain::Networks::ConnectionId)\0"
    "name,portInfoProvider\0"
    "addModule(std::string,SCIRun::Domain::Networks::ModuleInfoProvider)\0"
    "del()\0cut()\0copy()\0paste()\0bringToFront()\0"
    "sendToBack()\0properties()\0updateActions()\0"
};

const QMetaObject SCIRun::Gui::NetworkEditor::staticMetaObject = {
    { &QGraphicsView::staticMetaObject, qt_meta_stringdata_SCIRun__Gui__NetworkEditor,
      qt_meta_data_SCIRun__Gui__NetworkEditor, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SCIRun::Gui::NetworkEditor::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SCIRun::Gui::NetworkEditor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SCIRun::Gui::NetworkEditor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SCIRun__Gui__NetworkEditor))
        return static_cast<void*>(const_cast< NetworkEditor*>(this));
    return QGraphicsView::qt_metacast(_clname);
}

int SCIRun::Gui::NetworkEditor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: addConnection((*reinterpret_cast< const SCIRun::Domain::Networks::ConnectionDescription(*)>(_a[1]))); break;
        case 1: connectionDeleted((*reinterpret_cast< const SCIRun::Domain::Networks::ConnectionId(*)>(_a[1]))); break;
        case 2: addModule((*reinterpret_cast< const std::string(*)>(_a[1])),(*reinterpret_cast< const SCIRun::Domain::Networks::ModuleInfoProvider(*)>(_a[2]))); break;
        case 3: del(); break;
        case 4: cut(); break;
        case 5: copy(); break;
        case 6: paste(); break;
        case 7: bringToFront(); break;
        case 8: sendToBack(); break;
        case 9: properties(); break;
        case 10: updateActions(); break;
        default: ;
        }
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void SCIRun::Gui::NetworkEditor::addConnection(const SCIRun::Domain::Networks::ConnectionDescription & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SCIRun::Gui::NetworkEditor::connectionDeleted(const SCIRun::Domain::Networks::ConnectionId & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
