/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout_2;
    QVBoxLayout *mainVerticalLayout;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout_3;
    QTextEdit *editInfo;
    QTableWidget *table;
    QVBoxLayout *verticalLayoutParams;
    QWidget *widgetGen;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout_3;
    QLineEdit *editTime;
    QPushButton *btnGenerate;
    QHBoxLayout *horizontalLayout_6;
    QCheckBox *checkShepard;
    QCheckBox *checkRecursiveCollision;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_5;
    QHBoxLayout *horizontalLayout_4;
    QDoubleSpinBox *spinCenterLong;
    QLabel *label_19;
    QDoubleSpinBox *spinCenterLat;
    QScrollArea *scrollAreaParams;
    QWidget *scrollAreaWidgetContents_2;
    QVBoxLayout *verticalLayout;
    QWidget *scrollAreaParamsWidget;
    QVBoxLayout *verticalLayout_2;
    QFormLayout *formLayout_5;
    QLabel *label_3;
    QDoubleSpinBox *doubleSpinAnimalSize;
    QLabel *label;
    QSpinBox *spinAnimalsCount;
    QLabel *label_4;
    QDoubleSpinBox *doubleSpinArea;
    QLabel *label_2;
    QSpinBox *spinCollarsPercentage;
    QCheckBox *checkParamsHerding;
    QWidget *widgetSim;
    QGridLayout *gridLayout_2;
    QFormLayout *formLayout_2;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label_7;
    QLabel *label_8;
    QLabel *label_9;
    QLabel *label_10;
    QLabel *label_11;
    QLabel *label_12;
    QLabel *label_13;
    QDoubleSpinBox *spinAttrPower;
    QDoubleSpinBox *spinAttrDist;
    QDoubleSpinBox *spinRepDist;
    QDoubleSpinBox *spinCollDist;
    QDoubleSpinBox *spinMaxSpeed;
    QDoubleSpinBox *spinFriction;
    QDoubleSpinBox *spinRotFad;
    QDoubleSpinBox *spinTransDist;
    QDoubleSpinBox *spinTransAngle;
    QCheckBox *checkParamsG;
    QWidget *widgetGrazing;
    QFormLayout *formLayout_3;
    QFormLayout *formLayout;
    QLabel *label_15;
    QDoubleSpinBox *spinMeadowRadius;
    QLabel *label_17;
    QDoubleSpinBox *spinLawnRadius;
    QSpinBox *spinAnimalsPerLawn;
    QLabel *label_18;
    QDoubleSpinBox *spinMeadowCapacity;
    QLabel *label_16;
    QDoubleSpinBox *spinAnimalGrazingCapacity;
    QLabel *label_14;
    QSpacerItem *verticalScrollSpacer;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(431, 696);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Maximum, QSizePolicy::Policy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        horizontalLayout_2 = new QHBoxLayout(centralwidget);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        mainVerticalLayout = new QVBoxLayout();
        mainVerticalLayout->setObjectName("mainVerticalLayout");

        horizontalLayout_2->addLayout(mainVerticalLayout);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName("verticalLayout_3");
        editInfo = new QTextEdit(centralwidget);
        editInfo->setObjectName("editInfo");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(editInfo->sizePolicy().hasHeightForWidth());
        editInfo->setSizePolicy(sizePolicy1);
        editInfo->setReadOnly(true);

        verticalLayout_3->addWidget(editInfo);

        table = new QTableWidget(centralwidget);
        table->setObjectName("table");
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(table->sizePolicy().hasHeightForWidth());
        table->setSizePolicy(sizePolicy2);
        table->setMinimumSize(QSize(120, 120));
        table->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
        table->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
        table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
        table->horizontalHeader()->setCascadingSectionResizes(true);
        table->horizontalHeader()->setDefaultSectionSize(40);
        table->horizontalHeader()->setHighlightSections(false);
        table->verticalHeader()->setCascadingSectionResizes(false);

        verticalLayout_3->addWidget(table);


        horizontalLayout->addLayout(verticalLayout_3);

        verticalLayoutParams = new QVBoxLayout();
        verticalLayoutParams->setObjectName("verticalLayoutParams");
        widgetGen = new QWidget(centralwidget);
        widgetGen->setObjectName("widgetGen");
        widgetGen->setMinimumSize(QSize(0, 100));
        gridLayout = new QGridLayout(widgetGen);
        gridLayout->setObjectName("gridLayout");
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        editTime = new QLineEdit(widgetGen);
        editTime->setObjectName("editTime");
        editTime->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);
        editTime->setReadOnly(true);

        horizontalLayout_3->addWidget(editTime);

        btnGenerate = new QPushButton(widgetGen);
        btnGenerate->setObjectName("btnGenerate");

        horizontalLayout_3->addWidget(btnGenerate);


        gridLayout->addLayout(horizontalLayout_3, 1, 0, 1, 1);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName("horizontalLayout_6");
        checkShepard = new QCheckBox(widgetGen);
        checkShepard->setObjectName("checkShepard");
        checkShepard->setEnabled(false);

        horizontalLayout_6->addWidget(checkShepard);

        checkRecursiveCollision = new QCheckBox(widgetGen);
        checkRecursiveCollision->setObjectName("checkRecursiveCollision");

        horizontalLayout_6->addWidget(checkRecursiveCollision);


        gridLayout->addLayout(horizontalLayout_6, 2, 0, 1, 1);


        verticalLayoutParams->addWidget(widgetGen);

        groupBox = new QGroupBox(centralwidget);
        groupBox->setObjectName("groupBox");
        groupBox->setMinimumSize(QSize(0, 50));
        horizontalLayout_5 = new QHBoxLayout(groupBox);
        horizontalLayout_5->setSpacing(0);
        horizontalLayout_5->setObjectName("horizontalLayout_5");
        horizontalLayout_5->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        spinCenterLong = new QDoubleSpinBox(groupBox);
        spinCenterLong->setObjectName("spinCenterLong");

        horizontalLayout_4->addWidget(spinCenterLong);

        label_19 = new QLabel(groupBox);
        label_19->setObjectName("label_19");
        sizePolicy.setHeightForWidth(label_19->sizePolicy().hasHeightForWidth());
        label_19->setSizePolicy(sizePolicy);

        horizontalLayout_4->addWidget(label_19);

        spinCenterLat = new QDoubleSpinBox(groupBox);
        spinCenterLat->setObjectName("spinCenterLat");

        horizontalLayout_4->addWidget(spinCenterLat);


        horizontalLayout_5->addLayout(horizontalLayout_4);


        verticalLayoutParams->addWidget(groupBox);

        scrollAreaParams = new QScrollArea(centralwidget);
        scrollAreaParams->setObjectName("scrollAreaParams");
        sizePolicy2.setHeightForWidth(scrollAreaParams->sizePolicy().hasHeightForWidth());
        scrollAreaParams->setSizePolicy(sizePolicy2);
        scrollAreaParams->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
        scrollAreaParams->setWidgetResizable(true);
        scrollAreaWidgetContents_2 = new QWidget();
        scrollAreaWidgetContents_2->setObjectName("scrollAreaWidgetContents_2");
        scrollAreaWidgetContents_2->setGeometry(QRect(0, 0, 158, 481));
        verticalLayout = new QVBoxLayout(scrollAreaWidgetContents_2);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        scrollAreaParamsWidget = new QWidget(scrollAreaWidgetContents_2);
        scrollAreaParamsWidget->setObjectName("scrollAreaParamsWidget");
        verticalLayout_2 = new QVBoxLayout(scrollAreaParamsWidget);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        formLayout_5 = new QFormLayout();
        formLayout_5->setObjectName("formLayout_5");
        formLayout_5->setHorizontalSpacing(0);
        formLayout_5->setVerticalSpacing(0);
        label_3 = new QLabel(scrollAreaParamsWidget);
        label_3->setObjectName("label_3");
        label_3->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        formLayout_5->setWidget(1, QFormLayout::ItemRole::LabelRole, label_3);

        doubleSpinAnimalSize = new QDoubleSpinBox(scrollAreaParamsWidget);
        doubleSpinAnimalSize->setObjectName("doubleSpinAnimalSize");

        formLayout_5->setWidget(1, QFormLayout::ItemRole::FieldRole, doubleSpinAnimalSize);

        label = new QLabel(scrollAreaParamsWidget);
        label->setObjectName("label");
        label->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        formLayout_5->setWidget(2, QFormLayout::ItemRole::LabelRole, label);

        spinAnimalsCount = new QSpinBox(scrollAreaParamsWidget);
        spinAnimalsCount->setObjectName("spinAnimalsCount");
        spinAnimalsCount->setMinimum(1);
        spinAnimalsCount->setMaximum(1000);

        formLayout_5->setWidget(2, QFormLayout::ItemRole::FieldRole, spinAnimalsCount);

        label_4 = new QLabel(scrollAreaParamsWidget);
        label_4->setObjectName("label_4");
        label_4->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        formLayout_5->setWidget(3, QFormLayout::ItemRole::LabelRole, label_4);

        doubleSpinArea = new QDoubleSpinBox(scrollAreaParamsWidget);
        doubleSpinArea->setObjectName("doubleSpinArea");

        formLayout_5->setWidget(3, QFormLayout::ItemRole::FieldRole, doubleSpinArea);

        label_2 = new QLabel(scrollAreaParamsWidget);
        label_2->setObjectName("label_2");
        label_2->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        formLayout_5->setWidget(4, QFormLayout::ItemRole::LabelRole, label_2);

        spinCollarsPercentage = new QSpinBox(scrollAreaParamsWidget);
        spinCollarsPercentage->setObjectName("spinCollarsPercentage");
        spinCollarsPercentage->setMinimum(1);
        spinCollarsPercentage->setMaximum(100);

        formLayout_5->setWidget(4, QFormLayout::ItemRole::FieldRole, spinCollarsPercentage);


        verticalLayout_2->addLayout(formLayout_5);

        checkParamsHerding = new QCheckBox(scrollAreaParamsWidget);
        checkParamsHerding->setObjectName("checkParamsHerding");

        verticalLayout_2->addWidget(checkParamsHerding);

        widgetSim = new QWidget(scrollAreaParamsWidget);
        widgetSim->setObjectName("widgetSim");
        widgetSim->setMinimumSize(QSize(0, 100));
        gridLayout_2 = new QGridLayout(widgetSim);
        gridLayout_2->setObjectName("gridLayout_2");
        formLayout_2 = new QFormLayout();
        formLayout_2->setObjectName("formLayout_2");
        label_5 = new QLabel(widgetSim);
        label_5->setObjectName("label_5");
        label_5->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        formLayout_2->setWidget(0, QFormLayout::ItemRole::LabelRole, label_5);

        label_6 = new QLabel(widgetSim);
        label_6->setObjectName("label_6");
        label_6->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        formLayout_2->setWidget(1, QFormLayout::ItemRole::LabelRole, label_6);

        label_7 = new QLabel(widgetSim);
        label_7->setObjectName("label_7");
        label_7->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        formLayout_2->setWidget(2, QFormLayout::ItemRole::LabelRole, label_7);

        label_8 = new QLabel(widgetSim);
        label_8->setObjectName("label_8");
        label_8->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        formLayout_2->setWidget(3, QFormLayout::ItemRole::LabelRole, label_8);

        label_9 = new QLabel(widgetSim);
        label_9->setObjectName("label_9");
        label_9->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        formLayout_2->setWidget(4, QFormLayout::ItemRole::LabelRole, label_9);

        label_10 = new QLabel(widgetSim);
        label_10->setObjectName("label_10");
        label_10->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        formLayout_2->setWidget(5, QFormLayout::ItemRole::LabelRole, label_10);

        label_11 = new QLabel(widgetSim);
        label_11->setObjectName("label_11");
        label_11->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        formLayout_2->setWidget(6, QFormLayout::ItemRole::LabelRole, label_11);

        label_12 = new QLabel(widgetSim);
        label_12->setObjectName("label_12");
        label_12->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        formLayout_2->setWidget(7, QFormLayout::ItemRole::LabelRole, label_12);

        label_13 = new QLabel(widgetSim);
        label_13->setObjectName("label_13");
        label_13->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        formLayout_2->setWidget(8, QFormLayout::ItemRole::LabelRole, label_13);

        spinAttrPower = new QDoubleSpinBox(widgetSim);
        spinAttrPower->setObjectName("spinAttrPower");
        spinAttrPower->setDecimals(3);
        spinAttrPower->setMinimum(0.001000000000000);
        spinAttrPower->setMaximum(1000.000000000000000);
        spinAttrPower->setSingleStep(0.001000000000000);
        spinAttrPower->setValue(0.001000000000000);

        formLayout_2->setWidget(0, QFormLayout::ItemRole::FieldRole, spinAttrPower);

        spinAttrDist = new QDoubleSpinBox(widgetSim);
        spinAttrDist->setObjectName("spinAttrDist");
        spinAttrDist->setDecimals(3);
        spinAttrDist->setMinimum(0.001000000000000);
        spinAttrDist->setMaximum(1000.000000000000000);
        spinAttrDist->setSingleStep(0.001000000000000);
        spinAttrDist->setValue(0.001000000000000);

        formLayout_2->setWidget(1, QFormLayout::ItemRole::FieldRole, spinAttrDist);

        spinRepDist = new QDoubleSpinBox(widgetSim);
        spinRepDist->setObjectName("spinRepDist");
        spinRepDist->setDecimals(3);
        spinRepDist->setMinimum(0.001000000000000);
        spinRepDist->setMaximum(1000.000000000000000);
        spinRepDist->setSingleStep(0.001000000000000);
        spinRepDist->setValue(0.001000000000000);

        formLayout_2->setWidget(2, QFormLayout::ItemRole::FieldRole, spinRepDist);

        spinCollDist = new QDoubleSpinBox(widgetSim);
        spinCollDist->setObjectName("spinCollDist");
        spinCollDist->setDecimals(3);
        spinCollDist->setMinimum(0.001000000000000);
        spinCollDist->setMaximum(1000.000000000000000);
        spinCollDist->setSingleStep(0.001000000000000);
        spinCollDist->setValue(0.001000000000000);

        formLayout_2->setWidget(3, QFormLayout::ItemRole::FieldRole, spinCollDist);

        spinMaxSpeed = new QDoubleSpinBox(widgetSim);
        spinMaxSpeed->setObjectName("spinMaxSpeed");
        spinMaxSpeed->setDecimals(3);
        spinMaxSpeed->setMinimum(0.001000000000000);
        spinMaxSpeed->setMaximum(1000.000000000000000);
        spinMaxSpeed->setSingleStep(0.001000000000000);
        spinMaxSpeed->setValue(0.001000000000000);

        formLayout_2->setWidget(4, QFormLayout::ItemRole::FieldRole, spinMaxSpeed);

        spinFriction = new QDoubleSpinBox(widgetSim);
        spinFriction->setObjectName("spinFriction");
        spinFriction->setDecimals(3);
        spinFriction->setMinimum(0.001000000000000);
        spinFriction->setMaximum(1000.000000000000000);
        spinFriction->setSingleStep(0.001000000000000);
        spinFriction->setValue(0.001000000000000);

        formLayout_2->setWidget(5, QFormLayout::ItemRole::FieldRole, spinFriction);

        spinRotFad = new QDoubleSpinBox(widgetSim);
        spinRotFad->setObjectName("spinRotFad");
        spinRotFad->setDecimals(3);
        spinRotFad->setMinimum(0.001000000000000);
        spinRotFad->setMaximum(1000.000000000000000);
        spinRotFad->setSingleStep(0.001000000000000);
        spinRotFad->setValue(0.001000000000000);

        formLayout_2->setWidget(6, QFormLayout::ItemRole::FieldRole, spinRotFad);

        spinTransDist = new QDoubleSpinBox(widgetSim);
        spinTransDist->setObjectName("spinTransDist");
        spinTransDist->setDecimals(3);
        spinTransDist->setMinimum(0.001000000000000);
        spinTransDist->setMaximum(1000.000000000000000);
        spinTransDist->setSingleStep(0.001000000000000);
        spinTransDist->setValue(0.001000000000000);

        formLayout_2->setWidget(7, QFormLayout::ItemRole::FieldRole, spinTransDist);

        spinTransAngle = new QDoubleSpinBox(widgetSim);
        spinTransAngle->setObjectName("spinTransAngle");
        spinTransAngle->setDecimals(0);
        spinTransAngle->setMinimum(10.000000000000000);
        spinTransAngle->setMaximum(90.000000000000000);
        spinTransAngle->setSingleStep(10.000000000000000);
        spinTransAngle->setValue(10.000000000000000);

        formLayout_2->setWidget(8, QFormLayout::ItemRole::FieldRole, spinTransAngle);


        gridLayout_2->addLayout(formLayout_2, 1, 0, 1, 1);


        verticalLayout_2->addWidget(widgetSim);

        checkParamsG = new QCheckBox(scrollAreaParamsWidget);
        checkParamsG->setObjectName("checkParamsG");

        verticalLayout_2->addWidget(checkParamsG);

        widgetGrazing = new QWidget(scrollAreaParamsWidget);
        widgetGrazing->setObjectName("widgetGrazing");
        widgetGrazing->setMinimumSize(QSize(0, 50));
        formLayout_3 = new QFormLayout(widgetGrazing);
        formLayout_3->setObjectName("formLayout_3");
        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");

        formLayout_3->setLayout(0, QFormLayout::ItemRole::LabelRole, formLayout);

        label_15 = new QLabel(widgetGrazing);
        label_15->setObjectName("label_15");
        label_15->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        formLayout_3->setWidget(1, QFormLayout::ItemRole::LabelRole, label_15);

        spinMeadowRadius = new QDoubleSpinBox(widgetGrazing);
        spinMeadowRadius->setObjectName("spinMeadowRadius");
        spinMeadowRadius->setMaximum(10000.000000000000000);
        spinMeadowRadius->setSingleStep(10.000000000000000);

        formLayout_3->setWidget(1, QFormLayout::ItemRole::FieldRole, spinMeadowRadius);

        label_17 = new QLabel(widgetGrazing);
        label_17->setObjectName("label_17");
        label_17->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        formLayout_3->setWidget(4, QFormLayout::ItemRole::LabelRole, label_17);

        spinLawnRadius = new QDoubleSpinBox(widgetGrazing);
        spinLawnRadius->setObjectName("spinLawnRadius");
        spinLawnRadius->setMinimum(0.200000000000000);
        spinLawnRadius->setMaximum(10.000000000000000);
        spinLawnRadius->setSingleStep(0.200000000000000);

        formLayout_3->setWidget(4, QFormLayout::ItemRole::FieldRole, spinLawnRadius);

        spinAnimalsPerLawn = new QSpinBox(widgetGrazing);
        spinAnimalsPerLawn->setObjectName("spinAnimalsPerLawn");
        spinAnimalsPerLawn->setMinimum(1);
        spinAnimalsPerLawn->setMaximum(10);

        formLayout_3->setWidget(5, QFormLayout::ItemRole::FieldRole, spinAnimalsPerLawn);

        label_18 = new QLabel(widgetGrazing);
        label_18->setObjectName("label_18");
        label_18->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        formLayout_3->setWidget(5, QFormLayout::ItemRole::LabelRole, label_18);

        spinMeadowCapacity = new QDoubleSpinBox(widgetGrazing);
        spinMeadowCapacity->setObjectName("spinMeadowCapacity");
        spinMeadowCapacity->setMaximum(100.000000000000000);

        formLayout_3->setWidget(2, QFormLayout::ItemRole::FieldRole, spinMeadowCapacity);

        label_16 = new QLabel(widgetGrazing);
        label_16->setObjectName("label_16");
        label_16->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        formLayout_3->setWidget(2, QFormLayout::ItemRole::LabelRole, label_16);

        spinAnimalGrazingCapacity = new QDoubleSpinBox(widgetGrazing);
        spinAnimalGrazingCapacity->setObjectName("spinAnimalGrazingCapacity");
        spinAnimalGrazingCapacity->setDecimals(3);
        spinAnimalGrazingCapacity->setMinimum(1.000000000000000);
        spinAnimalGrazingCapacity->setMaximum(1000.000000000000000);
        spinAnimalGrazingCapacity->setSingleStep(1.000000000000000);
        spinAnimalGrazingCapacity->setValue(1.000000000000000);

        formLayout_3->setWidget(3, QFormLayout::ItemRole::FieldRole, spinAnimalGrazingCapacity);

        label_14 = new QLabel(widgetGrazing);
        label_14->setObjectName("label_14");
        label_14->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        formLayout_3->setWidget(3, QFormLayout::ItemRole::LabelRole, label_14);


        verticalLayout_2->addWidget(widgetGrazing);

        verticalScrollSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_2->addItem(verticalScrollSpacer);


        verticalLayout->addWidget(scrollAreaParamsWidget);

        scrollAreaParams->setWidget(scrollAreaWidgetContents_2);

        verticalLayoutParams->addWidget(scrollAreaParams);


        horizontalLayout->addLayout(verticalLayoutParams);


        horizontalLayout_2->addLayout(horizontalLayout);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 431, 17));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "HerdMind Simulator", nullptr));
#if QT_CONFIG(tooltip)
        editTime->setToolTip(QCoreApplication::translate("MainWindow", "Time elapsed since the herd generation", nullptr));
#endif // QT_CONFIG(tooltip)
        editTime->setText(QCoreApplication::translate("MainWindow", "Press ->", nullptr));
        btnGenerate->setText(QCoreApplication::translate("MainWindow", "Generate ", nullptr));
        checkShepard->setText(QCoreApplication::translate("MainWindow", "Shepard", nullptr));
        checkRecursiveCollision->setText(QCoreApplication::translate("MainWindow", "Rec Coll", nullptr));
        groupBox->setTitle(QCoreApplication::translate("MainWindow", "Center Long : Lat", nullptr));
#if QT_CONFIG(tooltip)
        spinCenterLong->setToolTip(QCoreApplication::translate("MainWindow", "Center Longitude", nullptr));
#endif // QT_CONFIG(tooltip)
        label_19->setText(QCoreApplication::translate("MainWindow", " : ", nullptr));
#if QT_CONFIG(tooltip)
        spinCenterLat->setToolTip(QCoreApplication::translate("MainWindow", "Center Latitude", nullptr));
#endif // QT_CONFIG(tooltip)
        label_3->setText(QCoreApplication::translate("MainWindow", "Size:", nullptr));
#if QT_CONFIG(tooltip)
        doubleSpinAnimalSize->setToolTip(QCoreApplication::translate("MainWindow", "Animal size in meters", nullptr));
#endif // QT_CONFIG(tooltip)
        label->setText(QCoreApplication::translate("MainWindow", "Count:", nullptr));
#if QT_CONFIG(tooltip)
        spinAnimalsCount->setToolTip(QCoreApplication::translate("MainWindow", "Animals count", nullptr));
#endif // QT_CONFIG(tooltip)
        label_4->setText(QCoreApplication::translate("MainWindow", "Area:", nullptr));
#if QT_CONFIG(tooltip)
        doubleSpinArea->setToolTip(QCoreApplication::translate("MainWindow", "Area size in meters", nullptr));
#endif // QT_CONFIG(tooltip)
        label_2->setText(QCoreApplication::translate("MainWindow", "Collars %", nullptr));
#if QT_CONFIG(tooltip)
        spinCollarsPercentage->setToolTip(QCoreApplication::translate("MainWindow", "Collars percentage (%)", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        checkParamsHerding->setToolTip(QCoreApplication::translate("MainWindow", "Herding simulation parameters widget", nullptr));
#endif // QT_CONFIG(tooltip)
        checkParamsHerding->setText(QCoreApplication::translate("MainWindow", "Parameters herding", nullptr));
        label_5->setText(QCoreApplication::translate("MainWindow", "Attr. power:", nullptr));
        label_6->setText(QCoreApplication::translate("MainWindow", "Attr. dist:", nullptr));
        label_7->setText(QCoreApplication::translate("MainWindow", "Rep. dist:", nullptr));
        label_8->setText(QCoreApplication::translate("MainWindow", "Coll. dist:", nullptr));
        label_9->setText(QCoreApplication::translate("MainWindow", "Max. speed:", nullptr));
        label_10->setText(QCoreApplication::translate("MainWindow", "Friction:", nullptr));
        label_11->setText(QCoreApplication::translate("MainWindow", "Rot. fad:", nullptr));
        label_12->setText(QCoreApplication::translate("MainWindow", "Trans. dist:", nullptr));
        label_13->setText(QCoreApplication::translate("MainWindow", "Trans. ang:", nullptr));
#if QT_CONFIG(tooltip)
        spinAttrPower->setToolTip(QCoreApplication::translate("MainWindow", "Attraction power", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        spinAttrDist->setToolTip(QCoreApplication::translate("MainWindow", "Attraction distance", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        spinRepDist->setToolTip(QCoreApplication::translate("MainWindow", "Repeling distance", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        spinCollDist->setToolTip(QCoreApplication::translate("MainWindow", "Collision distance", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        spinMaxSpeed->setToolTip(QCoreApplication::translate("MainWindow", "Maximum speed", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        spinFriction->setToolTip(QCoreApplication::translate("MainWindow", "Friction (fading of the speed)", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        spinRotFad->setToolTip(QCoreApplication::translate("MainWindow", "Rotation fading (not used)", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        spinTransDist->setToolTip(QCoreApplication::translate("MainWindow", "Transmit maximum distance", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        spinTransAngle->setToolTip(QCoreApplication::translate("MainWindow", "Transmit angle: bolus angle of view in degrees [0..90]", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        checkParamsG->setToolTip(QCoreApplication::translate("MainWindow", "Grazing and meadow parameters widget", nullptr));
#endif // QT_CONFIG(tooltip)
        checkParamsG->setText(QCoreApplication::translate("MainWindow", "Parameters grazing", nullptr));
        label_15->setText(QCoreApplication::translate("MainWindow", "M. radius", nullptr));
#if QT_CONFIG(tooltip)
        spinMeadowRadius->setToolTip(QCoreApplication::translate("MainWindow", "Meadow (array of lawns) radius in meters", nullptr));
#endif // QT_CONFIG(tooltip)
        label_17->setText(QCoreApplication::translate("MainWindow", "L. radius", nullptr));
#if QT_CONFIG(tooltip)
        spinLawnRadius->setToolTip(QCoreApplication::translate("MainWindow", "Lawn radius in meters. Lawn is part of the meadow.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        spinAnimalsPerLawn->setToolTip(QCoreApplication::translate("MainWindow", "Animals count", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        label_18->setToolTip(QCoreApplication::translate("MainWindow", "Animals count per lawn", nullptr));
#endif // QT_CONFIG(tooltip)
        label_18->setText(QCoreApplication::translate("MainWindow", "A. per lawn", nullptr));
#if QT_CONFIG(tooltip)
        spinMeadowCapacity->setToolTip(QCoreApplication::translate("MainWindow", "Meadow capacity in kg per square meter", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        label_16->setToolTip(QCoreApplication::translate("MainWindow", "Meadow grass capacity in kg per square meter", nullptr));
#endif // QT_CONFIG(tooltip)
        label_16->setText(QCoreApplication::translate("MainWindow", "M. cap", nullptr));
#if QT_CONFIG(tooltip)
        spinAnimalGrazingCapacity->setToolTip(QCoreApplication::translate("MainWindow", "Animal grazing capacity in kg per day", nullptr));
#endif // QT_CONFIG(tooltip)
        label_14->setText(QCoreApplication::translate("MainWindow", "A. cap", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
