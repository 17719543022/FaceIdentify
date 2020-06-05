#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>

namespace Ui {
class LogIn;
}

class LogIn : public QDialog
{
    Q_OBJECT

public:
    explicit LogIn(QWidget *parent = nullptr);
    ~LogIn();

private slots:
    void on_guanBiPushButton_clicked();

    void on_dengLuPushButton_clicked();

signals:
    void logInSuccess();

private:
    Ui::LogIn *ui;
};

#endif // LOGIN_H
