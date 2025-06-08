#pragma once

#include "Generators/MeshShapeGenerator.h"
#include "Util/IndexUtil.h"
#include "VFCommon.h"

namespace UE::Geometry::Frustum
{
    enum EFrustumPlane
    {
        Near,
        Far,
        Left,
        Right,
        Bottom,
        Top,
        ERROR
    };

    // 防止浮点数精度问题
    static int FloatFloor(float x)
    {
        x = FMath::FloorToFloat(x);
        return int(x + 0.1f);
    }

    // 防止浮点数精度问题
    static int FloatCeil(float x)
    {
        x = FMath::CeilToFloat(x);
        return int(x + 0.1f);
    }

    /*
    锥体生成器, 按SegmentSize进行分段.
    参照FGridBoxMeshGenerator写了几个版本全部无果, 干脆全部自己写了, 这样分段其实更好些.

    点存储顺序: 角点(bottom顺时针, top顺时针), 边点(X, Y, Z正方形顺时针, 值由小到大), 面点.
    边顺序: 分别以x/y/z轴的正方形作为右手螺旋定则的正方向, 再以角点0的边开始, 右手螺旋定则的方向进行.
    边点顺序: x/y/z轴的值由小到大.
    面顺序(EFrustumPlane): 0 -> near, 1 -> far, 2 -> left, 3 -> right, 4 -> bottom, 5 -> top.
    面点顺序: X轴面: -x → +x, -y → +y; 其它Side面: 正对着面(上窄下宽), 从左到右, 从上到下.
    */
    class /*VIEWFINDERCORE_API*/ FFrustumGenerator : public FMeshShapeGenerator
    {
    public: // 视锥参数
        float VerticalFOV = 60.0f;
        float AspectRatio = 16.0f / 9.0f;
        float NearPlaneDis = 100.0f;
        float FarPlaneDis = 10000.0f;
        // 根据x进行分层, 竖直朝向的边根据y进行分段, 水平方向的边根据z进行分段
        FVector SegmentSize = FVector{2000.0f, 2000.0f, 2000.0f};

    protected: // 中间参数缓存
        float NearWidthHalf = 0.f;
        float NearHeightHalf = 0.f;
        float FarWidthHalf = 0.f;
        float FarHeightHalf = 0.f;

        int Depth = -1;
        FIntVector2 NearVertices;
        FIntVector2 FarVertices;
        TArray<int> NumOfTypeVertices; // OnCorner, OnEdge, OnPlane

        // 0 -> near, 1 -> far, 2 -> left, 3 -> right, 4 -> bottom, 5 -> top
        TArray<int> NumOfPlaneVertices;
        TArray<int> NumOfPlaneTriangles;
        TArray<FVector3f> PlaneNormals; // 事实上，点法线和面法线并不需要手动填入(无需平滑, 光照), 会自动计算.

    public:
        virtual FMeshShapeGenerator &Generate() override
        {
            const float HalfFOVRad = FMath::DegreesToRadians(VerticalFOV * 0.5f);
            NearWidthHalf = NearPlaneDis * FMath::Tan(HalfFOVRad);
            NearHeightHalf = NearWidthHalf / AspectRatio;
            FarWidthHalf = FarPlaneDis * FMath::Tan(HalfFOVRad);
            FarHeightHalf = FarWidthHalf / AspectRatio;
            Depth = FloatCeil((FarPlaneDis - NearPlaneDis) / SegmentSize.X);
            NearVertices = {FloatFloor(NearWidthHalf / SegmentSize.Y) + 2,
                            FloatFloor(NearHeightHalf / SegmentSize.Z) + 2};
            FarVertices = {GetVerticesNumOfRowOrDepth(EFrustumPlane::Bottom, Depth, true),
                           GetVerticesNumOfRowOrDepth(EFrustumPlane::Left, Depth, true)};

            float HorAngle = VerticalFOV / 2;
            float VerAngle = HorAngle / AspectRatio;
            PlaneNormals.Add(FVector3f::BackwardVector);
            PlaneNormals.Add(FVector3f::ForwardVector);
            PlaneNormals.Add(FRotator3f(0, -HorAngle, 0).RotateVector(FVector3f::LeftVector));
            PlaneNormals.Add(FRotator3f(0, HorAngle, 0).RotateVector(FVector3f::RightVector));
            PlaneNormals.Add(FRotator3f(VerAngle, 0, 0).RotateVector(FVector3f::DownVector));
            PlaneNormals.Add(FRotator3f(-VerAngle, 0, 0).RotateVector(FVector3f::UpVector));

            CalculateBufferSizes();
            GenerateCornerVertex();
            GenerateEdgeVertex();
            GenerateVerticesInFrontPlane(true);
            GenerateVerticesInFrontPlane(false);
            GenerateVerticesInSidePlanes();
            GenerateUVs();
            SetSetTriangleGroup();
            GenerateTris();

            return *this;
        }

    private: // 工具函数
        int GetVerticesNumOfRowOrDepth(EFrustumPlane Plane, int DepthOrRow, bool bIncludingBoundary) const
        {
            check(DepthOrRow >= 0);
            int Res = -1;
            switch (Plane)
            {
            case EFrustumPlane::Near:
                Res = NearVertices.X;
                break;
            case EFrustumPlane::Far:
                Res = FarVertices.X;
                break;
            case EFrustumPlane::Left:
            case EFrustumPlane::Right:
            {
                float Rate = 1 - (Depth - DepthOrRow) * SegmentSize.X / FarPlaneDis;
                float Heigh = FarHeightHalf * Rate * 2;
                Res = FloatFloor(Heigh / SegmentSize.Z) + 2;
                break;
            }
            case EFrustumPlane::Bottom:
            case EFrustumPlane::Top:
            {
                float Rate = 1 - (Depth - DepthOrRow) * SegmentSize.X / FarPlaneDis;
                float Width = FarWidthHalf * Rate * 2;
                Res = FloatFloor(Width / SegmentSize.Y) + 2;
                break;
            }
            default:
                break;
            }
            if (!bIncludingBoundary)
                Res -= 2;
            return Res;
        }

        // 左, 右, 下, 上. // 不包括边点和面点
        int GetVerticesNumOfSidePlane(EFrustumPlane Plane, bool bIncludingBoundary) const
        {
            check(Depth >= 0);
            int Res = 0;
            for (int DepthCur = 0; DepthCur < Depth; ++DepthCur)
            {
                Res += GetVerticesNumOfRowOrDepth(Plane, DepthCur, bIncludingBoundary);
            }
            return Res;
        }

        // 近, 远
        int GetVerticesNumOfFrontPlane(bool bIsNear, bool bIncludingBoundary) const
        {
            int Row = bIsNear ? NearVertices.X : FarVertices.X;
            int Index = bIsNear ? NearVertices.Y : FarVertices.Y;
            if (!bIncludingBoundary)
            {
                Row -= 2;
                Index -= 2;
            }
            return Row * Index;
        }

        // 旁面(非X轴面)获取顶点索引 // checked
        int GetVertexIndexOfSidePlane(EFrustumPlane Plane, int DepthCur, int Index) const
        {
            check(Plane >= 2);
            check(DepthCur >= 0 && DepthCur <= Depth);
            int Num = GetVerticesNumOfRowOrDepth(Plane, DepthCur, true);
            check(Index >= 0 && Index < Num);

            bool IsOnEdgeOfX = Index == 0 || Index == (Num - 1);
            bool IsOnNear = DepthCur == 0;
            bool IsOnFar = DepthCur == Depth;
            bool IsCorner = IsOnEdgeOfX && (IsOnNear || IsOnFar);

            /*
            视锥不好画, 用一个立方体演示
               +z
               |
            +x 5---6
             \ |\  |\
               1-\-2 \
                \ 4---7
                 \|   |
                  0---3---+y
            */
            if (IsCorner)
            {
                if (Plane == EFrustumPlane::Left)
                {
                    if (Index == 0)
                        return IsOnNear ? 4 : 5;
                    else
                        return IsOnNear ? 0 : 1;
                }
                else if (Plane == EFrustumPlane::Right)
                {
                    if (Index == 0)
                        return IsOnNear ? 3 : 2;
                    else
                        return IsOnNear ? 7 : 6;
                }
                else if (Plane == EFrustumPlane::Bottom)
                {
                    if (Index == 0)
                        return IsOnNear ? 0 : 1;
                    else
                        return IsOnNear ? 3 : 2;
                }
                else if (Plane == EFrustumPlane::Top)
                {
                    if (Index == 0)
                        return IsOnNear ? 7 : 6;
                    else
                        return IsOnNear ? 4 : 5;
                }
            }

            // Near或Far上的边点, 交予GetVertexIndexOfFrontPlane处理
            if (IsOnNear || IsOnFar)
            {
                if (Plane == EFrustumPlane::Left)
                    return GetVertexIndexOfFrontPlane(
                        IsOnNear,
                        IsOnNear ? NearVertices.Y - Index - 1 : FarVertices.Y - Index - 1,
                        0);
                else if (Plane == EFrustumPlane::Right)
                    return GetVertexIndexOfFrontPlane(
                        IsOnNear,
                        Index,
                        IsOnNear ? NearVertices.X - 1 : FarVertices.X - 1);
                else if (Plane == EFrustumPlane::Bottom)
                    return GetVertexIndexOfFrontPlane(
                        IsOnNear,
                        0,
                        Index);
                else if (Plane == EFrustumPlane::Top)
                    return GetVertexIndexOfFrontPlane(
                        IsOnNear,
                        IsOnNear ? NearVertices.Y - 1 : FarVertices.Y - 1,
                        IsOnNear ? NearVertices.X - Index - 1 : FarVertices.X - Index - 1);
            }

            // 边点
            if (IsOnEdgeOfX)
            {
                int EdgeBase = -1;
                if ((Plane == EFrustumPlane::Bottom && Index == 0) ||
                    (Plane == EFrustumPlane::Left && Index != 0))
                    EdgeBase = 0;
                else if ((Plane == EFrustumPlane::Left && Index == 0) ||
                         (Plane == EFrustumPlane::Top && Index != 0))
                    EdgeBase = 1;
                else if ((Plane == EFrustumPlane::Top && Index == 0) ||
                         (Plane == EFrustumPlane::Right && Index != 0))
                    EdgeBase = 2;
                else if ((Plane == EFrustumPlane::Right && Index == 0) ||
                         (Plane == EFrustumPlane::Bottom && Index != 0))
                    EdgeBase = 3;
                return NumOfTypeVertices[0] + EdgeBase * (Depth - 1) + DepthCur - 1;
            }

            // 面点
            int PlaneBase = NumOfTypeVertices[0] + NumOfTypeVertices[1];
            for (int i = 0; i < Plane; ++i)
            {
                PlaneBase += NumOfPlaneVertices[i];
            }
            for (int DepthIndex = 0; DepthIndex < DepthCur; ++DepthIndex)
            {
                PlaneBase += GetVerticesNumOfRowOrDepth(Plane, DepthIndex, false);
            }
            return PlaneBase + Index - 1;
        }

        // X轴面获取顶点索引
        int GetVertexIndexOfFrontPlane(bool bIsNear, int Row, int Index) const
        {
            int RowMax = bIsNear ? NearVertices.Y : FarVertices.Y;
            int IndexMax = bIsNear ? NearVertices.X : FarVertices.X;
            check(Row >= 0 && Row < RowMax);
            check(Index >= 0 && Index < IndexMax);

            bool IsOnEdgeOfY = Row == 0 || Row == RowMax - 1;
            bool IsOnEdgeOfZ = Index == 0 || Index == IndexMax - 1;

            // 角点
            if (IsOnEdgeOfY && IsOnEdgeOfZ)
            {
                if (Row == 0 && Index == 0) // 左下
                    return bIsNear ? 0 : 1;
                else if (Row == 0 && Index == IndexMax - 1) // 右下
                    return bIsNear ? 3 : 2;
                else if (Row == RowMax - 1 && Index == 0) // 左上
                    return bIsNear ? 4 : 5;
                else if (Row == RowMax - 1 && Index == IndexMax - 1) // 右上
                    return bIsNear ? 7 : 6;
            }

            // 水平(Y)边点, 近下, 远下, 远上, 近上
            if (IsOnEdgeOfY)
            {
                int Offset = -1;
                if (bIsNear)
                {
                    Offset = Row == 0 ? 0 : NearVertices.X - 2 + (FarVertices.X - 2) * 2;
                }
                else
                {
                    Offset = Row == 0 ? NearVertices.X - 2 : NearVertices.X - 2 + FarVertices.X - 2;
                }
                check(Offset >= 0);
                return NumOfTypeVertices[0] + 4 * (Depth - 1) + Offset + Index - 1;
            }

            // 竖直(Z)边点, 近左, 近右, 远右, 远左
            if (IsOnEdgeOfZ)
            {
                int Offset = -1;
                if (bIsNear)
                {
                    Offset = Index == 0 ? 0 : NearVertices.Y - 2;
                }
                else
                {
                    Offset = Index == 0 ? (NearVertices.Y - 2) * 2 : (NearVertices.Y - 2) * 2 + FarVertices.Y - 2;
                }
                check(Offset >= 0);
                return NumOfTypeVertices[0] + 4 * (Depth - 1) +
                       2 * (NearVertices.X - 2 + FarVertices.X - 2) +
                       Offset + Row - 1;
            }

            // 面点
            int Base = NumOfTypeVertices[0] + NumOfTypeVertices[1];
            Base += bIsNear ? 0 : NumOfPlaneVertices[0];
            return Base + (Row - 1) * (bIsNear ? NearVertices.X - 2 : FarVertices.X - 2) + Index - 1;
        }

        // 顶点索引
        int GetVertexIndex(EFrustumPlane Plane, int DepthOrRow, int Index) const
        {
            return Plane > EFrustumPlane::Far
                       ? GetVertexIndexOfSidePlane(Plane, DepthOrRow, Index)
                       : GetVertexIndexOfFrontPlane(Plane == EFrustumPlane::Near,
                                                    DepthOrRow, Index);
        }

        int GetTrisNumOfSidePlane(EFrustumPlane Plane)
        {
            int Res = 0, NumOfCur = -1;
            int NumOfLast = GetVerticesNumOfRowOrDepth(Plane, 0, true);
            for (int DepthCur = 1; DepthCur <= Depth; ++DepthCur)
            {
                NumOfCur = GetVerticesNumOfRowOrDepth(Plane, DepthCur, true);
                Res += NumOfLast + NumOfCur - 2; // 三角化后三角形数为顶点数-2
                NumOfLast = NumOfCur;
            }
            return Res;
        }

    private: // 生成函数
        // 预分配
        void CalculateBufferSizes()
        {
            // 不包含角点和边点
            NumOfPlaneVertices.SetNum(6);
            NumOfPlaneVertices[0] = GetVerticesNumOfFrontPlane(true, false);
            NumOfPlaneVertices[1] = GetVerticesNumOfFrontPlane(false, false);
            NumOfPlaneVertices[2] = GetVerticesNumOfSidePlane(EFrustumPlane::Left, false);
            NumOfPlaneVertices[3] = NumOfPlaneVertices[2];
            NumOfPlaneVertices[4] = GetVerticesNumOfSidePlane(EFrustumPlane::Bottom, false);
            NumOfPlaneVertices[5] = NumOfPlaneVertices[4];

            NumOfPlaneTriangles.SetNum(6);
            NumOfPlaneTriangles[0] = 2 * (NearVertices.X - 1) * (NearVertices.Y - 1);
            NumOfPlaneTriangles[1] = 2 * (FarVertices.X - 1) * (FarVertices.Y - 1);
            NumOfPlaneTriangles[2] = GetTrisNumOfSidePlane(EFrustumPlane::Left);
            NumOfPlaneTriangles[3] = NumOfPlaneTriangles[2];
            NumOfPlaneTriangles[4] = GetTrisNumOfSidePlane(EFrustumPlane::Bottom);
            NumOfPlaneTriangles[5] = NumOfPlaneTriangles[4];
            {
                // 角点 + 边点 + 面点
                int NumOfVerticesOnCorner = 8;
                int NumOfVerticesOnEdge = 4 * (Depth - 1) +
                                          2 * (NearVertices.X - 2 + FarVertices.X - 2) +
                                          2 * (NearVertices.Y - 2 + FarVertices.Y - 2);
                int NumOfVerticesOnPlane = 0;
                for (int Plane = 0; Plane < EFrustumPlane::ERROR; ++Plane)
                {
                    NumOfVerticesOnPlane += NumOfPlaneVertices[Plane];
                }
                NumOfTypeVertices = {NumOfVerticesOnCorner, NumOfVerticesOnEdge, NumOfVerticesOnPlane};
            }

            int NumOfVertices = NumOfTypeVertices[0] + NumOfTypeVertices[1] + NumOfTypeVertices[2];
            int NumUVsAndNormals = NumOfPlaneVertices[0] + NumOfPlaneVertices[1] +
                                   NumOfPlaneVertices[2] + NumOfPlaneVertices[3] +
                                   NumOfPlaneVertices[4] + NumOfPlaneVertices[5];
            int NumTriangles = NumOfPlaneTriangles[0] + NumOfPlaneTriangles[1] +
                               NumOfPlaneTriangles[2] + NumOfPlaneTriangles[3] +
                               NumOfPlaneTriangles[4] + NumOfPlaneTriangles[5];

            SetBufferSizes(NumOfVertices, NumTriangles, NumUVsAndNormals, NumUVsAndNormals);
        }

        /*
        生成角点位置
        视锥不好画, 用一个立方体演示
           +z
           |
        +x 5---6
          \|\  |\
           1-\-2 \
            \ 4---7
             \|   |
              0---3---+y
        */
        void GenerateCornerVertex()
        {
            Vertices[0] = FVector(NearPlaneDis, -NearWidthHalf, -NearHeightHalf);
            Vertices[1] = FVector(FarPlaneDis, -FarWidthHalf, -FarHeightHalf);
            Vertices[2] = FVector(FarPlaneDis, FarWidthHalf, -FarHeightHalf);
            Vertices[3] = FVector(NearPlaneDis, NearWidthHalf, -NearHeightHalf);
            Vertices[4] = FVector(NearPlaneDis, -NearWidthHalf, NearHeightHalf);
            Vertices[5] = FVector(FarPlaneDis, -FarWidthHalf, FarHeightHalf);
            Vertices[6] = FVector(FarPlaneDis, FarWidthHalf, FarHeightHalf);
            Vertices[7] = FVector(NearPlaneDis, NearWidthHalf, NearHeightHalf);
        }

        // 生成边点位置, 不包括角点
        void GenerateEdgeVertex()
        {
            // X轴, 零头放到视锥前端
            {
                float Rate = 0.f, WidthHalf = 0.f, HeightHalf = 0.f, Distance = 0.f;
                int IndexOfLeftTop, IndexOfLeftBottom, IndexOfRightTop, IndexOfRightBottom;
                for (int DepthCur = 1; DepthCur < Depth; ++DepthCur)
                {
                    Rate = 1 - (Depth - DepthCur) * SegmentSize.X / FarPlaneDis;
                    Distance = FarPlaneDis * Rate;
                    WidthHalf = FarWidthHalf * Rate;
                    HeightHalf = FarHeightHalf * Rate;
                    IndexOfLeftBottom = GetVertexIndex(EFrustumPlane::Bottom, DepthCur, 0);
                    IndexOfLeftTop = GetVertexIndex(EFrustumPlane::Left, DepthCur, 0);
                    IndexOfRightTop = GetVertexIndex(EFrustumPlane::Top, DepthCur, 0);
                    IndexOfRightBottom = GetVertexIndex(EFrustumPlane::Right, DepthCur, 0);
                    Vertices[IndexOfLeftBottom] = FVector(Distance, -WidthHalf, -HeightHalf);
                    Vertices[IndexOfLeftTop] = FVector(Distance, -WidthHalf, HeightHalf);
                    Vertices[IndexOfRightTop] = FVector(Distance, WidthHalf, HeightHalf);
                    Vertices[IndexOfRightBottom] = FVector(Distance, WidthHalf, -HeightHalf);
                }
            }

            // Y轴, 轴对称, 零头挤到左右两边，不应当处理到角点
            {
                auto Generate = [this](const bool &IsNear)
                {
                    float XOffset = IsNear ? NearPlaneDis : FarPlaneDis;
                    float ZOffset = IsNear ? NearHeightHalf : FarHeightHalf;
                    auto Plane = IsNear ? EFrustumPlane::Near : EFrustumPlane::Far;
                    auto PlaneVertices = IsNear ? NearVertices : FarVertices;
                    float YOffset = 0.f;
                    if (PlaneVertices.X % 2 == 0)
                        YOffset = SegmentSize.Y / 2;

                    for (int Left = (PlaneVertices.X - 1) / 2, Right = PlaneVertices.X / 2; Right < PlaneVertices.X - 1; --Left, ++Right)
                    {
                        int IndexOfVertex = GetVertexIndex(Plane, 0, Left);
                        Vertices[IndexOfVertex] = FVector(XOffset, -YOffset, -ZOffset);
                        IndexOfVertex = GetVertexIndex(Plane, PlaneVertices.Y - 1, Left);
                        Vertices[IndexOfVertex] = FVector(XOffset, -YOffset, ZOffset);

                        IndexOfVertex = GetVertexIndex(Plane, 0, Right);
                        Vertices[IndexOfVertex] = FVector(XOffset, YOffset, -ZOffset);
                        IndexOfVertex = GetVertexIndex(Plane, PlaneVertices.Y - 1, Right);
                        Vertices[IndexOfVertex] = FVector(XOffset, YOffset, ZOffset);

                        YOffset += SegmentSize.Y;
                    }
                };
                Generate(true);
                Generate(false);
            }

            // Z轴, 轴对称, 零头挤到上下两边，不应当处理到角点
            {
                auto Generate = [this](const bool &IsNear)
                {
                    float XOffset = IsNear ? NearPlaneDis : FarPlaneDis;
                    float YOffset = IsNear ? NearWidthHalf : FarWidthHalf;
                    auto Plane = IsNear ? EFrustumPlane::Near : EFrustumPlane::Far;
                    auto PlaneVertices = IsNear ? NearVertices : FarVertices;
                    float ZOffset = 0.f;
                    if (PlaneVertices.Y % 2 == 0)
                        ZOffset = SegmentSize.Z / 2;

                    for (int Bottom = (PlaneVertices.Y - 1) / 2, Top = PlaneVertices.Y / 2; Top < PlaneVertices.Y - 1; --Bottom, ++Top)
                    {
                        int IndexOfVertex = GetVertexIndex(Plane, Bottom, 0);
                        Vertices[IndexOfVertex] = FVector(XOffset, -YOffset, -ZOffset);
                        IndexOfVertex = GetVertexIndex(Plane, Bottom, PlaneVertices.X - 1);
                        Vertices[IndexOfVertex] = FVector(XOffset, YOffset, -ZOffset);

                        IndexOfVertex = GetVertexIndex(Plane, Top, 0);
                        Vertices[IndexOfVertex] = FVector(XOffset, -YOffset, ZOffset);
                        IndexOfVertex = GetVertexIndex(Plane, Top, PlaneVertices.X - 1);
                        Vertices[IndexOfVertex] = FVector(XOffset, YOffset, ZOffset);

                        ZOffset += SegmentSize.Z;
                    }
                };
                Generate(true);
                Generate(false);
            }
        }

        // 使用GetVertexIndex后, 点遍历已经与存储顺序无关了
        // 不包括角点和边点
        void GenerateVerticesInFrontPlane(bool bIsNear)
        {
            int NumOFVerticesX = bIsNear ? NearVertices.X : FarVertices.X;
            int NumOFVerticesZ = bIsNear ? NearVertices.Y : FarVertices.Y;
            auto PlaneVertices = bIsNear ? NearVertices : FarVertices;
            TArray<float> Ys, Zs;
            {
                Ys.SetNum(PlaneVertices.X);
                float Offset = PlaneVertices.X % 2 ? 0.f : SegmentSize.Y / 2;
                for (int Left = (PlaneVertices.X - 1) / 2, Right = PlaneVertices.X / 2;
                     Right < PlaneVertices.X - 1; --Left, ++Right)
                {
                    Ys[Left] = -Offset;
                    Ys[Right] = Offset;
                    Offset += SegmentSize.Y;
                }

                Zs.SetNum(PlaneVertices.Y);
                Offset = PlaneVertices.Y % 2 ? 0.f : SegmentSize.Z / 2;
                for (int Bottom = (PlaneVertices.Y - 1) / 2, Top = PlaneVertices.Y / 2;
                     Top < PlaneVertices.Y - 1; ++Top, --Bottom)
                {
                    Zs[Bottom] = -Offset;
                    Zs[Top] = Offset;
                    Offset += SegmentSize.Z;
                }
            }

            float Offset = bIsNear ? NearPlaneDis : FarPlaneDis;
            // int IndexOfPlaneStart = NumOfTypeVertices[0] + NumOfTypeVertices[1];
            // 不包含角点和边点
            for (int i = 1; i < PlaneVertices.Y - 1; ++i)
            {
                for (int j = 1; j < PlaneVertices.X - 1; ++j)
                {
                    int Index = GetVertexIndexOfFrontPlane(bIsNear, i, j);
                    Vertices[Index] = FVector(Offset, Ys[j], Zs[i]);
                    // Normals[Index - IndexOfPlaneStart] = bIsNear ? PlaneNormals[0] : PlaneNormals[1];
                    // NormalParentVertex[Index - IndexOfPlaneStart] = Index;
                }
            }
        }

        /*
        生成面点, 对于四个侧面都按照此顺序
                Top
        Left   ----   Right 0
              /    \        1
             /      \       ...
            /        \
           /          \     Depth - 1
          --------------    Depth
               Bottom
        */
        void GenerateVerticesInSidePlanes()
        {
            int VertexIndex = -1, VerticesNum = -1;
            float Rate = 0.f, WidthHalf = 0.f, HeightHalf = 0.f, Distance = 0.f, Value = 0.f;
            int IndexOfPlaneStart = NumOfTypeVertices[0] + NumOfTypeVertices[1], IndexOfNormals = -1;
            for (int DepthCur = 1; DepthCur < Depth; ++DepthCur)
            {
                Rate = 1 - (Depth - DepthCur) * SegmentSize.X / FarPlaneDis;
                WidthHalf = FMath::Lerp(NearWidthHalf, FarWidthHalf, Rate);
                HeightHalf = FMath::Lerp(NearHeightHalf, FarHeightHalf, Rate);
                Distance = FMath::Lerp(NearPlaneDis, FarPlaneDis, Rate);

                VerticesNum = GetVerticesNumOfRowOrDepth(EFrustumPlane::Left, DepthCur, true);
                Value = VerticesNum % 2 ? 0.f : SegmentSize.Z / 2;
                for (int Left = (VerticesNum - 1) / 2, Right = VerticesNum / 2; Right < VerticesNum - 1; --Left, ++Right)
                {
                    int IndexOfVertex = GetVertexIndexOfSidePlane(EFrustumPlane::Left, DepthCur, Left);
                    Vertices[IndexOfVertex] = FVector(Distance, -WidthHalf, Value);
                    IndexOfVertex = GetVertexIndexOfSidePlane(EFrustumPlane::Left, DepthCur, Right);
                    Vertices[IndexOfVertex] = FVector(Distance, -WidthHalf, -Value);
                    Value += SegmentSize.Z;
                }

                VerticesNum = GetVerticesNumOfRowOrDepth(EFrustumPlane::Right, DepthCur, true);
                Value = VerticesNum % 2 ? 0.f : SegmentSize.Z / 2;
                for (int Left = (VerticesNum - 1) / 2, Right = VerticesNum / 2; Right < VerticesNum - 1; --Left, ++Right)
                {
                    int IndexOfVertex = GetVertexIndexOfSidePlane(EFrustumPlane::Right, DepthCur, Left);
                    Vertices[IndexOfVertex] = FVector(Distance, WidthHalf, -Value);
                    IndexOfVertex = GetVertexIndexOfSidePlane(EFrustumPlane::Right, DepthCur, Right);
                    Vertices[IndexOfVertex] = FVector(Distance, WidthHalf, Value);
                    Value += SegmentSize.Z;
                }

                VerticesNum = GetVerticesNumOfRowOrDepth(EFrustumPlane::Bottom, DepthCur, true);
                Value = VerticesNum % 2 ? 0.f : SegmentSize.Y / 2;
                for (int Left = (VerticesNum - 1) / 2, Right = VerticesNum / 2; Right < VerticesNum - 1; --Left, ++Right)
                {
                    int IndexOfVertex = GetVertexIndexOfSidePlane(EFrustumPlane::Bottom, DepthCur, Left);
                    Vertices[IndexOfVertex] = FVector(Distance, -Value, -HeightHalf);
                    IndexOfVertex = GetVertexIndexOfSidePlane(EFrustumPlane::Bottom, DepthCur, Right);
                    Vertices[IndexOfVertex] = FVector(Distance, Value, -HeightHalf);
                    Value += SegmentSize.Y;
                }

                VerticesNum = GetVerticesNumOfRowOrDepth(EFrustumPlane::Top, DepthCur, true);
                Value = VerticesNum % 2 ? 0.f : SegmentSize.Y / 2;
                for (int Left = (VerticesNum - 1) / 2, Right = VerticesNum / 2; Right < VerticesNum - 1; --Left, ++Right)
                {
                    int IndexOfVertex = GetVertexIndexOfSidePlane(EFrustumPlane::Top, DepthCur, Left);
                    Vertices[IndexOfVertex] = FVector(Distance, Value, HeightHalf);
                    IndexOfVertex = GetVertexIndexOfSidePlane(EFrustumPlane::Top, DepthCur, Right);
                    Vertices[IndexOfVertex] = FVector(Distance, -Value, HeightHalf);
                    Value += SegmentSize.Y;
                }
            }
        }

        void GenerateUVs()
        {
            // To do only when necessary.
        }

        void SetSetTriangleGroup()
        {
            // To do only when necessary.
        }

        void GenerateTris()
        {
            int IndexOfTri = 0;

            {
                int LeftTop, LeftBottom, RightTop, RightBottom;
                for (int PlaneIndex = EFrustumPlane::Near; PlaneIndex <= EFrustumPlane::Far; ++PlaneIndex)
                {
                    auto Plane = (EFrustumPlane)PlaneIndex;
                    bool IsNear = Plane == EFrustumPlane::Near;
                    auto PlaneVertices = IsNear ? NearVertices : FarVertices;
                    for (int Row = 0; Row < PlaneVertices.Y - 1; ++Row)
                    {
                        for (int Index = 0; Index < PlaneVertices.X - 1; ++Index)
                        {
                            LeftBottom = GetVertexIndex(Plane, Row, Index);
                            LeftTop = GetVertexIndex(Plane, Row + 1, Index);
                            RightTop = GetVertexIndex(Plane, Row + 1, Index + 1);
                            RightBottom = GetVertexIndex(Plane, Row, Index + 1);
                            if (IsNear)
                            {
                                // TriangleNormals[IndexOfTri] = FIndex3i(LeftTop, LeftBottom, RightTop);
                                Triangles[IndexOfTri++] = FIndex3i(LeftTop, LeftBottom, RightTop);
                                // TriangleNormals[IndexOfTri] = FIndex3i(LeftBottom, RightBottom, RightTop);
                                Triangles[IndexOfTri++] = FIndex3i(LeftBottom, RightBottom, RightTop);
                            }
                            else
                            {
                                // TriangleNormals[IndexOfTri] = FIndex3i(LeftTop, LeftBottom, RightTop);
                                Triangles[IndexOfTri++] = FIndex3i(LeftTop, RightTop, LeftBottom);
                                // TriangleNormals[IndexOfTri] = FIndex3i(LeftBottom, RightBottom, RightTop);
                                Triangles[IndexOfTri++] = FIndex3i(LeftBottom, RightTop, RightBottom);
                            }
                        }
                    }
                }
            }
            {
                /*
                将长边的点映射到短边的点上. 这思路太棒了.
                若长边的两点为同一组, 则它们应当共同构成一个三角形.
                不为同一组, 则需要作为两组之间的桥梁, 四个点构成两个三角形.
                */
                for (int PlaneIndex = EFrustumPlane::Left; PlaneIndex <= EFrustumPlane::Top; ++PlaneIndex)
                {
                    auto Plane = (EFrustumPlane)PlaneIndex;
                    for (int DepthCur = 0; DepthCur < Depth; ++DepthCur)
                    {
                        int DepthNext = DepthCur + 1;
                        int Shorter = GetVerticesNumOfRowOrDepth(Plane, DepthCur, true);
                        int Longer = GetVerticesNumOfRowOrDepth(Plane, DepthNext, true);
                        float GrounpLength = float(Longer) / Shorter;
                        int LeftTop = 0, RightTop = 0, LeftBottom = 0, RightBottom = 1;

                        while (RightBottom <= Longer - 1)
                        {
                            LeftTop = LeftBottom / GrounpLength;
                            RightTop = RightBottom / GrounpLength;
                            if (LeftTop == RightTop)
                            {
                                Triangles[IndexOfTri++] = FIndex3i(
                                    GetVertexIndex(Plane, DepthCur, LeftTop),
                                    GetVertexIndex(Plane, DepthNext, LeftBottom),
                                    GetVertexIndex(Plane, DepthNext, RightBottom));
                            }
                            else
                            {
                                Triangles[IndexOfTri++] = FIndex3i(
                                    GetVertexIndex(Plane, DepthCur, LeftTop),
                                    GetVertexIndex(Plane, DepthNext, LeftBottom),
                                    GetVertexIndex(Plane, DepthCur, RightTop));
                                Triangles[IndexOfTri++] = FIndex3i(
                                    GetVertexIndex(Plane, DepthNext, LeftBottom),
                                    GetVertexIndex(Plane, DepthNext, RightBottom),
                                    GetVertexIndex(Plane, DepthCur, RightTop));
                            }
                            LeftBottom++;
                            RightBottom++;
                        }
                    }
                }
            }
        }
    };
} // namespace UE::Geometry