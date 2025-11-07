//=====================================================================
//
// Player [player.cpp]
// Author : 
// 
//=====================================================================

//*********************************************************************
// 
// ***** インクルードファイル *****
// 
//*********************************************************************
#include "player.h"
#include "baseObject.h"
#include "input.h"
#include "util.h"
#include "camera.h"
#include "shadow.h"

//*********************************************************************
// 
// ***** マクロ定義 *****
// 
//*********************************************************************
#define MAX_TEXTURE		(8)

//*********************************************************************
// 
// ***** 構造体 *****
// 
//*********************************************************************


//*********************************************************************
// 
// ***** 列挙型 *****
// 
//*********************************************************************


//*********************************************************************
// 
// ***** プロトタイプ宣言 *****
// 
//*********************************************************************


//*********************************************************************
// 
// ***** グローバル変数 *****
// 
//*********************************************************************
LPD3DXMESH g_pMeshPlayer = NULL;			// メッシュ（頂点情報）へのポインタ
LPDIRECT3DTEXTURE9 g_apTexturePlayer[MAX_TEXTURE] = {};
LPD3DXBUFFER g_pBuffMatPlayer = NULL;	// マテリアルへのポインタ
DWORD g_dwNumMatPlayer = 0;				// マテリアル数
BASEOBJECT g_objPlayer;
D3DXMATRIX g_mtxWorldPlayer;	// ワールドマトリックス
int g_nIdxShadowPlayer = -1;

//=====================================================================
// 初期化処理
//=====================================================================
void InitPlayer(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();

	memset(&g_objPlayer, 0, sizeof(BASEOBJECT));
	g_objPlayer.bVisible = true;

	// Xファイルの読み込み
	D3DXLoadMeshFromX(
		"data\\MODEL\\SmileMan.x",
		D3DXMESH_SYSTEMMEM,
		pDevice,
		NULL,
		&g_pBuffMatPlayer,
		NULL,
		&g_dwNumMatPlayer,
		&g_pMeshPlayer
	);

	D3DXMATERIAL* pMat;

	// マテリアルデータへのポインタを取得
	pMat = (D3DXMATERIAL*)g_pBuffMatPlayer->GetBufferPointer();

	// テクスチャの読み込み
	for (int i = 0; i < (int)g_dwNumMatPlayer; i++)
	{
		if (pMat[i].pTextureFilename != NULL)
		{// テクスチャファイルが存在する
			D3DXCreateTextureFromFile(
				pDevice,
				pMat[i].pTextureFilename,
				&g_apTexturePlayer[i]
			);
		}
	}

	g_nIdxShadowPlayer = SetShadow();
}

//=====================================================================
// 終了処理
//=====================================================================
void UninitPlayer(void)
{
	for (int i = 0; i < (int)g_dwNumMatPlayer; i++)
	{
		if (g_apTexturePlayer[i] != NULL)
		{
			g_apTexturePlayer[i]->Release();
			g_apTexturePlayer[i] = NULL;
		}
	}

	if (g_pMeshPlayer != NULL)
	{// メッシュの破棄
		g_pMeshPlayer->Release();
		g_pMeshPlayer = NULL;
	}

	if (g_pBuffMatPlayer != NULL)
	{// マテリアルの破棄
		g_pBuffMatPlayer->Release();
		g_pBuffMatPlayer = NULL;
	}
}

//=====================================================================
// 更新処理
//=====================================================================
void UpdatePlayer(void)
{
	CAMERA* pCamera = GetCamera();

	static D3DXVECTOR3 move = D3DXVECTOR3_ZERO;
	D3DXVECTOR3 dir = D3DXVECTOR3_ZERO;

	if (GetKeyboardPress(DIK_LEFT))
	{
		dir -= D3DXVECTOR3(cosf(pCamera->rot.y), 0.0f, -sinf(pCamera->rot.y)) * 5.0f;
		g_objPlayer.rot.y = D3DX_PI * 0.5f + pCamera->rot.y;
	}
	if (GetKeyboardPress(DIK_RIGHT))
	{
		dir += D3DXVECTOR3(cosf(pCamera->rot.y), 0.0f, -sinf(pCamera->rot.y)) * 5.0f;
		g_objPlayer.rot.y = -D3DX_PI * 0.5f + pCamera->rot.y;
	}
	if (GetKeyboardPress(DIK_UP))
	{
		dir += D3DXVECTOR3(sinf(pCamera->rot.y), 0.0f, cosf(pCamera->rot.y)) * 5.0f;
		g_objPlayer.rot.y = D3DX_PI + pCamera->rot.y;
	}
	if (GetKeyboardPress(DIK_DOWN))
	{
		dir -= D3DXVECTOR3(sinf(pCamera->rot.y), 0.0f, cosf(pCamera->rot.y)) * 5.0f;
		g_objPlayer.rot.y = 0 + pCamera->rot.y;
	}

	if (GetKeyboardTrigger(DIK_SPACE))
	{
		move.y = 10.0f;
	}

	g_objPlayer.pos += dir + move;
	move.y -= 0.6f;
	Clampf(&g_objPlayer.pos.y, 0.0f, g_objPlayer.pos.y);

	SetShadowPosition(g_nIdxShadowPlayer, D3DXVECTOR3(g_objPlayer.pos.x, 0.0f, g_objPlayer.pos.z));
	SetShadowSize(g_nIdxShadowPlayer, D3DXVECTOR3(100.0f, 0.01f, 100.0f) * (g_objPlayer.pos.y + 1) * 0.0025f);
	SetShadowAlpha(g_nIdxShadowPlayer, 1.0f);
}

//=====================================================================
// 描画処理
//=====================================================================
void DrawPlayer(void)
{
	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	D3DXMATRIX mtxRot, mtxTrans;	// 計算用マトリックス
	D3DMATERIAL9 matDef;			// 現在のマテリアル保存用
	D3DXMATERIAL* pMat;				// マテリアルデータへのポインタ

	// ワールドマトリックスの初期化
	D3DXMatrixIdentity(&g_mtxWorldPlayer);

	// 向きを反映
	D3DXMatrixRotationYawPitchRoll(
		&mtxRot,
		g_objPlayer.rot.y, g_objPlayer.rot.x, g_objPlayer.rot.z
	);
	D3DXMatrixMultiply(
		&g_mtxWorldPlayer,
		&g_mtxWorldPlayer,
		&mtxRot
	);

	// 位置を反映
	D3DXMatrixTranslation(
		&mtxTrans,
		g_objPlayer.pos.x, g_objPlayer.pos.y, g_objPlayer.pos.z
	);
	D3DXMatrixMultiply(
		&g_mtxWorldPlayer,
		&g_mtxWorldPlayer,
		&mtxTrans
	);

	// ワールドマトリックスの設定
	pDevice->SetTransform(D3DTS_WORLD, &g_mtxWorldPlayer);

	// 現在のマテリアルを取得
	pDevice->GetMaterial(&matDef);

	// マテリアルデータへのポインタを取得
	pMat = (D3DXMATERIAL*)g_pBuffMatPlayer->GetBufferPointer();

	for (int nCntMat = 0; nCntMat < (int)g_dwNumMatPlayer; nCntMat++)
	{
		// マテリアルの設定
		pDevice->SetMaterial(&pMat[nCntMat].MatD3D);

		// テクスチャの設定
		pDevice->SetTexture(0, g_apTexturePlayer[nCntMat]);

		// モデル（パーツ）の描画
		g_pMeshPlayer->DrawSubset(nCntMat);
	}

	// 保存していたマテリアルを戻す
	pDevice->SetMaterial(&matDef);
}