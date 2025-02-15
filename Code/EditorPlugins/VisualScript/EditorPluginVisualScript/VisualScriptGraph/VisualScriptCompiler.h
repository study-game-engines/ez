#pragma once

#include <EditorPluginVisualScript/VisualScriptGraph/VisualScriptGraph.h>

class ezVisualScriptCompiler
{
public:
  ezVisualScriptCompiler();
  ~ezVisualScriptCompiler();

  void InitModule(ezStringView sBaseClassName, ezStringView sScriptClassName);

  ezResult AddFunction(ezStringView sName, const ezDocumentObject* pEntryObject, const ezDocumentObject* pParentObject = nullptr);

  ezResult Compile(ezStringView sDebugAstOutputPath = ezStringView());

  struct CompiledFunction
  {
    ezString m_sName;
    ezEnum<ezVisualScriptNodeDescription::Type> m_Type;
    ezEnum<ezScriptCoroutineCreationMode> m_CoroutineCreationMode;
    ezDynamicArray<ezVisualScriptNodeDescription> m_NodeDescriptions;
    ezVisualScriptDataDescription m_LocalDataDesc;
  };

  struct CompiledModule
  {
    CompiledModule();

    ezResult Serialize(ezStreamWriter& inout_stream) const;

    ezString m_sBaseClassName;
    ezString m_sScriptClassName;
    ezHybridArray<CompiledFunction, 16> m_Functions;

    ezVisualScriptDataDescription m_InstanceDataDesc;
    ezVisualScriptDataDescription m_ConstantDataDesc;
    ezVisualScriptDataStorage m_ConstantDataStorage;
  };

  const CompiledModule& GetCompiledModule() const { return m_Module; }

private:
  struct AstNode;

  struct DataInput
  {
    EZ_DECLARE_POD_TYPE();

    AstNode* m_pSourceNode = nullptr;
    ezUInt32 m_uiId = 0;
    ezUInt8 m_uiSourcePinIndex = 0;
    ezUInt8 m_uiTargetPinIndex = 0;
    ezEnum<ezVisualScriptDataType> m_DataType;
    ezUInt8 m_uiArrayIndex = 0;
  };

  struct DataOutput
  {
    ezSmallArray<AstNode*, 3> m_TargetNodes;
    ezUInt32 m_uiId = 0;
    ezUInt8 m_uiSourcePinIndex = 0;
    ezEnum<ezVisualScriptDataType> m_DataType;
  };

  struct AstNode
  {
    ezEnum<ezVisualScriptNodeDescription::Type> m_Type;
    ezEnum<ezVisualScriptDataType> m_DeductedDataType;
    bool m_bImplicitExecution = false;
    const ezDocumentObject* m_pObject = nullptr;
    ezSmallArray<AstNode*, 8> m_Next;
    ezSmallArray<DataInput, 4> m_Inputs;
    ezSmallArray<DataOutput, 4> m_Outputs;
  };

  EZ_ALWAYS_INLINE static ezStringView GetNiceTypeName(const ezDocumentObject* pObject)
  {
    return ezVisualScriptNodeManager::GetNiceTypeName(pObject);
  }

  EZ_ALWAYS_INLINE ezVisualScriptDataType::Enum GetDeductedType(const ezDocumentObject* pObject) const
  {
    return m_pManager->GetDeductedType(pObject);
  }

  ezUInt32 GetPinId(const ezVisualScriptPin* pPin);
  DataOutput& GetDataOutput(const DataInput& dataInput);

  AstNode* BuildAST(const ezDocumentObject* pEntryNode, const ezDocumentObject* pParentNode);
  void MarkAsCoroutine(AstNode* pEntryAstNode);
  ezResult InsertMakeArrayForDynamicPin(AstNode* pNode, const ezVisualScriptNodeRegistry::PinDesc& pinDesc, ezPin::Type pinType);
  ezResult InsertTypeConversions(AstNode* pEntryAstNode);
  ezResult BuildDataStack(AstNode* pEntryAstNode, ezDynamicArray<AstNode*>& out_Stack);
  ezResult BuildDataExecutions(AstNode* pEntryAstNode);
  ezResult FillDataOutputConnections(AstNode* pEntryAstNode);
  ezResult CollectData(AstNode* pEntryAstNode, ezVisualScriptDataDescription& inout_localDataDesc);
  ezResult BuildNodeDescriptions(AstNode* pEntryAstNode, ezDynamicArray<ezVisualScriptNodeDescription>& out_NodeDescriptions);

  struct ConnectionType
  {
    enum Enum
    {
      Execution = EZ_BIT(0),
      Data = EZ_BIT(1),

      All = Execution | Data,
    };
  };

  struct Connection
  {
    AstNode* m_pPrev = nullptr;
    AstNode* m_pCurrent = nullptr;
    ConnectionType::Enum m_Type = ConnectionType::Execution;
    ezUInt32 m_uiPrevPinIndex = 0;
  };

  struct ConnectionHasher
  {
    static ezUInt32 Hash(const Connection& c);
    static bool Equal(const Connection& a, const Connection& b);
  };

  enum class VisitorResult
  {
    Continue,
    Stop,
    Error,
  };

  using AstNodeVisitorFunc = ezDelegate<VisitorResult(const Connection& connection)>;
  ezResult TraverseAst(AstNode* pEntryAstNode, ezUInt32 uiConnectionTypes, AstNodeVisitorFunc func);

  ezResult FinalizeDataOffsets();
  ezResult FinalizeConstantData();

  void DumpAST(AstNode* pEntryAstNode, ezStringView sOutputPath, ezStringView sFunctionName, ezStringView sSuffix);
  void DumpGraph(ezArrayPtr<const ezVisualScriptNodeDescription> nodeDescriptions, ezStringView sOutputPath, ezStringView sFunctionName, ezStringView sSuffix);

  const ezVisualScriptNodeManager* m_pManager = nullptr;

  ezDeque<AstNode> m_AstNodes;
  ezHashTable<const ezDocumentObject*, AstNode*> m_ObjectToAstNode;
  ezHybridArray<AstNode*, 8> m_EntryAstNodes;

  ezHashSet<Connection, ConnectionHasher> m_ReportedConnections;

  ezHashTable<const ezVisualScriptPin*, ezUInt32> m_PinToId;
  ezUInt32 m_uiNextPinId = 0;

  using DataOffset = ezVisualScriptDataDescription::DataOffset;

  struct DataDesc
  {
    EZ_DECLARE_POD_TYPE();

    DataOffset m_DataOffset;
    ezUInt32 m_uiUsageCounter = 0;
  };

  ezHashTable<ezUInt32, DataDesc> m_PinIdToDataDesc;

  ezHashTable<ezVariant, ezUInt32> m_ConstantDataToIndex;

  CompiledModule m_Module;
};
