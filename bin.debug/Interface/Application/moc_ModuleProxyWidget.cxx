/****************************************************************************
** Meta object code from reading C++ file 'ModuleProxyWidget.h'
**
** Created: Fri Apr 20 15:50:51 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/Interface/Application/ModuleProxyWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ModuleProxyWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SCIRun__Gui__ModuleProxyWidget[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      32,   31,   31,   31, 0x05,

 // slots: signature, parameters, type, tag, flags
      43,   31,   31,   31, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_SCIRun__Gui__ModuleProxyWidget[] = {
    "SCIRun::Gui::ModuleProxyWidget\0\0"
    "selected()\0highlightIfSelected()\0"
};

const QMetaObject SCIRun::Gui::ModuleProxyWidget::staticMetaObject = {
    { &QGraphicsProxyWidget::staticMetaObject, qt_meta_stringdata_SCIRun__Gui__ModuleProxyWidget,
      qt_meta_data_SCIRun__Gui__ModuleProxyWidget, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SCIRun::Gui::ModuleProxyWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SCIRun::Gui::ModuleProxyWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SCIRun::Gui::ModuleProxyWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SCIRun__Gui__ModuleProxyWidget))
        return static_cast<void*>(const_cast< ModuleProxyWidget*>(this));
    return QGraphicsProxyWidget::qt_metacast(_clname);
}

int SCIRun::Gui::ModuleProxyWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsProxyWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: selected(); break;
        case 1: highlightIfSelected(); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void SCIRun::Gui::ModuleProxyWidget::selected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
