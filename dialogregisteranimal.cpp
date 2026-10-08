#include <QToolTip>
#include <QMessageBox>
#include <QCompleter>
#include <QLineEdit>

#include "dialogregisteranimal.h"
#include "ui_dialogregisteranimal.h"

#include "hardware/loradev_sim.h"
#include "mainwindow.h"
#include "simtools.h"
#include "herd.h"
#include "animal.h"

#define COL_ERR QColor(255, 50, 50)
#define COL_DONE QColor(50, 255, 50)

void DialogRegisterAnimal::updateExisting()
{
}

void DialogRegisterAnimal::clear()
{
    ui->comboName->lineEdit()->clear();
    // setStatus("", mStatusBackColor);
}


DialogRegisterAnimal::DialogRegisterAnimal(Herd* herd, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogRegisterAnimal)
{
    ui->setupUi(this);
    mHerd = herd;

    mMale = QIcon("://male.svg");
    mFemale = QIcon("://female.svg");
    mMaleNew = QIcon("://male_new.svg");
    mFemaleNew = QIcon("://female_new.svg");

    for( auto i = 0; i < mHerd->animalsCount(); i ++) {
        Animal* a = mHerd->animal(i);
        QListWidgetItem* item = new QListWidgetItem( (a->isMale() ? mMale : mFemale), a->name() );
        ui->listRegister->addItem( item );
    }

    QStringList ls = Animal::names(true) + Animal::names(false);
    QCompleter *completer = new QCompleter(ls, ui->comboName);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    ui->comboName->setCompleter(completer);

    updateExisting();
}

DialogRegisterAnimal::~DialogRegisterAnimal()
{
    delete ui;
}



bool DialogRegisterAnimal::isMale()
{
    return ui->radioMale->isChecked();
}

void DialogRegisterAnimal::on_btnClose_clicked()
{
    close();
}



QString DialogRegisterAnimal::name()
{
    return ui->comboName->currentText();
}


void DialogRegisterAnimal::on_btnAdd_clicked()
{
    if( name().isEmpty()) {
        ui->comboName->setFocus();
        return;
    }

    if( !ui->listRegister->findItems(name(), Qt::MatchExactly).empty() ) {
        gMainWindow->errorMsgBox(QString("An animal with the name %1 already exists!").arg(name()));
        return;
    }

    Animal* a = mHerd->newAnimal( ui->comboName->lineEdit()->text(), ui->radioMale->isChecked() ? true: false );

    if( !a) {
        gMainWindow->errorMsgBox(QString("Error adding animal %1").arg(name()));
        return;
    }


    QListWidgetItem* item = new QListWidgetItem( (a->isMale() ? mMale : mFemale), a->name() );
    ui->listRegister->addItem( item );

    mIsChange = true;
}


void DialogRegisterAnimal::on_btnRemove_clicked()
{
    QList<QListWidgetItem*> items = ui->listRegister->selectedItems();
    if( items.count() <= 0) {
        return;
    }

    QString names;

    for( QListWidgetItem* i:items) {
        names.append(i->text());
        names.append(", ");
    }

    names = names.left(names.length() - 2);

    if( QMessageBox::Yes != QMessageBox::question(this, "Remove animals?", QString("Are you sure you want to remove:%1").arg(names)) ) {
        return;
    }

    for( QListWidgetItem* i:items) {
        if( mHerd->removeAnimal(i->text()) ) {
            delete i;
            mIsChange = true;
        }else {
            gMainWindow->errorMsgBox( QString("Error removing animal: %1").arg(i->text()));
        }
    }


}

void DialogRegisterAnimal::on_listRegister_itemSelectionChanged()
{
    QList<QListWidgetItem*> items = ui->listRegister->selectedItems();
    ui->btnRemove->setEnabled(items.count());
}


void DialogRegisterAnimal::on_btnCancel_clicked()
{
    if( mIsChange ) {
        if( QMessageBox::Yes != QMessageBox::question(this, "Close without register?", QString("You made changes in the herd. Do you want to close it without saving (registering)?")) ) {
            return;
        }
    }
    close();
}


void DialogRegisterAnimal::on_btnRegister_clicked()
{
    if( QMessageBox::Yes != QMessageBox::question(this, "Save changes?", QString("Are you sure you want to overwrite current herd?")) ) {
        return;
    }

    mHerd->storeAnimals();
    gMainWindow->reload();
    mIsChange = false;
}



void DialogRegisterAnimal::on_comboName_editTextChanged(const QString &txt)
{
    if( Animal::names(true).indexOf(txt) >= 0 ) {
        ui->radioMale->setChecked(true);
    }else
    if( Animal::names(false).indexOf(txt) >= 0 ) {
        ui->radioFemale->setChecked(true);
    }
}

