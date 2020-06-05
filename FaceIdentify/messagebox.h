#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QDialog>

namespace Ui {
class MessageBox;
}

class MessageBox : public QDialog
{
    Q_OBJECT

public:
    explicit MessageBox(QWidget *parent = nullptr);
    ~MessageBox() override;

private slots:
    void on_closePushButton_clicked();

    void on_noPushButton_clicked();

    void on_yesPushButton_clicked();

private:
    Ui::MessageBox *ui;

public:
    void letGoTextLabelShow();
    void letGoIconLabelShow();
    void letGoTextLabelHide();
    void letGoIconLabelHide();

    void letStayTextLabelShow();
    void letStayIconLabelShow();
    void letStayTextLabelHide();
    void letStayIconLabelHide();

    void logInTextLabelShow();
    void logInIconLabelShow();
    void logInTextLabelHide();
    void logInIconLabelHide();

    void closeSysTextLabelShow();
    void closeSysIconLabelShow();
    void closeSysTextLabelHide();
    void closeSysIconLabelHide();

    void softwareTextLabelShow();
    void softwareIconLabelShow();
    void softwareTextLabelHide();
    void softwareIconLabelHide();

    void cheHuiTextLabelShow();
    void cheHuiIconLabelShow();
    void cheHuiTextLabelHide();
    void cheHuiIconLabelHide();

    void hitNullIconLabelShow();
    void hitNullTextLabelShow();
    void hitNullIconLabelHide();
    void hitNullTextLabelHide();

    void closeDivIconLabelShow();
    void closeDivTextLabelShow();
    void closeDivIconLabelHide();
    void closeDivTextLabelHide();

    void openDivIconLabelShow();
    void openDivTextLabelShow();
    void openDivIconLabelHide();
    void openDivTextLabelHide();

    void portErrorIconLabelShow();
    void portErrorTextLabelShow();
    void portErrorIconLabelHide();
    void portErrorTextLabelHide();
};

#endif // MESSAGEBOX_H
