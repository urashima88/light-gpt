#include "componentinspector.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>

ComponentInspector::ComponentInspector(const QString& pythonExe,
                                       const QString& inspectorScript,
                                       QObject* parent)
    : m_pythonExe(pythonExe)
    , m_inspectorScript(inspectorScript)
    , QObject(parent)
{

}

ComponentInspector::~ComponentInspector()
{
    if (m_process) {
        m_process->kill();
        m_process->deleteLater();
    }
}

void ComponentInspector::inspect(
    const QString& modulePath,
    const QString& virtualPath,
    const QString& iconPath,
    const QString& className,
    const QString& tag
)
{
    m_queue.enqueue({modulePath, virtualPath, iconPath, className, tag});
    if (!m_busy) {
        startNext();
    }
}

void ComponentInspector::startNext()
{
    if (m_queue.isEmpty()) {
        m_busy = false;
        return;
    }
    m_busy = true;
    Request req = m_queue.dequeue();
    m_currentTag = req.tag;

    if (!m_process) {
        m_process = new QProcess(this);
        connect(
            m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ComponentInspector::onProcessFinished);
        connect(m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
            if (error == QProcess::FailedToStart) {
                emit errorOccured(m_currentTag, tr("Failed to start Python process"));
                m_busy = false;
                startNext();
            }
        });
    }

    QStringList args;
    args << m_inspectorScript << req.modulePath << req.className;
    m_process->start(m_pythonExe, args);
}

void ComponentInspector::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QByteArray stdOut = m_process->readAllStandardOutput();
    QByteArray stdErr = m_process->readAllStandardError();

    if (exitStatus == QProcess::CrashExit || exitCode != 0) {
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(stdOut, &parseError);
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains(QStringLiteral("error"))) {
                QString errorMsg = obj[QStringLiteral("error")].toString();
                emit errorOccured(m_currentTag, errorMsg);
            } else {
                emit errorOccured(m_currentTag, QString::fromUtf8(stdOut));
            }
        } else {
            QString errMsg = QString::fromUtf8(stdErr);
            if (errMsg.isEmpty())
                errMsg = tr("Inspector process exited with code %1").arg(exitCode);
            emit errorOccured(m_currentTag, errMsg);
        }
    } else {
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(stdOut, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            emit errorOccured(m_currentTag, tr("JSON parse error: %1").arg(parseError.errorString()));
        } else {
            QJsonObject obj = doc.object();
            if (obj.contains(QStringLiteral("error"))) {
                emit errorOccured(m_currentTag, obj[QStringLiteral("error")].toString());
            } else {
                ComponentMeta meta = ComponentMeta::fromJson(obj);
                emit metaReady(m_currentTag, meta);
            }
        }
    }
    m_busy = false;
    startNext();
}
