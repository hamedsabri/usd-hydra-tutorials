#pragma once

#include <QObject>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/usd/stage.h>

namespace HVW_NS
{

/**
 * @brief Manages the lifetime and access to a single USD stage.
 */
class UsdDocument : public QObject
{
    Q_OBJECT
public:
    UsdDocument(QObject* parent = nullptr);

    virtual ~UsdDocument() = default;
    PXR_NS::UsdStageRefPtr createNewStageInMemory();
    PXR_NS::UsdStageRefPtr openStage(const QString& path);

    void setEditTargetLayer(PXR_NS::SdfLayerHandle layer);
    PXR_NS::UsdStageRefPtr getCurrentStage() const;
    PXR_NS::SdfLayerRefPtr getRootLayer() const;

Q_SIGNALS:
    void stageOpened(const QString& filePath);

private:
    PXR_NS::UsdStageRefPtr m_stage;
};

} // namespace HVW_NS
