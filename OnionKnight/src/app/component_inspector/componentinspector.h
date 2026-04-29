#pragma once

#include "componentmeta.h"
#include <QObject>
#include <QProcess>
#include <QQueue>

class ComponentInspector: public QObject {
    Q_OBJECT

public:
    explicit ComponentInspector(const QString& pythonExe,
                                const QString& inspectorScript,
                                QObject* parent = nullptr);
    ~ComponentInspector();

    void inspect(const QString& modulePath, const QString& className, const QString& tag = QString());

signals:
    void metaReady(const QString& tag, const ComponentMeta& meta);
    void errorOccured(const QString& tag, const QString& errorMessage);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    struct Request {
        QString modulePath;
        QString className;
        QString tag;
    };

    void startNext();

    QProcess* m_process = nullptr;
    QQueue<Request> m_queue;
    bool m_busy = false;
    QString m_currentTag;

    QString m_pythonExe;
    QString m_inspectorScript;
};
