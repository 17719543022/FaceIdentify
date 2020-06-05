#ifndef SUBSCREEN_H
#define SUBSCREEN_H

#include <QDialog>
#include "Include/dataanalysis.h"

namespace Ui {
class SubScreen;
}

class SubScreen : public QDialog
{
    Q_OBJECT

public:
    explicit SubScreen(QWidget *parent = nullptr);
    ~SubScreen();
    void updateVideoFrame(const QPixmap& pixmap);

private slots:
    void animate();

private:
    Ui::SubScreen *ui;
    QMetaObject::Connection animationConnection;
};

#endif // SUBSCREEN_H
