#include "usdDocument.h"

#include <QDebug>
#include <QMessageBox>

namespace HVW_NS
{

UsdDocument::UsdDocument(QObject* parent)
    : QObject(parent)
{
    qDebug() << "[UsdDocument] Created.";
}

PXR_NS::UsdStageRefPtr UsdDocument::createNewStageInMemory()
{
    qDebug() << "[UsdDocument] Creating new in-memory stage...";

    m_stage = PXR_NS::UsdStage::CreateInMemory();

    if (m_stage) {
        qDebug() << "[UsdDocument] New stage created successfully.";
        emit stageOpened("untitled");
    }
    else {
        qCritical() << "[UsdDocument] Failed to create in-memory stage.";
        return nullptr;
    }

    return m_stage;
}

PXR_NS::UsdStageRefPtr UsdDocument::openStage(const QString& path)
{
    qDebug() << "[UsdDocument] Opening stage from file:" << path;

    m_stage = PXR_NS::UsdStage::Open(path.toStdString(), PXR_NS::UsdStage::LoadAll);

    if (m_stage) {
        qDebug() << "[UsdDocument] Stage opened successfully.";
        emit stageOpened(path);
    }
    else {
        qCritical() << "[UsdDocument] Failed to open stage:" << path;
        return nullptr;
    }

    return m_stage;
}

PXR_NS::UsdStageRefPtr UsdDocument::getCurrentStage() const
{
    return m_stage;
}

PXR_NS::SdfLayerRefPtr UsdDocument::getRootLayer() const
{
    return m_stage->GetRootLayer();
}

void UsdDocument::setEditTargetLayer(PXR_NS::SdfLayerHandle layer)
{
    qDebug() << "[UsdDocument] Setting new edit target layer:"
             << QString::fromStdString(layer->GetIdentifier());

    m_stage->SetEditTarget(PXR_NS::UsdEditTarget(layer));
}

} // namespace HVW_NS