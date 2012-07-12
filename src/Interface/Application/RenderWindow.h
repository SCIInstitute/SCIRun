#ifndef RENDERWINDOW_H
#define RENDERWINDOW_H

#include <QDialog>
#include <QVTKWidget.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

namespace Ui {
class RenderWindow;
}

class RenderWindow : public QDialog
{
    Q_OBJECT
    
public:
    explicit RenderWindow(QWidget *parent = 0);
    ~RenderWindow();
    
private:
    Ui::RenderWindow *ui;
    QVTKWidget* mVtkWidget;
    vtkRenderer* mRen;

};

#endif // RENDERWINDOW_H
