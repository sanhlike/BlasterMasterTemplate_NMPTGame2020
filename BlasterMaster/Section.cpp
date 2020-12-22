#include "Section.h"
#include "TileArea.h"
#include "Sophia.h"
#include "CollisionSolver.h"
#include "JasonSideview.h"
#include "GameObjectBehaviour.h"

CSection::CSection(int bgTextureId, int fgTextureId)
{
	this->backgroundTextureId = bgTextureId;
	this->foregroundTextureId = fgTextureId;

	LPDIRECT3DTEXTURE9 backgroundTexture = CTextures::GetInstance()->Get(backgroundTextureId);

	// getting the size of texture
	D3DSURFACE_DESC surfaceDesc;
	backgroundTexture->GetLevelDesc(0, &surfaceDesc);

	bgWidth = surfaceDesc.Width;
	bgHeight = surfaceDesc.Height;
}



void CSection::Update(DWORD dt)
{
	// CuteTN to do: filter out the neccessary objects to update

	// collision objects for static object, i.e. tiles
	//vector<LPGAMEOBJECT> coObjs_static;
	//coObjs_static.clear();
	int cnt = 0;

	for (auto obj : Objects)
	{
		if (!obj)
			continue;

		if (dynamic_cast<LPTILE_AREA>(obj))
			obj->Update(dt, nullptr);
		else
		{
			if (obj->isUpdatedWhenOffScreen || checkObjInCamera(obj, SCREEN_EXTEND_OFFSET_DEFAULT))
			{
				obj->Update(dt, &Objects);
			}
		}
	}
}

//SANH-CAMERA:
void CSection::Render(float offset_x, float offset_y)
{
	// CuteTN Note: the order of rendering would be implemented here :)
	RenderTexture(backgroundTextureId, offset_x, offset_y);

	for (auto obj : Objects)
	{
		if (checkObjInCamera(obj, SCREEN_EXTEND_OFFSET_DEFAULT))
			if(obj->isHiddenByForeground)
				obj->Render(offset_x, offset_y);
	}

	RenderTexture(foregroundTextureId, offset_x, offset_y);

	for (auto obj : Objects)
	{
		if (checkObjInCamera(obj, SCREEN_EXTEND_OFFSET_DEFAULT))
			if (!obj->isHiddenByForeground)
				obj->Render(offset_x, offset_y);
	}
}

void CSection::RenderTexture(int textureId, float offset_x, float offset_y)
{
	LPDIRECT3DTEXTURE9 backgroundTexture = CTextures::GetInstance()->Get(textureId);
	CGame::GetInstance()->Draw(offset_x, offset_y, backgroundTexture, 0, 0, bgWidth, bgHeight);
}


//SANH-CAMERA
void CSection::deleteSophia()
{
	int index = -1;
	for (int i=0; i<Objects.size(); i++)
		if (Objects[i]->classId == CLASS_SOPHIA)
		{
			index = i;
			break;
		}
	if (index == -1) return;
	Objects.erase(Objects.begin()+index);
}

void CSection::pushSophia(float x, float y, int sectionID)
{
	this->deleteSophia();
	CSophia::GetInstance()->SetPosition(x, y);
	CSophia::GetInstance()->currentSectionId = sectionID;
	Objects.push_back(CSophia::GetInstance());
}

void CSection::deleteJasonSideview()
{
	int index = -1;
	for (int i = 0; i < Objects.size(); i++)
		if (Objects[i]->classId == CLASS_JASONSIDEVIEW)
		{
			index = i;
			break;
		}
	if (index == -1)
		return;
	Objects.erase(Objects.begin() + index);
}

void CSection::pushJasonSideview(float x, float y, int sectionID)
{
	this->deleteJasonSideview();
	CJasonSideview::GetInstance()->SetPosition(x, y);
	CJasonSideview::GetInstance()->currentSectionId = sectionID;
	Objects.push_back(CJasonSideview::GetInstance());
}

void CSection::addObject(LPGAMEOBJECT obj)
{
	if (!obj)
		return;

	Objects.push_back(obj);
}

/// <summary>
/// CuteTN Note: Copied from Sanh's function :)
/// </summary>
/// <param name="obj"></param>
/// <param name="deleteAfterRemoving"></param>
void CSection::removeObject(LPGAMEOBJECT obj, bool deleteAfterRemoving)
{
	if (!obj)
		return;

	for (int i = 0; i < Objects.size(); i++)
		if (Objects[i] == obj)
		{
			if(deleteAfterRemoving)
				delete Objects[i];

			Objects.erase(Objects.begin() + i);
			break;
		}
}
