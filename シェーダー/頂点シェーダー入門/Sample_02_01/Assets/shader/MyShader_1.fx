//今やったものを自作
//上手く出た！

//入力構造体
struct VS_INPUT
{
    float4 Pos : POSITION;
};

//出力構造体
struct VS_OUTPUT
{
    float4 Pos : SV_Position;
};

//頂点シェーダー
VS_OUTPUT VSMain(VS_INPUT In)
{
    
    //出力構造体の変数を宣言して、0で初期化する
    //0をVS_OUTPUT型にキャストしている
    //初期化のイディオムとして、構造体の変数を宣言して0で初期化する方法がある
    VS_OUTPUT Out = (VS_OUTPUT) 0;
    
    //入力された頂点座標を出力データに代入する
    //Out = Inはできないので、Out.Pos = In.PosとしてPositionだけを代入する
    Out.Pos = In.Pos;
    
    //値を返す
    return Out;
    
}

//ピクセルシェーダー
float4 PSMain(VS_OUTPUT In) : SV_Target0
{
    //赤色を返す
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}