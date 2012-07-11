#ifndef RENDERWINDOW_H
#define RENDERWINDOW_H

#include <QDialog>

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
};

#endif // RENDERWINDOW_H
