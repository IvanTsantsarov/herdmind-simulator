#include <QMessageBox>
#include <QToolTip>
#include "mainwindow.h"
#include "dialogsettings.h"
#include "chirpstack.h"
#include "ui_dialogsettings.h"

#define ERROR_COLOR QColor(255, 200, 200)

const QRegularExpression DialogSettings::mRegexProfileId =
    QRegularExpression(R"(^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$)");

const QRegularExpression DialogSettings::mRegexId =
    QRegularExpression(R"(^[0-9a-fA-F]{16})");

const QRegularExpression DialogSettings::mRegexKey =
    QRegularExpression(R"(^eyJ[A-Za-z0-9_-]+\.[A-Za-z0-9_-]+\.[A-Za-z0-9_-]+$)");



void DialogSettings::loadSettings()
{
    ui->editAppId->setText(mSettings.value("Chirpstack/appId").toString());
    ui->editTenantId->setText(mSettings.value("Chirpstack/tenantId").toString());
    // ui->editBolusProfileId->setText(mSettings.value("Chirpstack/bolusProfileId").toString());
    ui->editCollarProfileId->setText(mSettings.value("Chirpstack/collarProfileId").toString());
    ui->editGatewayId->setText(mSettings.value("Chirpstack/gatewayId").toString());
    ui->editApiKey->setText(mSettings.value("Chirpstack/apiKey").toString());
    ui->editTenantKey->setText(mSettings.value("Chirpstack/tenantKey").toString());
}

void DialogSettings::saveSettings()
{
    mSettings.setValue("Chirpstack/appId", ui->editAppId->text());
    mSettings.setValue("Chirpstack/tenantId", ui->editTenantId->text());
    // ui->editBolusProfileId->setText(mSettings.value("Chirpstack/bolusProfileId").toString());
    mSettings.setValue("Chirpstack/collarProfileId", ui->editCollarProfileId->text());
    mSettings.setValue("Chirpstack/gatewayId", ui->editGatewayId->text());
    mSettings.setValue("Chirpstack/apiKey", ui->editApiKey->text());
    mSettings.setValue("Chirpstack/tenantKey", ui->editTenantKey->text());

    mSettings.sync();

    if( mSettings.status() != QSettings::NoError ) {
        qCritical() << "DialogSettings: Error saving Settings" << mSettings.fileName()
            << (mSettings.isWritable() ? "" : "File is READONLY.");
    }

    QToolTip::showText( QCursor::pos(), "Settings saved!");
}

QColor DialogSettings::setColor(QWidget* w, const QColor &newColor)
{
    QPalette p = w->palette();
    QColor oldColor = p.color(QPalette::Base);
    p.setColor(QPalette::Base, newColor);
    w->setPalette(p);
    return oldColor;
}

void DialogSettings::error(const QString &str, QWidget* w)
{
    if( mLastErrorWidget ) {
        setColor(mLastErrorWidget, mLastErrorColor);
    }

    mLastErrorWidget = w;
    mLastErrorColor = setColor(w, ERROR_COLOR);

    ui->labelError->setText(str);

    w->setFocus();
}

DialogSettings::DialogSettings(QSettings& s, QWidget *parent)
    : QDialog(parent),
    mSettings(s),
    ui(new Ui::DialogSettings)
{
    ui->setupUi(this);
    loadSettings();
}

DialogSettings::~DialogSettings()
{
    delete ui;
}



bool DialogSettings::checkValues()
{
    QRegularExpressionMatch match;

    match = mRegexProfileId.match(ui->editAppId->text());
    if( !match.hasMatch() ) {
        error( "Wrong App ID format", ui->editAppId );
        return false;
    }

    match = mRegexProfileId.match(ui->editTenantId->text());
    if( !match.hasMatch() ) {
        error( "Wrong Tenant Id", ui->editTenantId );
        return false;
    }

    match = mRegexProfileId.match(ui->editCollarProfileId->text());
    if( !match.hasMatch() ) {
        error( "Wrong Collar Profile format", ui->editCollarProfileId );
        return false;
    }

    match = mRegexId.match(ui->editGatewayId->text());
    if( !match.hasMatch() ) {
        error( "Wrong Gateway ID format", ui->editGatewayId );
        return false;
    }

    match = mRegexKey.match(ui->editApiKey->text());
    if( !match.hasMatch() ) {
        error( "Wrong API Key format", ui->editApiKey );
        return false;
    }

    match = mRegexKey.match(ui->editTenantKey->text());
    if( !match.hasMatch() ) {
        error( "Wrong Tenant Key format", ui->editTenantKey );
        return false;
    }

    ChirpStackTokenValidator::ValidationResult validatorApiKey = ChirpStackTokenValidator::validateOffline(ui->editApiKey->text());
    if (!validatorApiKey.isValid) {
        error( "Wrong API Key format", ui->editApiKey );
        return false;
    }

    ChirpStackTokenValidator::ValidationResult validatorTenantKey = ChirpStackTokenValidator::validateOffline(ui->editTenantKey->text());
    if (!validatorTenantKey.isValid) {
        error( "Wrong API Key format", ui->editTenantKey );
        return false;
    }

    return true;
}


void DialogSettings::on_btnCancel_clicked()
{
    close();
}


void DialogSettings::on_btnOk_clicked()
{
    if( !checkValues() ) {
        return;
    }

    saveSettings();

    gMainWindow->onDlgSettingsChanged();
    close();
}

