float4 VS(uint vertexID : SV_VertexID) : SV_Position
{
    float4 triangle_position[] =
    {
        float4(-0.0f, 0.5f, 0.0F, 1.0f),
        float4(0.5f, -0.5f, 0.0f, 1.0f),
        float4(-0.5f, -0.5f, 0.0f, 1.0f)
    };
    
    return triangle_position[vertexID];
}