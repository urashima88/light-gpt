#include "componentmeta.h"
#include <QJsonObject>
#include <QJsonArray>

ComponentMeta ComponentMeta::fromJson(const QJsonObject& json) {
    ComponentMeta meta;
    meta.className = json["class_name"].toString();
    meta.modulePath = json["module_path"].toString();

    for (const auto& val : json["constructor_params"].toArray()) {
        QJsonObject obj = val.toObject();
        ParamInfo p;
        p.name = obj["name"].toString();
        p.type = obj["type"].toString();
        p.defaultValue = obj["default"].toString();
        p.hasDefault = obj["has_default"].toBool();
        meta.constructorParams.append(p);
    }

    for (const auto& val : json["inputs"].toArray()) {
        QJsonObject obj = val.toObject();
        IOInfo io;
        io.name = obj["name"].toString();
        io.type = obj["type"].toString();
        meta.inputs.append(io);
    }

    for (const auto& val : json["outputs"].toArray()) {
        QJsonObject obj = val.toObject();
        IOInfo io;
        io.name = obj["name"].toString();
        io.type = obj["type"].toString();
        meta.outputs.append(io);
    }

    return meta;
}