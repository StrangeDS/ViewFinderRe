// Copyright StrangeDS. All Rights Reserved.

#pragma once

#include "Generators/MeshShapeGenerator.h"
#include "Util/IndexUtil.h"
#include "VFLog.h"

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

    // Prevent floating-point precision issues
    static int FloatFloor(float x)
    {
        x = FMath::FloorToFloat(x);
        return int(x + 0.1f);
    }

    // Prevent floating-point precision issues
    static int FloatCeil(float x)
    {
        x = FMath::CeilToFloat(x);
        return int(x + 0.1f);
    }

    /*
    Frustum generator, segmented according to SegmentSize.

    Vertex order(Vertices):
    CornerVertices(clockwise in bottom plane, clockwise in top plane),
    EdgeVertices(X, Y, Z square clockwise, values from small to large),
    PlaneVertices.

    Edge order:
    Using the squares along the x/y/z axes as the positive direction according to the right-hand rule,
    Starting from the edge at corner point 0, following the direction of the right-hand rule.
    EdgeVertices order:: Values along x/y/z axes from small to large.
    Plane order(EFrustumPlane): 0 -> near, 1 -> far, 2 -> left, 3 -> right, 4 -> bottom, 5 -> top.
    PlaneVertices oorder:
    X-axis planes: -x -> +x, -y -> +y.
    Other side planes: facing the plane directly (narrower at top, wider at bottom), left to right, top to bottom.
    */
    class /*VIEWFINDERCORE_API*/ FFrustumGenerator : public FMeshShapeGenerator
    {
    public: // Frustum Parameters
        float VerticalFOV = 60.0f;
        float AspectRatio = 16.0f / 9.0f;
        float NearPlaneDis = 100.0f;
        float FarPlaneDis = 10000.0f;
        // Stratified along the X-axis, vertically oriented edges segmented according to Y, horizontally oriented edges segmented according to Z.
        FVector SegmentSize = FVector{2000.0f, 2000.0f, 2000.0f};

    protected: // Intermediate Parameter Cache
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
        TArray<FVector3f> PlaneNormals; // In fact, point normals and face normals do not need to be filled in manually (no need for smoothing and lighting), and will be calculated automatically.

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

    private: // Utility function
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
                float Heigh = FMath::Max(FarHeightHalf * Rate * 2, NearHeightHalf * 2);
                Res = FloatFloor(Heigh / SegmentSize.Z) + 2;
                break;
            }
            case EFrustumPlane::Bottom:
            case EFrustumPlane::Top:
            {
                float Rate = 1 - (Depth - DepthOrRow) * SegmentSize.X / FarPlaneDis;
                float Width = FMath::Max(FarWidthHalf * Rate * 2, NearWidthHalf * 2);
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

        // Left, right, bottom, top planes. Exclude edge points and face points.
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

        // Near, Far planes.
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

        // Side face (non X axis face) Get vertex index
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
            The sight cone is not easy to draw. Use a cube to demonstrate.
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

            // EdgeVertices on the Near or Far plane are handled by GetVertexIndexOfFrontPlane.
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

            // EdgeVertices
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

            // PlaneVertices
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

        // X axis face acquiring vertex index
        int GetVertexIndexOfFrontPlane(bool bIsNear, int Row, int Index) const
        {
            int RowMax = bIsNear ? NearVertices.Y : FarVertices.Y;
            int IndexMax = bIsNear ? NearVertices.X : FarVertices.X;
            check(Row >= 0 && Row < RowMax);
            check(Index >= 0 && Index < IndexMax);

            bool IsOnEdgeOfY = Row == 0 || Row == RowMax - 1;
            bool IsOnEdgeOfZ = Index == 0 || Index == IndexMax - 1;

            // CornerVertex
            if (IsOnEdgeOfY && IsOnEdgeOfZ)
            {
                if (Row == 0 && Index == 0) // Bottom left
                    return bIsNear ? 0 : 1;
                else if (Row == 0 && Index == IndexMax - 1) // Bottom right
                    return bIsNear ? 3 : 2;
                else if (Row == RowMax - 1 && Index == 0) // Top left
                    return bIsNear ? 4 : 5;
                else if (Row == RowMax - 1 && Index == IndexMax - 1) // Top Right
                    return bIsNear ? 7 : 6;
            }

            // Horizontal (Y) edge point, near bottom, far bottom, far top, near top
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

            // Vertical (Z) edge point, near left, near right, far right, far left
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

            // Plane vertices
            int Base = NumOfTypeVertices[0] + NumOfTypeVertices[1];
            Base += bIsNear ? 0 : NumOfPlaneVertices[0];
            return Base + (Row - 1) * (bIsNear ? NearVertices.X - 2 : FarVertices.X - 2) + Index - 1;
        }

        // Vertex Index
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
                Res += NumOfLast + NumOfCur - 2; // Number of triangles after triangulation equals vertex count minus 2.
                NumOfLast = NumOfCur;
            }
            return Res;
        }

    private:
        // Preallocation
        void CalculateBufferSizes()
        {
            // Exclude corner vertices and edge vertices
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
                // corner vertices + edge vertices + plane vertices
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
        Generate Corner vertices
        he sight cone is not easy to draw. Use a cube to demonstrate.
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

        // Generate edge vertices, Exclude corner vertices
        void GenerateEdgeVertex()
        {
            // X-axis, place fractional values at the frustum front.
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

            // Y-axis, symmetrically distributed, squeeze the fractional parts of the values to both left and right sides, and should not process corner points.
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

            // Z-axis, symmetrically distributed, squeeze the fractional parts to the top and bottom sides, and should not affect corner points.
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

        // After using GetVertexIndex, point traversal is no longer dependent on the storage order.
        // Exclude corner vertices and edge vertices.
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
            // Exclude corner vertices and edge vertices
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
        Generate face points, applying this same order to all four side faces.
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
                                Triangles[IndexOfTri++] = FIndex3i(LeftTop, LeftBottom, RightTop);
                                // TriangleNormals[IndexOfTri] = FIndex3i(LeftBottom, RightBottom, RightTop);
                                Triangles[IndexOfTri++] = FIndex3i(LeftBottom, RightBottom, RightTop);
                            }
                        }
                    }
                }
            }
            {
                /*
                Map vertices from the longer edge to the shorter edge. This approach is brilliant.
                If two vertices on the longer edge belong to the same group, they should form a single triangle together.
                If they are not in the same group, they need to act as a bridge between the two groups, forming two triangles with four vertices.
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