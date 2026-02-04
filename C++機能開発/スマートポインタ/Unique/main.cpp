///////////////////////////////////////////
//main.cpp                               
//                                       
//  プログラム               
//                                         
//  ～概要～                             
//      
//                                       
//  ～更新履歴～                         
//  yyyy/mm/dd 制作開始 -Nakajima Yuhi-  
//   
//                                       
///////////////////////////////////////////

//===== インクルード =====
#include <iostream>
#include <memory>

//===== 名前空間宣言 =====
using namespace std;

//===== 定数・マクロ定義 =====

//===== 構造体定義 =====

//===== グローバル変数宣言 =====

//===== プロトタイプ宣言 =====

/// //////////////////////////
//main関数                  
//                          
//概　要:エントリーポイント 
//引　数:なし               
//戻り値:0                  
//////////////////////////////
int main() 
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	//MakeUniqueで初期化
	std::unique_ptr<int> pData = std::make_unique<int>(10);

	//値のコピーはできる
	int i = *pData;

	//参照を持つことができる
	//&を付ける     *を付ける(値と同じ渡し方)
	int& pData2 = *pData;

	//後で値を入れても...
	*pData = 15;
	
	//反映されてる
	cout << "pData2:" << pData2 << endl;



	rewind(stdin);
	getchar();
	return 0;
}

//stringの初期化
//new string("text")
//new string{"text"}