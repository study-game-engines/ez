#pragma once

#include <Core/Configuration/PlatformProfile.h>
#include <GameEngine/GameEngineDLL.h>

class EZ_GAMEENGINE_DLL ezRenderPipelineProfileConfig : public ezProfileConfigData
{
  EZ_ADD_DYNAMIC_REFLECTION(ezRenderPipelineProfileConfig, ezProfileConfigData);

public:
  virtual void SaveRuntimeData(ezChunkStreamWriter& inout_stream) const override;
  virtual void LoadRuntimeData(ezChunkStreamReader& inout_stream) override;

  ezString m_sMainRenderPipeline;
  // ezString m_sEditorRenderPipeline;
  // ezString m_sDebugRenderPipeline;

  ezMap<ezString, ezString> m_CameraPipelines;
};
