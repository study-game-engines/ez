#pragma once

#include <EditorFramework/Assets/SimpleAssetDocument.h>

class ezSoundBankAssetProperties : public ezReflectedClass
{
  EZ_ADD_DYNAMIC_REFLECTION(ezSoundBankAssetProperties, ezReflectedClass);

public:
  ezSoundBankAssetProperties() = default;

  ezString m_sSoundBank;
};

class ezSoundBankAssetDocument : public ezSimpleAssetDocument<ezSoundBankAssetProperties>
{
  EZ_ADD_DYNAMIC_REFLECTION(ezSoundBankAssetDocument, ezSimpleAssetDocument<ezSoundBankAssetProperties>);

public:
  ezSoundBankAssetDocument(const char* szDocumentPath);

protected:
  virtual void UpdateAssetDocumentInfo(ezAssetDocumentInfo* pInfo) const override;
  virtual ezTransformStatus InternalTransformAsset(ezStreamWriter& stream, const char* szOutputTag, const ezPlatformProfile* pAssetProfile,
    const ezAssetFileHeader& AssetHeader, ezBitflags<ezTransformFlags> transformFlags) override;
};
