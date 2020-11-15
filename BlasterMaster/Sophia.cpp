#include "Sophia.h"
#include "TileArea.h"

CSophia::CSophia(int classId, int x, int y, int animsId) : CAnimatableObject::CAnimatableObject(classId, x, y, animsId)
{
	SetState(SOPHIA_STATE_IDLE3_RIGHT);
	vyMax = 100;
	vxMax = SOPHIA_MAX_SPEED;
};

void CSophia::updateWheel() {
	if (abs(vx) > 0.001 && GetTickCount() - lastTimeMoveWheel >= 300 - 2500 * abs(vx)) {
		wheel = (wheel + 1) % 4;
		lastTimeMoveWheel = GetTickCount();
	}
}

void CSophia::setIdleRight() {
	updateWheel();
	SetState(idleRightStates[3-wheel]);
}

void CSophia::setIdleLeft() {
	updateWheel();
	SetState(idleLeftStates[3-wheel]);
}

void CSophia::setTurnRight() {
	SetState(gunTurnRight[3 - wheel]);
}

void CSophia::setTurnLeft() {
	SetState(gunTurnLeft[3 - wheel]);
}

void CSophia::setJumpRight() {
	updateWheel();
	SetState(jumpRight[3 - wheel]);
}

void CSophia::setJumpLeft() {
	updateWheel();
	SetState(jumpLeft[3 - wheel]);
}

void CSophia::setFallLeft() {
	updateWheel();
	SetState(fallLeft[3 - wheel]);
}

void CSophia::setFallRight() {
	updateWheel();
	SetState(fallRight[3 - wheel]);
}

#pragma region key events handling

void CSophia::HandleKeys(DWORD dt)
{
	HandleKeysHold(dt);

	auto keyEvents = NewKeyEvents();
	for (auto e : keyEvents)
	{
		int keyCode = e->GetKeyCode();
		if (e->IsDown())
			HandleKeyDown(dt, keyCode);
		else
			HandleKeyUp(dt, keyCode);
	}
}

void CSophia::HandleKeysHold(DWORD dt)
{
	if (IsKeyDown(DIK_RIGHT))
	{
		if (isLeft)
			doneTurn = false;
		isLeft = false;
		if (!doneTurn) {
			setTurnRight();
			if (animationHandlers[state]->currentFrameIndex == animationHandlers[state]->animation->GetNumberOfFrames() - 1) {
				doneTurn = true;
			}
		}
		if (doneTurn) {
			animationHandlers[state]->currentFrameIndex = 0;
			setIdleRight();
		}
		ax = 0.001;
		
	}
	else if (IsKeyDown(DIK_LEFT))
	{
		if (!isLeft)
			doneTurn = false;
		isLeft = true;
		if (!doneTurn) {
			setTurnLeft();
			if (animationHandlers[state]->currentFrameIndex == animationHandlers[state]->animation->GetNumberOfFrames() - 1) {
				doneTurn = true;

			}
		}
		if (doneTurn) {
			animationHandlers[state]->currentFrameIndex = 0;
			setIdleLeft();
		}
		ax = -0.001;

	}
	if (IsKeyDown(DIK_UP) || IsKeyDown(DIK_UP) && IsKeyDown(DIK_RIGHT))
	{
		if (!isLeft && state != SOPHIA_STATE_GUNUP_RIGHT)
		{
			SetState(SOPHIA_STATE_GUNUP_RIGHT);
			animationHandlers[state]->currentFrameIndex = 0;
		}
		if (isLeft && state != SOPHIA_STATE_GUNUP_LEFT)
		{
			SetState(SOPHIA_STATE_GUNUP_LEFT);
			animationHandlers[state]->currentFrameIndex = 0;
		}
	}
	if (IsKeyDown(DIK_DOWN))
	{
	}
}
void CSophia::HandleKeyUp(DWORD dt, int keyCode)
{
	if (keyCode == DIK_RIGHT) {
		flagStop = true;
		stopLeft = false;
	}
	if (keyCode == DIK_LEFT) {
		flagStop = true;
		stopLeft = true;
	}

	if (keyCode == DIK_UP || keyCode == DIK_DOWN)
		vy = 0;
	if (keyCode == DIK_UP) {
		if (!isLeft) {
			SetState(SOPHIA_STATE_GUNDOWN_RIGHT);
			animationHandlers[state]->currentFrameIndex = 0;
		}
		if (isLeft) {
			SetState(SOPHIA_STATE_GUNDOWN_LEFT);
			animationHandlers[state]->currentFrameIndex = 0;
		}
	}
	/*if (keyCode == DIK_X)
	{
		if (isLeft)
			setFallLeft();
		else
			setFallRight();
	}*/
}

void CSophia::HandleKeyDown(DWORD dt, int keyCode)
{
	if (!flagOnAir && keyCode == DIK_X)
	{
		/*if (!isLeft) {
			setJumpRight();
		}
		else
			setJumpLeft();*/
		y -= 1;
		vy = 0;
		ay = -0.018;
		//vy = 0.5;
		flagOnAir = true;
	}
}

void CSophia::HandleCollision(DWORD dt, LPCOLLISIONEVENT coEvent)
{
	if (coEvent == nullptr)
		return;
	if (coEvent->otherObject == this)
		return;

	LPGAMEOBJECT obj = coEvent->otherObject;

	if (dynamic_cast<LPTILE_AREA>(obj))
	{
		LPTILE_AREA tileArea = dynamic_cast<LPTILE_AREA>(obj);

		switch (tileArea->GetClass())
		{
		case CLASS_TILE_BLOCKABLE:
		{
			// on top of a blockable tile
			if (coEvent->ny < 0)
			{
				flagOnAir = false;
			}
			break;
		}
		}
	}
}

void CSophia::Update(DWORD dt, vector<LPGAMEOBJECT>* coObjs)
{
	CGameObject::Update(dt, coObjs);
	HandleKeys(dt);

	// Simple fall down
	vy += SOPHIA_GRAVITY;
	//SolveClassicalMechanics();
	float dx, dy;
	GetPositionDifference(dx, dy);

	vector<LPCOLLISIONEVENT> coEvents;
	vector<LPCOLLISIONEVENT> coEventsResult;
	coEvents.clear();

	// turn off collision when die 
	if (!flagDead)
		CalcPotentialCollisions(coObjs, coEvents);
	if (flagStop && !stopLeft)
		if (vx > 0) {
			vx -= 0.005;
			if (doneTurn)
			setIdleRight();
		}
		else {
			vx = 0;
			flagStop = false;
		}
	if (flagStop && stopLeft)
		if (vx < 0) {
			vx += 0.005;
			if (doneTurn)
			setIdleLeft();
		}
		else {
			vx = 0;
			flagStop = false;
		}

	// No collision occured, proceed normally
	if (coEvents.size() == 0)
	{
		//x += dx;
		//y += dy;
		SolveClassicalMechanics();
	}
	else
	{
		float min_tx, min_ty, nx = 0, ny;
		float rdx = 0;
		float rdy = 0;

		// TODO: This is a very ugly designed function!!!!
		FilterCollision(coEvents, coEventsResult, min_tx, min_ty, nx, ny, rdx, rdy);
		// block every object first!
		// CuteTN Note: wth is 0.4f??? WHAT IS IT?
		x += min_tx * dx + nx * 0.4f;
		y += min_ty * dy + ny * 0.4f;
		if (nx != 0) vx = 0;
		if (ny != 0) vy = 0;

		// collision logic
		for (LPCOLLISIONEVENT coEvent : coEvents)
			HandleCollision(dt, coEvent);

	}

}

void CSophia::GetBoundingBox(float& left, float& top, float& right, float& bottom)
{
	left = x + SOPHIA_BOUNDBOX_OFFSETX;
	top = y + SOPHIA_BOUNDBOX_OFFSETY;
	right = left + SOPHIA_BOUNDBOX_WIDTH;
	bottom = top + SOPHIA_BOUNDBOX_HEIGHT;
}
