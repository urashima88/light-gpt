#pragma once

#include "componentmeta.h"
#include <QObject>
#include <QProcess>
#include <QQueue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QColor>

enum CodeCommand {
    Inspect,
    Analyze,
    ApplyChanges
};

struct CodeRequest {
    CodeCommand command;
    QString filePath;
    QString virtualPath;
    QString iconPath;
    QColor color;
    QString className;
    QString typeId;
    QString containerClassName;
    QJsonObject changes;
};

class ComponentCodeManager: public QObject {
    Q_OBJECT

public:
    explicit ComponentCodeManager(const QString& pythonExe,
                                  const QString& editorScript,
                                  QObject* parent = nullptr);

    ~ComponentCodeManager() override;

    void inspectComponent(const QString& filePath,
                          const QString& virtualPath,
                          const QString& iconPath,
                          const QColor& color,
                          const QString& className,
                          const QString& typeId = QString());

    void requestAnalysis(const QString& filePath,
                         const QString& containerClassName);

    void applyChanges(const QString& filePath,
                      const QString& containerClassName,
                      const QJsonObject& changes);
signals:
    void metaReady(const QString& typeId, const ComponentMeta& meta);
    void analysisReady(const QString& containerClassName, const QJsonObject& analysis);
    void changesApplied(const QString& containerClassName, const QJsonObject& newAnalysis);
    void errorOccurred(const QString& typeIdOrContainer, const QString& errorMessage);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);

private:
    void startNext();
    void processInspectResult(const QJsonObject& json);
    void processAnalyzeResult(const  QJsonObject& json);

    QProcess* m_process = nullptr;
    QQueue<CodeRequest> m_queue;
    CodeRequest m_currentRequest;
    bool m_busy = false;

    QString m_pythonExe;
    QString m_editorScript;
};
