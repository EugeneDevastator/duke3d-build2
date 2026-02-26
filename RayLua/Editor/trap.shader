Shader "Unlit/TrapezoidProjective"
{
    Properties
    {
        _MainTex ("Texture", 2D) = "white" {}
    }
    SubShader
    {
        Tags { "RenderType"="Opaque" }
        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            #include "UnityCG.cginc"

            sampler2D _MainTex;

            struct appdata
            {
                float4 vertex : POSITION;
                float4 uvq    : TEXCOORD0;
            };

            struct v2f
            {
                float4 pos : SV_POSITION;
                float4 uvq : TEXCOORD0;
            };

            v2f vert(appdata v)
            {
                v2f o;
                o.pos = UnityObjectToClipPos(v.vertex);
                o.uvq = v.uvq;
                return o;
            }

            fixed4 frag(v2f i) : SV_Target
            {
            i.uvq.x*=2;
            i.uvq.y*=2;
                return tex2Dproj(_MainTex, i.uvq);
            }
            ENDCG
        }
    }
}
