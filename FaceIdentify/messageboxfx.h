#ifndef MESSAGEBOXFX_H
#define MESSAGEBOXFX_H

#include <QDialog>

namespace Ui {
class MessageBoxFX;
}

class MessageBoxFX : public QDialog
{
    Q_OBJECT

public:
    explicit MessageBoxFX(QWidget *parent = nullptr);
    ~MessageBoxFX();

    void imageShow(const QPixmap& pixmap);

private slots:
    void on_closePushButton_clicked();

    void on_noPushButton_clicked();

    void on_yesPushButton_clicked();

private:
    Ui::MessageBoxFX *ui;
};

#endif // MESSAGEBOXFX_H
