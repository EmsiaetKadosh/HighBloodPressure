//
// Created by EmsiaetKadosh on 25-4-5.
//

#pragma once

/*
 * TODO(EmsiaetKadosh.DirectX): 关于VRS
 *
 * RTV: RenderTargetView
 * DSV: DepthStencilView
 */


/**
 * @brief 资源类。
 * @brief Resource是D3D12中所有GPU可访问数据的基类，包括：
 * - 缓冲区（Buffers）：如顶点缓冲区、索引缓冲区、常量缓冲区。
 * - 纹理（Textures）：1D/2D/3D纹理、贴图、深度/模板缓冲区。
 * - 其他类型：如UAV（无序访问视图）资源、结构化缓冲区等。
 * @note 资源需要通过描述符绑定到管线（如SRV、UAV、RTV等）。
 * - 资源可以存在于不同的内存堆（Heap）中（如默认堆、上传堆、回读堆）。
 * - 资源状态（Resource States）需要显式管理（例如从D3D12_RESOURCE_STATE_COPY_DEST切换到D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE）。
 */
struct Resource {
	ComPtr<ID3D12Resource2> object;

	/**
	 * @brief 获取资源的扩展描述信息
	 * @code D3D12_RESOURCE_DESC1 @endcode
	 * - 提供更详细的属性，如采样反馈、光追加速结构的特定标志。
	 * - 支持新的资源格式和布局。
	 */
	D3D12_RESOURCE_DESC1 STDMETHODCALLTYPE getDesc1() const { return object->GetDesc1(); }

	/**
	 * @brief 官网未提供注释。
	 * @note 继承自
	 * @code D3D12Resource1 @endcode
	 */
	HRESULT STDMETHODCALLTYPE getProtectedResourceSession(const IID& riid, void** ppProtectedSession) const { return object->GetProtectedResourceSession(riid, ppProtectedSession); }

	/**
	 * @brief 获取指向资源中指定子资源的CPU指针，但可能不会向应用程序公开指针值；还会在必要时使CPU缓存失效，以便CPU读取到此地址时反映GPU所做的任何修改。
	 * @param Subresource 子资源的索引
	 * @param pReadRange 要访问的内存范围
	 * @param ppData [out] 返回指针
	 * @note @link {https://learn.microsoft.com/zh-cn/windows/win32/api/d3d12/nf-d3d12-id3d12resource-map} @endlink
	 * - Map和Unmap可由多个线程安全地调用。
	 * - 支持嵌套调用，并且会进行引用计数。
	 *   - 首次调用Map可为资源分配CPU虚拟地址范围。最后一次调用Unmap可取消分配CPU虚拟地址范围。
	 *   - CPU虚拟地址通常返回到应用程序；但是，使用未知布局操作纹理的内容会阻止泄露CPU虚拟地址。（即不返回）
	 *   - 除非Map是永久嵌套的，否则应用程序不能依赖于地址保持一致。
	 * - 有关更多详细信息，请参阅WriteToSubresource。
	 * - Map返回的指针不保证具有普通指针的所有功能，但大多数应用程序不会注意到正常用法的差异。
	 *   - 例如，与WRITE_BACK行为相比，具有WRITE_COMBINE行为的指针具有较弱的CPU内存排序保证。
	 *   - 由于PCIe限制，不保证CPU和GPU均可访问的内存共享CPU具有的相同原子内存保证。使用Fence进行同步。
	 * - Map有两种使用模型类别：简单和高级。简单使用模型可最大程度地提高工具性能，因此建议应用程序坚持使用简单模型，除非应用需要高级模型。细节参考网站。
	 */
	HRESULT STDMETHODCALLTYPE map(const unsigned int Subresource, const D3D12_RANGE* pReadRange, void** ppData) const { return object->Map(Subresource, pReadRange, ppData); }

	/**
	 * @brief 使指向资源中指定子资源的CPU指针失效。
	 * @param Subresource 子资源的索引
	 * @param pWrittenRange 指定要取消映射的内存范围，表示CPU可能已修改的区域，并且坐标是子资源相对的。nullptr指示整个子资源可能已被CPU修改。指定End <= Begin时，意为未写入任何数据。此参数仅由工具使用，不用于实际取消映射操作的正确性。
	 * @note 参考map
	 */
	void STDMETHODCALLTYPE unmap(const unsigned int Subresource, const D3D12_RANGE* pWrittenRange) const { return object->Unmap(Subresource, pWrittenRange); }

	/**
	 * @brief 获取描述。建议使用getDesc1。
	 */
	[[deprecated]] D3D12_RESOURCE_DESC STDMETHODCALLTYPE getDesc() const { return object->GetDesc(); }

	/**
	 * @brief 获取缓冲区资源的GPU虚拟地址
	 * @returns
	 * @code D3D12_GPU_VIRTUAL_ADDRESS = QWORD @endcode
	 * @note 继承自
	 * @code ID3D12Resource @endcode
	 */
	D3D12_GPU_VIRTUAL_ADDRESS STDMETHODCALLTYPE getGPUVirtualAddress() const { return object->GetGPUVirtualAddress(); }

	/**
	 * @brief 使用CPU将数据复制到子资源中，使CPU能够修改具有未定义布局的大多数纹理的内容。
	 * @param DstSubresource 子资源的索引
	 * @param pDstBox [optional] 指向框的指针，该框定义要将资源数据复制到其中的目标子资源部分。如果为nullptr，则数据将写入目标子资源，且没有偏移量。源的维度必须符合目标（D3D12_BOX）。传入空Box不执行任何操作
	 * @param pSrcData 源数据指针
	 * @param SrcRowPitch 从一行源数据到下一行的距离
	 * @param SrcDepthPitch 从源数据一个深度切片到下一个深度切片的距离
	 * @note
	 * - 应首先使用Map映射资源。
	 * - 纹理必须处于D3D12_RESOURCE_STATE_COMMON状态，才能通过WriteToSubresource和ReadFromSubresource进行CPU访问，才能合法；但缓冲区不需要。
	 * - 为提高效率，请确保框中的水平边界和对齐方式（64/[每像素占用字节]）像素。
	 *   - 垂直边界和对齐方式应为2行，但使用每像素1字节格式的情况除外，此时建议4行。
	 *   - 每个调用只处理单个深度切片时效率较高。建议但不强制提供128字节对齐的指针和步幅。
	 * - 写入子mipmap级别时，建议使用比上述更大的宽度和高度。这是因为小mipmap级别实际上可能存储在较大的内存块中，偏移量不透明，这可能会干扰缓存行的对齐。
	 * - WriteToSubresource和ReadFromSubresource为UMA适配器启用近乎零复制优化，但可能会严重损害离散/NUMA适配器的效率，因为纹理数据不能驻留在本地VideoMemory中。
	 *   - 典型的应用程序应坚持离散友好的上传技术，除非它们识别适配器体系结构是UMA。
	 *   - 有关上传的更多详细信息，请参阅CopyTextureRegion。
	 *   - 有关UMA的更多详细信息，请参阅D3D12_FEATURE_DATA_ARCHITECTURE。
	 * - 在UMA系统上，此例程可用于通过称为循环平铺的循环优化来最大程度地降低内存复制的成本。通过将上传内容分解为适合CPU缓存的chuck，CPU与主内存之间的有效带宽更接近于理论上的最大值。
	 */
	HRESULT STDMETHODCALLTYPE writeToSubresource(const unsigned int DstSubresource, const D3D12_BOX* pDstBox, const void* pSrcData, const unsigned int SrcRowPitch, const unsigned int SrcDepthPitch) const { return object->WriteToSubresource(DstSubresource, pDstBox, pSrcData, SrcRowPitch, SrcDepthPitch); }

	/**
	 * @brief 使用CPU从子资源复制数据，使CPU能够读取具有未定义布局的大多数纹理的内容
	 * @param pDstData 指向内存中目标数据的指针
	 * @param DstRowPitch 从一行目标数据到下一行的距离
	 * @param DstDepthPitch 目标数据一个深度切片到下一个深度切片的距离
	 * @param SrcSubresource 子资源的索引
	 * @param pSrcBox 指向框的指针，该框定义要从其复制资源数据的目标子资源部分。如果为nullptr，则从目标子资源读取数据，且没有偏移量。目标的尺寸必须符合目标（D3D12_BOX）。传入空Box不执行任何操作
	 */
	HRESULT STDMETHODCALLTYPE readFromSubresource(void* pDstData, const unsigned int DstRowPitch, const unsigned int DstDepthPitch, const unsigned int SrcSubresource, const D3D12_BOX* pSrcBox) const { return object->ReadFromSubresource(pDstData, DstRowPitch, DstDepthPitch, SrcSubresource, pSrcBox); }

	/**
	 * @brief 检索已放置和已提交资源的资源堆的属性。
	 * @param pHeapProperties [out, optional]
	 * @code D3D12_HEAP_PROPERTIES* @endcode
	 * @param pHeapFlags [out, optional]
	 * @code D3D12_HEAP_FLAGS* @endcode
	 * @note 此方法仅适用于已放置和提交的资源，不适用于保留资源。如果资源创建为保留资源，则返回
	 * @code E_INVALIDARG @endcode
	 * 。页面可以映射到任何、一个或多个堆。
	 */
	HRESULT STDMETHODCALLTYPE getHeapProperties(D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS* pHeapFlags) const { return object->GetHeapProperties(pHeapProperties, pHeapFlags); }
};

struct CommandList {
	ComPtr<ID3D12GraphicsCommandList7> object;

	/**
	 * @brief 将屏障集合添加到图形命令列表录制中。需要DirectX 12 Agility SDK 1.608或更高版本。
	 * @param NumBarrierGroups pBarrierGroups指向的屏障组数
	 * @param pBarrierGroups 指向
	 * @code D3D12_BARRIER_GROUP @endcode
	 * 对象的数组的指针
	 * @note 继承自
	 * @code ID3D12GraphicsCommandList7 @endcode
	 */
	void barrier(const unsigned int NumBarrierGroups, const D3D12_BARRIER_GROUP* pBarrierGroups) const { object->Barrier(NumBarrierGroups, pBarrierGroups); }

	/**
	 * @brief 官网未提供注释。
	 * @note 继承自
	 * @code ID3D12GraphicsCommandList6 @endcode
	 */
	void dispatchMesh(const unsigned int ThreadGroupCountX, const unsigned int ThreadGroupCountY, const unsigned int ThreadGroupCountZ) const { object->DispatchMesh(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ); }

	/**
	 * @brief 设置可变速率着色（VRS）的基本着色速率和组合器。有关详细信息，请参阅可变速率着色（VRS）。
	 * - 不是所有像素都需要相同的着色计算精度。
	 * - 例如，画面中快速移动的区域、边缘模糊的部分或远离视点的物体，可以降低着色频率（如多个像素共享同一着色结果），而视觉焦点区域（如角色面部）仍保持全精度着色。
	 * @param baseShadingRate
	 * @code D3D12_SHADING_RATE @endcode
	 * 枚举中的常量，描述要设置的基本着色速率
	 * @param combiners 数组指针
	 * - 数组中元素的计数必须等于2，也就是
	 * @code D3D12_RS_SET_SHADING_RATE_COMBINER_COUNT@endcode
	 * - 由于第1层VRS不支持基于每个基元和屏幕空间图像的VRS，因此这些值要有意义，adapter需要Tier2VRS支持。
	 * - 请参阅D3D12_FEATURE_DATA_D3D12_OPTIONS6和D3D12_VARIABLE_SHADING_RATE_TIER
	 * - nullptr等效于默认着色组合器，这两者都是
	 * @code D3D12_SHADING_RATE_COMBINER_PASSTHROUGH @endcode
	 * @note 继承自
	 * @code ID3D12GraphicsCommandList5 @endcode
	 * @link {https://learn.microsoft.com/zh-cn/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist5-rssetshadingrate} @endlink
	 */
	void rsSetShadingRate(const D3D12_SHADING_RATE baseShadingRate, const D3D12_SHADING_RATE_COMBINER* combiners) const { object->RSSetShadingRate(baseShadingRate, combiners); }

	/**
	 * @brief 为VRS设置屏幕空间着色率图像。需要Tier2 VRS支持。请参阅D3D12_FEATURE_DATA_D3D12_OPTIONS6和D3D12_VARIABLE_SHADING_RATE_TIER。
	 * @param shadingRateImage 指向ID3D12Resource的可选指针，表示屏幕空间着色率图像。如果为nullptr，则效果与具有着色率图像相同，其中所有值为1x1的着色速率。
	 * @note 继承自
	 * @code ID3D12GraphicsCommandList5 @endcode
	 */
	void rsSetShadingRateImage(ID3D12Resource* shadingRateImage) const { object->RSSetShadingRateImage(shadingRateImage); }

	/**
	 * @brief 绑定一组输出资源，并启动RenderPass。绑定到一个或多个RTV或深度模具视图DSV。
	 * @param NumRenderTargets 目标数量
	 * @param pRenderTargets 描述绑定到的RTV，以及它们的开始和结束访问特征
	 * @param pDepthStencil 描述绑定到的DSV，以及它们的开始和结束访问特征
	 * @param Flags RenderPass的requirements和nature
	 * @note 继承自
	 * @code ID3D12GraphicsCommandList4 @endcode
	 */
	void beginRenderPass(const unsigned int NumRenderTargets, const D3D12_RENDER_PASS_RENDER_TARGET_DESC* pRenderTargets, const D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* pDepthStencil, const D3D12_RENDER_PASS_FLAGS Flags) const { object->BeginRenderPass(NumRenderTargets, pRenderTargets, pDepthStencil, Flags); }

	/**
	 * @brief 在GPU上执行光线跟踪加速结构生成，并可以选择在生成后立即输出生成后的信息。
	 * @param pDesc 加速结构描述符
	 * @param NumPostbuildInfoDescs 下一个参数的数组的大小。如果不需要生成后信息，则设置为0
	 * @param pPostbuildInfoDescs 生成后信息的说明的可选数组，用于生成已生成的加速结构的描述属性
	 * @note 继承自
	 * @code ID3D12GraphicsCommandList4 @endcode
	 */
	void buildRaytracingAccelerationStructure(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC* pDesc, unsigned int NumPostbuildInfoDescs, const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC* pPostbuildInfoDescs) const { object->BuildRaytracingAccelerationStructure(pDesc, NumPostbuildInfoDescs, pPostbuildInfoDescs); }

	/**
	 * @brief 应用指定转换时，将源加速结构复制到目标内存
	 * @param DestAccelerationStructureData 目标内存
	 * - 如果需要指定模式，可以通过事先调用
	 * @code this.EmitRaytracingAccelerationStructurePostbuildInfo @endcode
	 * 来发现所需的大小。
	 * - 无论指定的模式如何，目标起始地址都必须与 256 字节对齐（定义为D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT）。
	 * - 目标内存范围不能与源重叠。否则，结果为未定义。
	 * - 指向的内存必须处于的资源状态取决于Mode参数。有关详细信息，请参阅D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE。
	 * @param SourceAccelerationStructureData 数据的地址
	 * - 数据保持不变且可用。该操作仅复制SourceAccelerationStructureData指向的数据，而不复制源数据可能指向的任何其他数据，例如加速结构。
	 * - 例如，对于顶级加速结构，不会在操作中复制它指向的任何底层加速结构。
	 * - 源内存必须对齐到256字节（定义为D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT），而不考虑指定的模式。
	 * - 指向的内存必须处于的资源状态取决于Mode参数。有关详细信息，请参阅D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE。
	 * @param Mode 要执行的复制操作的类型。有关详细信息，请参阅D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE。
	 * @note 继承自
	 * @code ID3D12GraphicsCommandList4 @endcode
	 * - 由于光线跟踪加速结构可能包含内部指针，并且具有设备依赖的不透明布局，因此复制它们或以其他方式操作它们需要专用API，以便驱动程序可以处理请求的操作。
	 * - 此方法可以从图形（GCL）或计算命令列表（ComputeCL）调用，但不能从捆绑包（Bundle）调用。
	 */
	void copyRaytracingAccelerationStructure(const D3D12_GPU_VIRTUAL_ADDRESS DestAccelerationStructureData, const D3D12_GPU_VIRTUAL_ADDRESS SourceAccelerationStructureData, const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE Mode) const { object->CopyRaytracingAccelerationStructure(DestAccelerationStructureData, SourceAccelerationStructureData, Mode); }

	/**
	 * @brief 启动光线生成着色器的线程。
	 * @param pDesc 光线调度说明符
	 * @note 继承自
	 * @code ID3D12GraphicsCommandList4 @endcode
	 * - 可以从图形或计算命令列表和捆绑包调用此方法。
	 * - 必须在命令列表中设置光线跟踪管道状态。否则，此调用的行为是未定义的。
	 * - 有3个维度传入来设置网格大小：宽度/高度/深度，必须使
	 * @code width * height * depth <= 2^30 @endcode
	 * 。超过此值会产生未定义的行为。如果任何网格维度为0，则不启动任何线程。
	 */
	void dispatchRays(const D3D12_DISPATCH_RAYS_DESC* pDesc) const { object->DispatchRays(pDesc); }

	/**
	 * @brief 为一组加速结构发出生成后属性。这使应用程序能够知道通过ID3D12GraphicsCommandList4::CopyRaytracingAccelerationStructure执行加速结构操作的输出资源要求。
	 * @param pDesc 描述要生成的生成后信息的D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC对象
	 * @param NumSourceAccelerationStructures 指向pSourceAccelerationStructureData指向的加速结构GPU虚拟地址的指针数
	 * - 此数字还影响目标（输出），后者将是NumSourceAccelerationStructures输出结构的连续数组，其中结构的类型取决于pDesc说明中提供的InfoType字段。
	 * @param pSourceAccelerationStructureData 指向大小为NumSourceAccelerationStructures的GPU虚拟地址数组的指针
	 * - 地址必须对齐到256个字节，定义为D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT。
	 * - 指向的内存必须处于D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE状态。
	 * @note 继承自
	 * @code ID3D12GraphicsCommandList4 @endcode
	 * - 此方法可以从图形或计算命令列表调用，但不能从捆绑包调用。
	 */
	void emitRaytracingAccelerationStructurePostbuildInfo(const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC* pDesc, const unsigned int NumSourceAccelerationStructures, const D3D12_GPU_VIRTUAL_ADDRESS* pSourceAccelerationStructureData) const { object->EmitRaytracingAccelerationStructurePostbuildInfo(pDesc, NumSourceAccelerationStructures, pSourceAccelerationStructureData); }

	/**
	 * @brief 标记呈现通道的结束。
	 * @note 继承自
	 * @code ID3D12GraphicsCommandList4 @endcode
	 */
	void endRenderPass() const { object->EndRenderPass(); }

	/**
	 * @brief 将指定的元命令的执行（或调用）记录到图形命令列表中。
	 * - 在执行元命令之前，调用
	 * @code ID3D12GraphicsCommandList4::InitializeMetaCommand @endcode
	 * - 在调用期间，可以为任何运行时参数的值指定替代。
	 * - 可以在同一图形命令列表中执行多个元命令。
	 * - 可以在同一命令列表中多次执行同一元命令。
	 * - 使用元命令获取PIX捕获后，可以在同一硬件配置上播放该捕获。
	 * - 但是，根据设计，它不能移植到其他GPU。
	 * @param pMetaCommand 要初始化的元命令
	 * @param pExecutionParametersData 用于执行meta命令的参数的值
	 * @param ExecutionParametersDataSizeInBytes 上一个参数指向的结构大小的size_t（如果已设置），否则为0
	 * @returns ReturnCode
	 * @note 继承自
	 * @code ID3D12GraphicsCommandList4 @endcode
	 * - 应用程序负责根据元命令规范以所需的状态设置提供给元命令的资源。
	 * - 元命令定义规范定义了每个参数的预期资源状态。
	 * - 应用程序负责插入输入资源的UAV Barrier，然后再使用元命令的算法。
	 * - 应用程序负责插入输出资源的UAV Barrier，然后再read-back输出资源。
	 * - 在算法调用期间，驱动程序可能会根据需要为输出资源插入任意数量的UAV Barrier，以同步算法实现中的输出资源使用情况。
	 * - 从应用程序的角度来看，应假定所有输出和传入/输出资源都由元命令写入，包括暂存内存。
	 */
	void executeMetaCommand(ID3D12MetaCommand* pMetaCommand, const void* pExecutionParametersData, const QWORD ExecutionParametersDataSizeInBytes) const { object->ExecuteMetaCommand(pMetaCommand, pExecutionParametersData, ExecutionParametersDataSizeInBytes); }

	/**
	 * @brief 必须至少初始化一次元命令才能再GPU线上执行它。初始化使实现能够执行加速元命令调用所需的任何工作。必须提供足够的资源参数，包括永久性缓存资源。
	 * @param pMetaCommand 要初始化的元命令
	 * @param pInitializationParametersData 包含用于初始化meta命令的参数的值
	 * @param InitializationParametersDataSizeInBytes 上一个参数指向的结构大小的size_t（如果已设置），否则为0
	 * @note 继承自
	 * @code D3D12GraphicsCommandList4 @endcode
	 */
	void initializeMetaCommand(ID3D12MetaCommand* pMetaCommand, const void* pInitializationParametersData, const QWORD InitializationParametersDataSizeInBytes) const { object->InitializeMetaCommand(pMetaCommand, pInitializationParametersData, InitializationParametersDataSizeInBytes); }

	/**
	 * @brief 在命令列表中设置状态对象。
	 * @param pStateObject 要对命令列表设置的状态对象。在当前版本中，必须是D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE类型。
	 * @note 继承自
	 * @code D3D12GraphicsCommandList4 @endcode
	 * - 可以从图形或计算命令列表和捆绑包调用此方法。
	 * - 替代ID3D12GraphicsCommandList::SetPipelineState，它仅为图形和计算着色器定义。
	 * - 命令列表中一次只有一个管道状态处于活动状态，因此任一调用都设置当前管道状态。
	 * - 调用之间的区别在于，每个调用仅设置特定类型的管道状态。
	 * - 在当前版本中，SetPipelineState1仅用于设置光线跟踪管道状态。
	 */
	void setPipelineState1(ID3D12StateObject *pStateObject) const { object->SetPipelineState1(pStateObject); }
};
