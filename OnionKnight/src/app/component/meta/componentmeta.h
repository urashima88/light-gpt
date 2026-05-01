#pragma once

#include <QString>
#include <QVector>
#include <QJsonObject>

struct ParamInfo {
    QString name;
    QString type;
    QString defaultValue;
    bool hasDefault = false;
};

struct IOInfo {
    QString name;
    QString type;
};

class ComponentMeta {
public:
    QString className;
    QString modulePath;
    QVector<ParamInfo> constructorParams;
    QVector<IOInfo> inputs;
    QVector<IOInfo> outputs;

    bool isValid() const { return !className.isEmpty(); }

    static ComponentMeta fromJson(const QJsonObject& json);
    QJsonObject toJson() const;
};
