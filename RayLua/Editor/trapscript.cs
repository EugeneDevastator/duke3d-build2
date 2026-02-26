using UnityEngine;
using System.Collections.Generic;

[ExecuteAlways]
[RequireComponent(typeof(MeshFilter), typeof(MeshRenderer))]
public class trapscript : MonoBehaviour
{
    public float BottomWidth = 4f;
    public float TopWidth    = 1.5f;
    public float Height      = 3f;

    [Header("Corner Y offsets")]
    public float X00 = 0f; // bottom-left
    public float X10 = 0f; // bottom-right
    public float X01 = 0f; // top-left
    public float X11 = 0f; // top-right

    [Header("Per-corner W")]
    public float W00 = 1f;
    public float W10 = 1f;
    public float W01 = 1f;
    public float W11 = 1f;

    void Update() { BuildTrapezoid(); }

    [ContextMenu("Build Trapezoid")]
    public void BuildTrapezoid()
    {
        Vector2 Q00 = new Vector2(-BottomWidth * 0.5f+ X00, 0f    );
        Vector2 Q10 = new Vector2( BottomWidth * 0.5f+ X10, 0f    );
        Vector2 Q01 = new Vector2(-TopWidth   * 0.5f+ X01,  Height );
        Vector2 Q11 = new Vector2( TopWidth   * 0.5f+ X11,  Height );

        Vector4[] uvqs = new Vector4[]
        {
            new Vector4(0f * W00, 0f * W00, 0f, W00),
            new Vector4(1f * W10, 0f * W10, 0f, W10),
            new Vector4(0f * W01, 1f * W01, 0f, W01),
            new Vector4(1f * W11, 1f * W11, 0f, W11),
        };

        Vector3[] verts = new Vector3[]
        {
            new Vector3(Q00.x, Q00.y, 0),
            new Vector3(Q10.x, Q10.y, 0),
            new Vector3(Q01.x, Q01.y, 0),
            new Vector3(Q11.x, Q11.y, 0),
        };

        int[] tris = new int[] { 0, 1, 3,  0, 3, 2 };

        Mesh mesh = new Mesh();
        mesh.name = "Trapezoid";
        mesh.vertices = verts;
        mesh.triangles = tris;
        mesh.SetUVs(0, new List<Vector4>(uvqs));
        mesh.RecalculateNormals();
        mesh.RecalculateBounds();

        GetComponent<MeshFilter>().sharedMesh = mesh;
    }
}