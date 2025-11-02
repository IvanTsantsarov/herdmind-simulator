/********************************************************************************
** Form generated from reading UI file 'focusanim.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FOCUSANIM_H
#define UI_FOCUSANIM_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>

QT_BEGIN_NAMESPACE

class Ui_FocusAnim
{
public:

    void setupUi(QDialog *FocusAnim)
    {
        if (FocusAnim->objectName().isEmpty())
            FocusAnim->setObjectName("FocusAnim");
        FocusAnim->resize(103, 104);

        retranslateUi(FocusAnim);

        QMetaObject::connectSlotsByName(FocusAnim);
    } // setupUi

    void retranslateUi(QDialog *FocusAnim)
    {
        FocusAnim->setWindowTitle(QCoreApplication::translate("FocusAnim", "Dialog", nullptr));
    } // retranslateUi

};

namespace Ui {
    class FocusAnim: public Ui_FocusAnim {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FOCUSANIM_H
