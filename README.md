## 前置声明
`个人学习项目.无商业目的, 原则上也不允许商用(除非你得到了ViewFinder官方认可).开源协议: MPL2.0`

在虚幻引擎(5.3.2)中, 独立实现ViewFidner中**物体图案转换, 相机拍照, 照片覆盖, 时间回溯**的核心玩法, 并包装为**插件**.  
网格(动态网格体)切割实现, 基于官方插件[GeometryScripting](https://dev.epicgames.com/documentation/zh-cn/unreal-engine/geometry-scripting-users-guide-in-unreal-engine), 但已做了剥离, 可**独立使用**.  
**现不支持网络游戏**, 未实现色门相关机制.  
项目是个空壳, 内容已包装进插件, 蓝图资源也在插件中.两个模块: ViewFinderCore是核心代码逻辑, ViewFinderRe是用于交互和演示.  
90%+(大概?)的逻辑为C++, 蓝图使用较少, 阅读可能有难度, 最后面有类图和流程图; 类都支持蓝图化, 几乎所有函数支持蓝图调用, 优化过使用逻辑, 使用难度低.

## 目录
- [特性一览](#-特性特性一览)
- [上手Demo](#-上手Demo)
- [代码框架](#-代码框架)

## 特性一览
- 拍照与放置  
![拍照gif](Plugins/ViewFinderRe/Resources/Icon128.png)
  - 对场景中的静态网格体**无需做额外改动**, 即可使用相机相关功能.
  - VFHelper组件: 可**额外设置不可被拍入/不可被照片覆盖**; 使用**动态委托**, 可插入额外逻辑, (理论上)能够实现ViewFinder中全部的玩法.
  - 照片放置出来的物体, 遵循拍照前的模拟物理和开启重力, 无需手动管理.
  - 动态装卸的组件使用**对象池**, 组件内的动态网格体使用官方对象池.
  - 拍入本地的Pawn, (使用VFHelper和VFPawnStandIn), 放置会瞬移.
  - **视锥支持设置**: FOV, 宽高比, 近平面距离, 远平面距离.
  - 当前(或独立)场景预制照片, 可预制**递归照片**,.

- 物体透视图案转换  
![拍照gif](Plugins/ViewFinderRe/Resources/Icon128.png)
  - 使用贴花组件改造, 自动支持正确的**近/远处缩放**, **仅第一个表面存在颜色**.
  - ReplaceWithDecal()和RestoreWithActors(), (支持蓝图)可在运行时使用物体/贴花.
  - 预留动态委托触发.
  - 支持: 修改触发逻辑, 只替换部分物体, 当前(或独立)场景预制。

- 时间回溯  
![拍照gif](Plugins/ViewFinderRe/Resources/Icon128.png)
  - 使用**世界子系统**, 只与接口类VFStepsRecordInterface进行交互.
  - 已有机制都已支持回溯, 支持快速回溯到关键事件.
  - 使用AVFTransfromRecorderActor和AVFTransformRecordVolume来**批量处理**只记录Transform的静态网格体, 支持带物理的Actor.
  - 要**自定义回溯逻辑**, 需继承IVFStepsRecordInterface接口, 有**两种实现路**(建议任选其一, 也可组合使用): 
    - 实现TickForward()**生成本地的Steps记录**.实现TickBackward()获得回溯的当前时间值, 用本地的Steps执行对应回溯操作.
      - 适合: 生成较多数据的, 强要求顺序的, 高频生成的.
      - 例子: AVFCharacter需要记录Transform和速度, AVFTransfromRecorderActor记录多个物体的Transform.
    - 在操作中, **向子系统SubmitStep**当前操作信息.实现StepBack(), 拿到子系统退回的操作信息进行对应回溯操作.
      - 适合: 简单的, 偶尔触发的, 瞬时性的.
      - 例子: AVFPhoto2DSteppable的FoldUp和PlaceDown操作.
    - 另外, 插件中自带的回溯, 全部是**增量**的. 实现思路是在TickForward()中进行差异判断, 只在数据变动后提交, 或者只提交变动数据.

- 拍照滤镜  
![拍照gif](Plugins/ViewFinderRe/Resources/Icon128.png)
  - 使用**后处理**和**自定义深度模板**实现.
  - 相机在持有/收纳的时候为角色的摄像机添加/删除后处理材质, 后处理材质中对Stencil进行差异化处理.
  - 拍摄到的网格会交由UVFPostProcessComponent::SetStencilValueNext()处理, 里面会根据既定的规则对Stencil进行修改, 以达到不同的效果. 例如: 多次拍照的色暖加深, 循环色偏等.
  - 单独做了个ViewFinder西瓜滤镜和星际拓荒外星站文物的融合体.

## 上手Demo
### 流程设计  
基础实现:  
1. 物理物体位于高处, 旋转/放置头顶, 熟悉时间回溯.
2. 透视图案生成地形(需要检查?), 不同的天空盒.
3. 固定相机, 复制物体/照片.
4. 手持相机, 勾选仅Helper, 不能靠近的拍入.
5. 手持相机, 不限仅Helper, 但自定义可拍入和可覆盖.
6. 手持相机, 裁剪物理物体.
7. 后处理相机, 记得按X丢弃之前的相机再来试试哦.
进阶实现:
1. 瞬移自己, 注意朝向的同步.
2. 复制相机, 相机也可以复制, 复制固定相机拍照, 以获得更奇怪的角度的自拍.
3. 所有的委托, ViewFinder理论上所有相关的功能都能实现.
4. 递归照片, 依赖委托OnCopyBeforeBeingEnabled实现, 动态复制Photo3D的内容.
5. 迭代透视图案, 不断的加载关卡实例.
6. 西瓜滤镜/虚拟渲染, 玩过星际拓荒吗? 试试丢下相机, 离它远点.
7. 试着回溯到最开始吧, 是的, 这里的所有东西都支持回溯.

### 搭建指南
#### 贴花场景, 迭代出现固定相机
提供了两种相机: BP_PhotoDecalSteppable_Hide, BP_PhotoDecalSteppable_Show
下面使用
#### 相机, 与物块, 地形, 照片的交互
#### 不可拍入的墙, 背后的可被裁切的物块
#### 委托打印物块

#### 预处理照片
Photo2D索引Photo3D, 所以最简单的做法就是把场景放置在Photo3D中.但这样场景是没有被裁切的, 
直接由相机在编辑器中, 对场景进行拍照, 然后生成Photo2D和Photo3D, 再将这俩作为场景实例就可以完美解决这个问题.
递归照片解决方案: Photo2D的Help中, 在被放置后复制外层Photo3D, 初始化为None并FlodUp, 即可解决.
#### 迭代照片

### 还未完全解决, 打勾是有暂时的处理办法
- [x] 贴花颜色无法与场景完全一致(AVFPhotoDecal).  
      - 已知问题为:   场景捕捉的本身就与玩家相机颜色存在偏差. 一个可能的解决方案: [插件CameraCapture](https://zhuanlan.zhihu.com/p/702941885).   

      更新:   
      1. 主因: 二次光照. 贴花映射的物体再进行了一次PBR流程, 导致颜色偏暗/亮. 同时光源的暖色和间接光(?)也因为二次关照, 导致颜色偏红/黄.
      2. 依据为: 调整场景主光源强度, 强度越高,贴花越亮; 强度越弱, 贴花越暗. 测试强度为3.7 lux时较为合适.
      3. 虽然能够设置光源的"不适用暖色"和ShowFlags不显示大气, 但似乎存在不可设置的间接光(不确定)依然导致偏色.
      4. 这样的办法, 在不同的距离下颜色也不协调, 但对于ViewFinder来说，它就应该是那个距离的光照.
- [X] 场景捕捉与虚拟阴影贴图不兼容. 在使用VSM时, 捕捉到的阴影会出现块状缺失.  
      项目已关闭VSM. [官方对此问题的描述](https://dev.epicgames.com/documentation/en-us/unreal-engine/virtual-shadow-maps-in-unreal-engine#scene-capture)
- [x] 编辑器中查看组件属性, 会出现报错, 原因未知, 无影响, 忽略."LogOutputDevice: Error: Ensure condition failed: PropertyNodeMap.ParentProperty == CurObjectNode [File:D:\build\++UE5\Sync\Engine\Source\Editor\PropertyEditor\Private\DetailLayoutHelpers.cpp] [Line: 137]"
- [ ] 视锥偶尔碰撞检测不到物体, 换个角度/位置, 甚至重新来一次就行, 原因不确定. 天空盒缺失也是这个原因, 它没有被碰撞到, 没有被复制到照片, 放置出来便是空的. 猜测可能是球体被挖空后生成的碰撞有问题.  
      通过视锥分段(commit: [6423631](https://github.com/StrangeDS/ViewFinderRe/commit/6423631) [7794c81](https://github.com/StrangeDS/ViewFinderRe/commit/7794c81)), 已排除**三角面被过分拉长, 导致重叠检测失效**的可能.

      更新:   
      1. 天空盒的错误目前认为是在几次截取后, 剩余的部分凸包近似已经无法办到了, 导致生成的简单碰撞失效. 现已使用APlaneActor替代(ViewFinder的做法).
      2. 可能推翻前面的推测, 因为突然发现动态网格默认使用复杂碰撞, 所以可能是复杂碰撞的原因.
      3. AI归纳为:
         1. 物体A：使用简单碰撞(如立方体, 球体等)
         2. 物体B：使用复杂碰撞(逐三角形网格)
         3. 空间关系:
            1. A完全穿透B的内部空间
            2. A与B的任何边(棱)都不接触
            3. A仅与B的三角面片内部区域相交
      4. 又一个可能性是视锥的法线存在问题

### ToDoList:
- [x] 贴花的不穿透影响: 实现是使用一个场景捕捉当前的场景深度, 给入贴花中, 贴花比较深度纹理和像素深度, 前者 >= 后者时意味着像素在相机拍摄到的深度上, 即不穿透的面. 目前使用额外的场景捕捉单独捕获深度, SCS_SceneColorSceneDepth似乎有版本问题, 我这无法使用.
- [x] 后处理相关
- [ ] Demo场景
- [ ] README更新
### 原游戏流程记录
0.1 板子坏, 时间回溯; 照片出现传送门.  
0.2 电池, 电路板开启传送门; 照片出现电池. 物理抓手.  
0.3 旋转照片; 照片中还可以有照片.  

第一章主场景.  
1.1.1 照片用于生成地形(距离、角度)、破坏地形.照片的大小.  
1.1.2 可破坏地形, 也可以破坏关键道具.  
1.1.3 0.3迭代版, 全景拍照投影.插入背景.  
1.1.4 多风格照片  

1.2.1 影印机获取电池.  
1.2.2 照片搭建地形.多照片组合地形.  

挑战关卡  
1.x.1 场景迭代.重力不同下的多照片地形组合.  

第二章主场景.  
2.1.1 透视图案的还原.可迭代.  
2.1.2 照片破坏地形, 透视图案的物体不应被破坏, 地形还要求能够到达透视图案的指定位置和旋转.  
2.1.3 开关; 三原色组合图案.  
 
2.2.1 固定相机; 拍摄制造地形、拉近传送门.  
2.2.2 固定相机; 地形, 复制电池.  
2.2.3 两个固定相机; 移动相机去拍摄、拉近传送门.  

2.3.1 图案/照片与物理电池的顺序逻辑解密.  
2.3.2 开关, 图案破坏, 固定相机, 电池的顺序逻辑解密.  

挑战关卡  
2.x.1 传送门处于照片物体背面, 3维旋转解密.  

第三章主场景  
3.1.1 开关三原色; 获得相机, 复制电池.  

3.2.1 相机进阶使用; 地形旋转.  
3.2.2 相机主动拉近传送门, 距离.  
3.2.3 主动拉近传送门, 地形.  
3.2.4 主动拉近传送门, 倒置.  
3.2.5 相机破坏地形, 复制电池.  

3.3.1 相机破坏地形, 旋转传送门.  
3.3.2 相机复制电池, 同时作为地形使用.  
3.3.3 相机复制电池, 同时旋转地形.  
3.3.4 需要相机远处, 角度, 拍下开关和传送门.  
3.3.5 同上, 要求更强.  
3.3.6 同上, 拍照角度需要自己去构建地形.  
3.3.7 同上, 但竖直方向.  

3.4.1 延时开关; 破坏地形.  
3.4.2 延时节点; 旋转地形.  
3.4.3 同上, 但触发需要自行制造地形.  
3.4.4 同上, 但拍照需要角度, 抢时间.  
3.4.5 同上, 但利用重力枪时间.  

3.5.1 实体变图案; 地形变化.  
3.5.2 错误的图案映在正确的实体上; 地形变化.  
3.5.3 错图正体; 靠近变图案; 实体重力掉出触发区域.  

第四章主场景  
4.1.1 不可被拍入且不可被覆盖.  
4.1.2 垫板凳; 利用不可被覆盖, 只覆盖门; 利用不可被拍照, 跨墙拍摄.  
4.1.3 利用不可被覆盖, 不覆盖地形.  

4.2.1 利用不可被覆盖, 跨墙放置物体.  
4.2.2 同上, 但把传送门放进去.  
4.2.3 利用不可被拍入, 跨墙拍传送门.  
4.2.4 同4.2.2.  

4.3.1 音源开关; 三原色照片.  
4.3.2 照片移动音源.  
4.3.3 利用不可覆盖, 串联音源.  
4.3.4 延时音源串联; 照片拉近距离.  
4.3.5 同4.3.2, 但用照片搬延时音源.  
4.3.6 三维旋转, 串联电路.  

4.x.1 胶卷也能复制; 2指数幂的复制.  
4.x.2 制造地形, 但尽可能多和远.  
4.x.3 三维旋转, 但接住电池.  
4.x.4 资源管理.一张破门&复制电池; 一张旋转地形&复制电池.  
4.x.5 (西瓜)重量开关; 切西瓜.  

4.4.- 色门机制.  

第五章主场  
5.1.1 定时拍照; 拍自己, 瞬移进不可覆盖的笼子.  
5.1.2 同上, 但进去开开关再出来.  
5.1.3 同上, 但还抓着电池.  
5.1.4 时机, 远距离瞬移自己.  
5.1.5 利用不可被拍入, 远距离瞬移自己.  

5.2.1 被裁切.  
5.2.2 剪切地形.  
5.2.3 剪切电池, 影印机复印.  
5.2.4 利用剪切, 同时拆墙和搭路.  

5.3.1 同时拍电池和自己.  
5.3.2 裁切会断路, 瞬移自己抢时间.  

5.4.1 最终关卡.  

### 未实现猜测(八九不离十):
1. Actor失效: 使用Helper就完事了
   1. 电池, 传送装置, 管线等大多数: 使用OnCopyEndPlacingPhoto, OnOriginalEndPlacingPhoto判断(关键)组件的三角面(/顶点)数。
   2. 风车: OnOriginalEndTakingPhoto判断关键组件.
   3. 猫咪: 使用透明网格的StandIn，这样就能: 不被拍入照片, 拍摄换位置, 放置后出现在新场景等.
2. 关卡传送装置: 这个其实很怪异.
   1. 能够确认并非预制的贴图, 更可能像传送门一样实时实时绘制.
   2. 实时绘制就疑惑是怎么处理性能问题的, 多的时候:
      1. 要一起渲染可能10+个场景?
      2. 10+场景都加载在内存中?
3. 缆车切换大关卡:
   1. 经测试(疯狂移动和跳), 隧道中会卡两次, 意思就很明确: 卸载, 加载.
   2. 这就仅需要弯曲隧道处一致就看不出来差别了.
   3. 隧道出口, 没想到具体实现, 但估计也是一个后处理效果.
4. 色门: 
   1. 传送门:
      1. 传送门摄像机同步玩家摄像机位置.
      2. 拥有一个自定义模板值, 摄像机使用此模板值.
      3. 传送碰撞体积翻转法线, 可避免穿透绘制平面.
   2. 进入传送门将交换玩家的自定义模板值与传送门的模板值.
   3. 进入传送门将设置玩家摄像机自定义模板值, 委托触发障碍显隐.
5. 剪切下的螺旋错误:
   1. 可能性1(不大):
      1. 背景的螺旋错误, 实质是背景生成面(AVFPlaneActor)上赋予自定义模板值. 在OnOriginalEndTakingPhoto处理就完事了: 不隐藏且赋予自定义模板值.
      2. 近处的螺旋错误, 实质是将布尔差的网格在原地去掉碰撞, 然后赋予了自定义模板值.
      3. 需要担心的问题是, 这么多的后处理效果, 到底该怎么好写进一个后处理材质中?
   2. 可能性2: 复制一个Photo3D，然后将里面的东西全部设为破坏的材质.
6. 相机和物体的后处理:
7. 西瓜: 估计是材质实现.

## 代码框架
### 类图
#### PhotoCatcher相关, 最为复杂, 设计两层接口和功能衍生
```mermaid
classDiagram
    class PhotoCatcher {
      #TObjectPtr~UVFViewFrustumComponent~ ViewFrustum
      #TObjectPtr~UVFPhotoCaptureComponent~ PhotoCapture

      +virtual void GetOverlapComps()
      +virtual void TakeAPhoto()
    }

    class IVFStepsRecordInterface {
      <<interface>>
      +void TickForward(float Time)
      +void TickBackward(float Time)
      +virtual bool StepBack(FVFStepInfo &StepInfo)
    }

    class AVFPhotoCatcherSteppable {
      +TObjectPtr~UVFStepsRecorderWorldSubsystem~ StepRecorder

      +virtual bool StepBack(FVFStepInfo &StepInfo) override
    }

    class IVFInteractInterface {
      <<interface>>
      +virtual bool StartAiming(APlayerController*)
      +virtual bool EndAiming(APlayerController*)
      +virtual bool Interact(APlayerController*)
    }

    class AVFPhotoCatcher_Interact {
      #TSubclassOf~UUserWidget~ AimingHintUMG
    }

    class AVFPhotoCatcher_Fixed {
      +FTimerHandle TimerHandleOfTakingPhoto
      +float TimeOfTakingPhoto
    }

    class AVFPhotoCatcher_PickUp {
      +void PickUp()
      +void DropDown()
    }

    class AVFPhotoCatcherPref {
      +void PrefabricateAPhotoLevel()
    }

    AActor <|-- PhotoCatcher: 继承
    PhotoCatcher <|-- AVFPhotoCatcherSteppable: 继承
    IVFStepsRecordInterface <|.. AVFPhotoCatcherSteppable: 实现
    AVFPhotoCatcherSteppable <|-- AVFPhotoCatcher_Interact: 继承
    IVFInteractInterface <|.. AVFPhotoCatcher_Interact: 实现
    AVFPhotoCatcher_Interact <|-- AVFPhotoCatcher_Fixed: 继承
    AVFPhotoCatcher_Interact <|-- AVFPhotoCatcher_PickUp: 继承
    PhotoCatcher <|-- AVFPhotoCatcherPref: 继承
```

#### 动态网格体相关, 拍照后网格体都会被用它来替代
```mermaid
classDiagram
    class UVFDynamicMeshComponent {
      +TObjectPtr~UPrimitiveComponent~ SourceComponent

      +virtual void CopyMeshFromComponent()
      +virtual void ReplaceMeshForComponent()
      +virtual void IntersectMeshWithDMComp()
      +virtual void SubtractMeshWithDMComp()
      +virtual void UnionMeshWithDMComp()
      +void UnionMeshWithDMComp()
    }

    class IVFStepsRecordInterface {
      <<interface>>
      +void TickForward(float Time)
      +void TickBackward(float Time)
      +virtual bool StepBack(FVFStepInfo &StepInfo)
    }

    class FVFDMCompStep {
      UVFDMCompStepOperation Operation
      TObjectPtr~UDynamicMesh~ Mesh
      float Time
    }

    class UVFDMSteppableComponent {
      +UVFStepsRecorderWorldSubsystem StepRecorder
      +TArray~FVFDMCompStep~ Steps
      +UDynamicMeshPool LocalPool

      +virtual bool StepBack(FVFStepInfo &StepInfo) override
    }

    UDynamicMeshComponent <|-- UVFDynamicMeshComponent: 继承
    UVFDynamicMeshComponent <|-- UVFDMSteppableComponent: 继承
    IVFStepsRecordInterface <|.. UVFDMSteppableComponent: 实现
    FVFDMCompStep <.. UVFDMSteppableComponent: 数据结构
```

#### Photo2D和Photo3D, 分别作为手中的照片和场景中的照片
```mermaid
classDiagram
    class AVFPhoto2D {
      #TObjectPtr~UTextureRenderTarget2D~ RenderTarget
      #TObjectPtr~UMaterialInstanceDynamic~ MaterialInstance
      #TObjectPtr~AVFPhoto3D~ Photo3D

      +virtual void FoldUp()
      +virtual void PlaceDown()
      +void Preview() 
    }

    class AVFPhoto2DSteppable {
      TObjectPtr~UVFStepsRecorderWorldSubsystem~

      +virtual bool StepBack(FVFStepInfo &StepInfo) override
    }

    class AVFPhoto3D {
      +TObjectPtr~UVFViewFrustumComponent~ ViewFrustumRecorder

      +virtual void FoldUp()
      +virtual void PlaceDown()
      +void SetViewFrustumVisible(const bool &Visiblity)
    }

    class AVFPhoto3DSteppable {
      TObjectPtr~UVFStepsRecorderWorldSubsystem~

      +virtual bool StepBack(FVFStepInfo &StepInfo) override
    }

    class IVFStepsRecordInterface {
      <<interface>>
      +void TickForward(float Time)
      +void TickBackward(float Time)
      +virtual bool StepBack(FVFStepInfo &StepInfo)
    }

    AActor <|-- AVFPhoto2D: 继承
    AActor <|-- AVFPhoto3D: 继承
    AVFPhoto2D --> AVFPhoto3D: 关联
    AVFPhoto2D <|-- AVFPhoto2DSteppable: 继承
    AVFPhoto3D <|-- AVFPhoto3DSteppable: 继承
    IVFStepsRecordInterface <|.. AVFPhoto2DSteppable: 实现
    IVFStepsRecordInterface <|.. AVFPhoto3DSteppable: 实现
```

#### PhotoDecal, 与其它的交互最少, 结构最清晰
```mermaid
classDiagram
    class AVFPhotoDecal {
      +TObjectPtr~UVFPhotoCaptureComponent~ PhotoCapture
      +TObjectPtr~UVFViewFrustumComponent~ ViewFrustum
      +TObjectPtr~UDecalComponent~ Decal
      +FVFPhotoDecalDelegate OnReplace
      +FVFPhotoDecalDelegate OnRestore

      +void ReplaceWithDecal()
      +void RestoreWithActors()
    }

    class IVFStepsRecordInterface {
      <<interface>>
      +void TickForward(float Time)
      +void TickBackward(float Time)
      +virtual bool StepBack(FVFStepInfo &StepInfo)
    }

    class AVFPhotoDecalSteppable {
      +UVFStepsRecorderWorldSubsystem StepRecorder

      +virtual bool StepBack(FVFStepInfo &StepInfo) override
    }

    AActor <|-- AVFPhotoDecal: 继承
    AVFPhotoDecal <|-- AVFPhotoDecalSteppable: 继承
    IVFStepsRecordInterface <|.. AVFPhotoDecalSteppable: 实现
```

#### PawnStandIn: 在Pawn被拍到时, 用之替换
```mermaid
classDiagram
    class IVFStandInInterface {
      <<interface>>
      +virtual void SetSourceActor(AActor *Source)
      +virtual UPrimitiveComponent *GetPrimitiveComp()
    }

    class AVFPawnStandIn {
      +TObjectPtr~APawn~ TargetPawn
      +TObjectPtr~UStaticMeshComponent~ StaticMesh
      +TObjectPtr~UVFHelperComponent~ Helper

      +void TeleportTargetPawn(UObject* Sender)
      +virtual void SetSourceActor(AActor *Source) override
      +virtual UPrimitiveComponent *GetPrimitiveComp() override
    }

    class IVFStepsRecordInterface {
      <<interface>>
      +virtual void SetSourceActor(AActor *Source)
      +virtual UPrimitiveComponent *GetPrimitiveComp()
    }

    class AVFPawnStandInSteppable {
      +UVFStepsRecorderWorldSubsystem StepRecorder

      +virtual bool StepBack(FVFStepInfo &StepInfo) override
    }

    AActor <|-- AVFPawnStandIn: 继承
    IVFStandInInterface <|.. AVFPawnStandIn: 实现
    AVFPawnStandIn <|-- AVFPawnStandInSteppable: 继承
    IVFStepsRecordInterface <|.. AVFPawnStandInSteppable: 实现
```
---

### 核心流程
#### 拍照
```mermaid
%%{init: {'theme': 'neutral', 'themeVariables': { 'clusterBkg': '#f5f5f5'}}}%%
flowchart TB
    subgraph sg1 [PhotoCatcher触发]
        A[重叠检测组件]
    end

    A --> B[映射Helper组件] --> C[替换StandIn]
    C --> sg2 --> sg3 --> F[附加到新创建的Photo3D]
    F --> G[创建Photo2D进行拍照] --> H[替换的网格与视锥做差集]
    H --> I[复制网格与视锥做交集] --> J[Photo2D和Photo3D的后续处理, 保存额外信息等]
    J --> 返回Photo2D

    subgraph sg2 [CheckVFDMComps]
        direction LR
        sg2A[Actor是否存在动态网格] -->|不存在| sg2B[将相关网格都替换成动态网格]
        sg2A -->|存在| sg2C[只会是动态网格]
    end

    subgraph sg3 [CloneActorRuntime]
        direction TB
        sg3A[原Actor] --> sg3B[移除所有动态网格] --> sg3C[装回动态网格]
        sg3B --NewObject使用Template--> sg3D[副本Actor]
        sg3D --> sg3E[复制组件并同步层级]
        sg3C -.辅助.- sg3E
    end

    subgraph delegates [VFHelper动态委托]
        direction TB
        dA[OnOriginalBeforeTakenInPhoto]
        dB[OnOriginalBeforeCopyingToPhoto]
        dC[OnOriginalAfterCutByPhoto]
        dD[OnOriginalAfterTakingPhoto]
        dE[OnCopyAfterCopiedForPhoto]
        dF[OnCopyBeforeFoldedInPhoto]
        dG[OnCopyAfterPlacedByPhoto]
    end

    %% 样式调整
    classDef main fill:#e6f3ff,stroke:#4a90e2,stroke-width:2px;
    classDef sub fill:#f0f0f0,stroke:#666,stroke-width:1px;
    classDef delegate fill:#fff3cd,stroke:#ffc107,stroke-dasharray:5 5;

    class A,B,C,F,G,H,I,J main;
    class sg1,sg2,sg3,delegates sub;
    class dA,dB,dC,dD,dE,dF,dG delegate;

    %% 连接线调整
    sg2A -.触发.-> dA
    sg3A -.触发.-> dB
    I -.触发.-> dC
    J -.触发.-> dD
    sg3E -.触发.-> dE
    J -.触发.-> dF

    %% 对齐优化
    linkStyle 0,1,2,3,4,5,6,7,8 stroke:#4a90e2,stroke-width:2px;
    linkStyle 9,10,11,12,13,14 stroke:#666,stroke-dasharray:3;
```

#### Photo3D放置, 覆盖场景
```mermaid
%%{init: {'theme': 'neutral', 'themeVariables': { 'clusterBkg': '#f5f5f5'}}}%%
flowchart TB
    A[重叠检测组件] --> B[映射Helper组件] --> sg1 --> C[替换的网格与视锥做差集] --> D[递归恢复Actor状态] --> E[结束]

    subgraph sg1 [CheckVFDMComps]
        direction LR
        sg1A{Actor是否存在动态网格} -->|不存在| sg1B[替换为动态网格]
        sg1A -->|存在| sg1C[保持动态网格]
    end

    subgraph sg2 [VFHelper组件配置]
        C
    end

    subgraph delegates [动态委托]
        direction TB
        dC[OnOriginalAfterCutByPhoto]
        dG[OnCopyAfterPlacedByPhoto]
    end

    %% 修正后的样式定义
    classDef main fill:#e6f3ff,stroke:#4a90e2;
    classDef condition fill:#ffebee,stroke:#e53935;
    classDef delegate fill:#fff3cd,stroke:#ffc107,stroke-dasharray:5 5;
    classDef subCluster fill:#f8f9fa,stroke:#adb5bd;

    class A,B,C,D,E main;
    class sg1A condition;
    class dC,dG delegate;
    class sg1,sg2,delegates subCluster;

    %% 连接线优化
    C -.->|触发| dC
    D -.->|触发| dG

    %% 连接线样式
    linkStyle 0,1,2,3,4 stroke:#4a90e2,stroke-width:2px;
    linkStyle 5,6 stroke:#666,stroke-dasharray:3;
```