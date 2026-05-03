#include "componentcodemanager.h"
#include <QCoreApplication>
#include <QDebug>

ComponentCodeManager::ComponentCodeManager(const QString& pythonExe,
                                           const QString& editorScript,
                                           QObject* parent)
    : QObject(parent)
    , m_pythonExe(pythonExe)
    , m_editorScript(editorScript)
{

}

ComponentCodeManager::~ComponentCodeManager()
{
    if (m_process) {
        m_process->kill();
        m_process->deleteLater();
    }
}

void ComponentCodeManager::inspectComponent(const QString& filePath,
                                            const QString& virtualPath,
                                            const QString& iconPath,
                                            const QColor& color,
                                            const QString& className,
                                            const QString& typeId)
{
    CodeRequest req;
    req.command = CodeCommand::Inspect;
    req.filePath = filePath;
    req.virtualPath = virtualPath;
    req.iconPath = iconPath;
    req.color = color;
    req.className = className;
    req.typeId = typeId;
    m_queue.enqueue(req);
    if (!m_busy)
        startNext();
}

void ComponentCodeManager::requestAnalysis(const QString& filePath,
                                           const QString& containerClassName)
{
    CodeRequest req;
    req.command = CodeCommand::Analyze;
    req.filePath = filePath;
    req.containerClassName = containerClassName;
    m_queue.enqueue(req);
    if (!m_busy)
        startNext();
}

void ComponentCodeManager::applyChanges(const QString& filePath,
                                        const QString& containerClassName,
                                        const QJsonObject& changes)
{
    CodeRequest req;
    req.command = CodeCommand::ApplyChanges;
    req.filePath = filePath;
    req.containerClassName = containerClassName;
    req.changes = changes;
    m_queue.enqueue(req);
    if (!m_busy)
        startNext();
}

void ComponentCodeManager::startNext()
{
    if (m_queue.isEmpty()) {
        m_busy = false;
        return;
    }

    m_busy = true;
    m_currentRequest = m_queue.dequeue();

    if (!m_process) {
        m_process = new QProcess(this);
        connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &ComponentCodeManager::onProcessFinished);
        connect(m_process, &QProcess::errorOccurred,
                this, &ComponentCodeManager::onProcessError);
    }

    QStringList args;
    args << m_editorScript;

    switch (m_currentRequest.command) {
    case CodeCommand::Inspect:
        args << "inspect" << m_currentRequest.filePath << m_currentRequest.className;
        break;
    case CodeCommand::Analyze:
        args << "analyze" << m_currentRequest.containerClassName << m_currentRequest.filePath;
        break;
    case CodeCommand::ApplyChanges: {
        args << "apply_changes" << m_currentRequest.containerClassName << m_currentRequest.filePath;
        QByteArray changesJson = QJsonDocument(m_currentRequest.changes).toJson(QJsonDocument::Compact);
        args << QString::fromUtf8(changesJson);
        break;
    }
    }
    m_process->start(m_pythonExe, args);
}

void ComponentCodeManager::onProcessError(QProcess::ProcessError error)
{
    if (error == QProcess::FailedToStart) {
        QString id = m_currentRequest.command == CodeCommand::Inspect ?
                         m_currentRequest.typeId : m_currentRequest.containerClassName;
        emit errorOccurred(id, tr("Failed to start Python process"));
        m_busy = false;
        startNext();
    }
}

void ComponentCodeManager::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QByteArray stdOut = m_process->readAllStandardOutput();
    QByteArray stdErr = m_process->readAllStandardError();

    if (exitStatus == QProcess::CrashExit || exitCode != 0) {
        QString errorMsg;
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(stdOut, &parseError);
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains(QStringLiteral("error")))
                errorMsg = obj[QStringLiteral("error")].toString();
        }
        if (errorMsg.isEmpty())
            errorMsg = QString::fromUtf8(stdErr);
        if (errorMsg.isEmpty())
            errorMsg = tr("Process crashed with exit code %1").arg(exitCode);

        QString id = m_currentRequest.command == CodeCommand::Inspect ?
                         m_currentRequest.typeId : m_currentRequest.containerClassName;

        qDebug() << "ComponentCodeManager::onProcessedFinished - Error - " << errorMsg << '\n';

        emit errorOccurred(id, errorMsg);
        m_busy = false;
        startNext();
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(stdOut, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        QString id = m_currentRequest.command == CodeCommand::Inspect ?
                         m_currentRequest.typeId : m_currentRequest.containerClassName;

        qDebug() << "ComponentCodeManager::onProcessedFinished - Error - " << parseError.errorString() << "at offset" << parseError.offset << '\n';

        emit errorOccurred(id, tr("JSON parse error: %1").arg(parseError.errorString()));
        m_busy = false;
        startNext();
        return;
    }

    QJsonObject json = doc.object();
    if (json.contains(QStringLiteral("error"))) {
        QString id = m_currentRequest.command == CodeCommand::Inspect ?
                         m_currentRequest.typeId : m_currentRequest.containerClassName;

        qDebug() << "ComponentCodeManager::onProcessedFinished - Error - " << json[QStringLiteral("error")].toString() << '\n';

        emit errorOccurred(id, json[QStringLiteral("error")].toString());
        m_busy = false;
        startNext();
        return;
    }

    switch (m_currentRequest.command) {
    case CodeCommand::Inspect:
        processInspectResult(json);
        break;
    case CodeCommand::Analyze:
        processAnalyzeResult(json);
        break;
    case CodeCommand::ApplyChanges:
        processAnalyzeResult(json);
        emit changesApplied(m_currentRequest.containerClassName, json);
        break;
    }

    m_busy = false;
    startNext();
}

void ComponentCodeManager::processInspectResult(const QJsonObject& json)
{
    ComponentMeta meta = ComponentMeta::fromJson(json);
    emit metaReady(m_currentRequest.typeId, meta);
}

void ComponentCodeManager::processAnalyzeResult(const QJsonObject& json)
{
    emit analysisReady(m_currentRequest.containerClassName, json);
}