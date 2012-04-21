/****************************************************************************
** Meta object code from reading C++ file 'ModuleDialogBasic.h'
**
** Created: Fri Apr 20 15:50:51 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/Interface/Application/ModuleDialogBasic.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ModuleDialogBasic.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SCIRun__Gui__ModuleDialogGeneric[] = {

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
      39,   34,   33,   33, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_SCIRun__Gui__ModuleDialogGeneric[] = {
    "SCIRun::Gui::ModuleDialogGeneric\0\0"
    "time\0executionTimeChanged(int)\0"
};

const QMetaObject SCIRun::Gui::ModuleDialogGeneric::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_SCIRun__Gui__ModuleDialogGeneric,
      qt_meta_data_SCIRun__Gui__ModuleDialogGeneric, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SCIRun::Gui::ModuleDialogGeneric::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SCIRun::Gui::ModuleDialogGeneric::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SCIRun::Gui::ModuleDialogGeneric::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SCIRun__Gui__ModuleDialogGeneric))
        return static_cast<void*>(const_cast< ModuleDialogGeneric*>(this));
    return QDialog::qt_metacast(_clname);
}

int SCIRun::Gui::ModuleDialogGeneric::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: executionTimeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void SCIRun::Gui::ModuleDialogGeneric::executionTimeChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_SCIRun__Gui__ModuleDialogBasic[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_SCIRun__Gui__ModuleDialogBasic[] = {
    "SCIRun::Gui::ModuleDialogBasic\0"
};

const QMetaObject SCIRun::Gui::ModuleDialogBasic::staticMetaObject = {
    { &ModuleDialogGeneric::staticMetaObject, qt_meta_stringdata_SCIRun__Gui__ModuleDialogBasic,
      qt_meta_data_SCIRun__Gui__ModuleDialogBasic, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SCIRun::Gui::ModuleDialogBasic::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SCIRun::Gui::ModuleDialogBasic::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SCIRun::Gui::ModuleDialogBasic::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SCIRun__Gui__ModuleDialogBasic))
        return static_cast<void*>(const_cast< ModuleDialogBasic*>(this));
    if (!strcmp(_clname, "Ui::ModuleDialogBasic"))
        return static_cast< Ui::ModuleDialogBasic*>(const_cast< ModuleDialogBasic*>(this));
    return ModuleDialogGeneric::qt_metacast(_clname);
}

int SCIRun::Gui::ModuleDialogBasic::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ModuleDialogGeneric::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
