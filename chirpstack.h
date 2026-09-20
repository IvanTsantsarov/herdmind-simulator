#ifndef CHIRPSTACK_H
#define CHIRPSTACK_H

////////////////////////////////////////////////////////////////////////////////
/// AI generated class
////////////////////////////////////////////////////////////////////////////////

#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>

class ChirpStackTokenValidator {
public:
    struct ValidationResult {
        bool isValid;
        QString message;
        QJsonObject payload;
    };

    static ValidationResult validateOffline(const QString &apiKey) {
        ValidationResult result = {false, "", QJsonObject()};

        // 1. Basic Structure Check (Must have exactly 3 parts separated by '.')
        QStringList parts = apiKey.split('.');
        if (parts.size() != 3) {
            result.message = "Invalid Format: Token must consist of 3 dot-separated parts.";
            return result;
        }

        // 2. Prefix Check (JWT header usually encodes to start with "eyJ")
        if (!apiKey.startsWith("eyJ")) {
            result.message = "Invalid Header: Does not start with standard JWT prefix 'eyJ'.";
            return result;
        }

        // 3. Decode the Payload (The second part)
        // Convert from Base64URL encoding to standard Base64 for Qt
        QString payloadB64 = parts[1];
        payloadB64.replace('-', '+').replace('_', '/');

        // Pad the Base64 string if necessary
        int remainder = payloadB64.length() % 4;
        if (remainder > 0) {
            payloadB64.append(QString(4 - remainder, '='));
        }

        QByteArray decodedBytes = QByteArray::fromBase64(payloadB64.toUtf8());

        // 4. Parse the JSON Payload
        QJsonParseError jsonError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(decodedBytes, &jsonError);
        if (jsonError.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
            result.message = "Malformed Data: Failed to parse payload JSON.";
            return result;
        }

        QJsonObject payloadObj = jsonDoc.object();
        result.payload = payloadObj;

        // 5. Check Expiration Claim ('exp')
        if (payloadObj.contains("exp")) {
            qint64 expTimestamp = payloadObj["exp"].toVariant().toLongLong();
            qint64 currentTimestamp = QDateTime::currentSecsSinceEpoch();

            if (currentTimestamp > expTimestamp) {
                QDateTime expTime = QDateTime::fromSecsSinceEpoch(expTimestamp);
                result.message = QString("Expired: Token expired on %1").arg(expTime.toString(Qt::ISODate));
                return result;
            }
        }

        // Everything looks structurally correct
        result.isValid = true;
        result.message = "Structurally Valid.";
        return result;
    }
};


#endif // CHIRPSTACK_H
