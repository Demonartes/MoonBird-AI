#define _CRT_SECURE_NO_WARNINGS
#include <graphics.h>
#include <windows.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Msimg32.lib")

#define WIN_W 640
#define WIN_H 480

#define BIRD_W 40
#define BIRD_H 30

#define PIPE_COUNT 3
#define PIPE_W 80
#define PIPE_GAP 150
#define PIPE_SPEED 3

#define GAME_MENU 0
#define GAME_PLAYING 1
#define GAME_OVER 2

#define AI_MODE 1

#define STATE_COUNT 18
#define ACTION_COUNT 2

typedef struct
{
	int x;
	int y;
	float vy;
	float gravity;
} Bird;

typedef struct
{
	int x;
	int gapY;
	int passed;
} Pipe;

IMAGE bg;
IMAGE birdImg;
IMAGE pipeTopImg;
IMAGE pipeBottomImg;

Pipe pipes[PIPE_COUNT];

int gameState = GAME_MENU;

int score = 0;
int bestScore = 0;
int episode = 0;

int pipeSpeed = PIPE_SPEED;

float Q[STATE_COUNT][ACTION_COUNT];
float alpha = 0.1f;
float gammaValue = 0.9f;
float epsilon = 0.1f;
float minEpsilon = 0.01f;

int lastState = 0;
int lastAction = 0;
int jumpCooldown = 0;

int pipeImgW;
int pipeImgH;

int birdImgW;
int birdImgH;

void InitBird(Bird* bird)
{
	bird->x = 120;
	bird->y = WIN_H / 2;
	bird->vy = 0.0f;
	bird->gravity = 0.35f;
}

void DrawPNG(int x, int y, IMAGE* img, int w, int h)
{
	HDC dst = GetImageHDC(NULL);
	HDC src = GetImageHDC(img);

	BLENDFUNCTION bf;
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = 255;
	bf.AlphaFormat = AC_SRC_ALPHA;

	AlphaBlend(dst, x, y, w, h, src, 0, 0, w, h, bf);
}

void UpdateBird(Bird* bird)
{
	bird->vy += bird->gravity;
	bird->y += (int)bird->vy;
}

void DrawBird(Bird* bird)
{
	DrawPNG((int)bird->x, (int)bird->y, &birdImg, BIRD_W, BIRD_H);
}

void CheckBirdBoundary(Bird* bird)
{
	if (bird->y < 15)
	{
		bird->y = 15;
		bird->vy = 0;
	}

	if (bird->y > WIN_H - 15)
	{
		bird->y = WIN_H - 15;
		bird->vy = 0;
	}
}

void ProcessInput(Bird* bird)
{
	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	{
		bird->vy = -7.5f;

		PlaySound(_T("assets/sounds/flap.wav"), NULL, SND_FILENAME | SND_ASYNC);
	}
}

void DrawScore()
{
	TCHAR text[100];
	_stprintf(text, _T("Score:%d Best:%d"), score, bestScore);

	settextcolor(WHITE);
	settextstyle(32, 0, _T("Consolas"));
	outtextxy(20, 20, text);
}

int GetNextPipeIndex()
{
	int nextIndex = -1;
	int minDistance = WIN_W + 1000;

	for (int i = 0; i < PIPE_COUNT; i++)
	{
		int distance = pipes[i].x + PIPE_W - 120;

		if (distance >= 0 && distance < minDistance)
		{
			minDistance = distance;
			nextIndex = i;
		}
	}

	return nextIndex;
}

int GetState(Bird* bird)
{
	int index = GetNextPipeIndex();

	if (index == -1)
	{
		return 2;
	}

	int gapCenter = pipes[index].gapY;
	int diff = bird->y - gapCenter;

	int positionState;

	if (diff < -80)
		positionState = 0;
	else if (diff < -30)
		positionState = 1;
	else if (diff < 0)
		positionState = 2;
	else if (diff < 30)
		positionState = 3;
	else if (diff < 80)
		positionState = 4;
	else
		positionState = 5;

	int velocityState;

	if (bird->vy < -2)
		velocityState = 0;
	else if (bird->vy < 2)
		velocityState = 1;
	else
		velocityState = 2;

	return positionState * 3 + velocityState;
}

void DrawAIInfo(Bird* bird)
{
	int index = GetNextPipeIndex();

	if (index == -1)
		return;

	int gapCenter = pipes[index].gapY;
	int diff = bird->y - gapCenter;
	int state = GetState(bird);

	TCHAR text[100];

	settextcolor(WHITE);
	settextstyle(18, 0, _T("Consolas"));
	outtextxy(20, 65, text);
}

void DrawPipes()
{
	for (int i = 0; i < PIPE_COUNT; i++)
	{
		int topH = pipes[i].gapY - PIPE_GAP / 2;

		int bottomY = pipes[i].gapY + PIPE_GAP / 2;

		DrawPNG(pipes[i].x, topH - pipeImgH, &pipeTopImg, pipeImgW, pipeImgH);

		DrawPNG(pipes[i].x, bottomY, &pipeBottomImg, pipeImgW, pipeImgH);
	}
}

void RenderGame(Bird* bird)
{
	setbkcolor(RGB(135, 206, 235));
	cleardevice();

	putimage(0, 0, &bg);
	DrawPipes();
	DrawBird(bird);
	DrawScore();

	FlushBatchDraw();
}

void InitGraphics()
{
	initgraph(WIN_W, WIN_H);

	loadimage(&bg, _T("assets/images/background.png"), WIN_W, WIN_H);
	loadimage(&birdImg, _T("assets/images/bird.png"), BIRD_W, BIRD_H, true);
	birdImgW = BIRD_W;
	birdImgH = BIRD_H;
	loadimage(&pipeTopImg, _T("assets/images/pipeup.png"), 80, 300, true);
	loadimage(&pipeBottomImg, _T("assets/images/pipedown.png"), 80, 300, true);

	pipeImgW = pipeTopImg.getwidth();
	pipeImgH = pipeTopImg.getheight();

	BeginBatchDraw();
}

void InitPipes()
{
	for (int i = 0; i < PIPE_COUNT; i++)
	{
		pipes[i].x = WIN_W + i * 220;
		pipes[i].gapY = 120 + rand() % 220;
		pipes[i].passed = 0;
	}
}

bool CheckCollision(Bird* bird)
{
	int birdLeft = bird->x + 10;
	int birdRight = bird->x + BIRD_W - 10;

	int birdTop = bird->y + 6;
	int birdBottom = bird->y + BIRD_H - 6;

	setlinecolor(RED);

	rectangle(
		birdLeft,
		birdTop,
		birdRight,
		birdBottom
	);

	if (birdTop <= 0 || birdBottom >= WIN_H)
	{
		return true;
	}

	for (int i = 0; i < PIPE_COUNT; i++)
	{
		int pipeLeft = pipes[i].x + 10;
		int pipeRight = pipes[i].x + pipeImgW - 10;

		int topPipeBottom = pipes[i].gapY - PIPE_GAP / 2;
		int bottomPipeTop = pipes[i].gapY + PIPE_GAP / 2;

		setlinecolor(GREEN);

		rectangle(
			pipeLeft,
			0,
			pipeRight,
			topPipeBottom
		);

		rectangle(
			pipeLeft,
			bottomPipeTop,
			pipeRight,
			WIN_H
		);
		if (birdRight > pipeLeft && birdLeft < pipeRight)
		{
			if (birdTop < topPipeBottom || birdBottom > bottomPipeTop)
			{
				return true;
			}
		}
	}
	return false;
}

void DrawGameOver()
{
	setfillcolor(RGB(20, 20, 20));
	solidrectangle(120, 120, 520, 340);

	settextcolor(RED);
	settextstyle(60, 0, _T("Consolas"));
	outtextxy(140, 140, _T("GAME OVER"));

	TCHAR scoreText[100];

	_stprintf(
		scoreText,
		_T("Score: %d"),
		score
	);

	settextcolor(BLACK);
	settextstyle(32, 0, _T("Consolas"));
	outtextxy(200, 220, scoreText);

	settextcolor(BLACK);
	settextstyle(20, 0, _T("Consolas"));

	outtextxy(180, 280, _T("Press R to restart"));
	outtextxy(180, 310, _T("Press ESC to menu"));
}

void DrawMenu()
{
	setbkcolor(RGB(135, 206, 235));
	cleardevice();

	putimage(0, 0, &bg);

	settextcolor(YELLOW);
	settextstyle(42, 0, _T("Consolas"));
	outtextxy(120, 130, _T("MOON BIRD"));

	settextcolor(WHITE);
	settextstyle(22, 0, _T("Consolas"));
	outtextxy(185, 220, _T("Press SPACE to start"));
	outtextxy(215, 260, _T("Press ESC to exit"));

#if AI_MODE
	outtextxy(220, 300, _T("AI learns automatically"));
#else
	outtextxy(170, 300, _T("Fly through the pipes"));
#endif

	FlushBatchDraw();
}

void SaveQTable()
{
	FILE* fp = fopen("qtable.txt", "w");

	if (fp == NULL)
	{
		return;
	}

	fprintf(fp, "%d\n", bestScore);
	fprintf(fp, "%f\n", epsilon);

	for (int s = 0; s < STATE_COUNT; s++)
	{
		for (int a = 0; a < ACTION_COUNT; a++)
		{
			fprintf(fp, "%f ", Q[s][a]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}

void UpdateEpsilon()
{
	if (epsilon > minEpsilon)
	{
		epsilon *= 0.995f;
	}

	if (epsilon < minEpsilon)
	{
		epsilon = minEpsilon;
	}
}

void StartGame(Bird* bird)
{
	InitBird(bird);
	InitPipes();

	score = 0;
	pipeSpeed = PIPE_SPEED;
	jumpCooldown = 0;

#if AI_MODE
	episode++;
	UpdateEpsilon();

	if (episode % 20 == 0)
	{
		SaveQTable();
	}
#endif
	gameState = GAME_PLAYING;
}

void UpdateDifficulty()
{
	pipeSpeed = PIPE_SPEED + score / 5;

	if (pipeSpeed > 6)
	{
		pipeSpeed = 6;
	}
}

int ChooseAction(int state)
{
	int randomValue = rand() % 100;

	if (randomValue < epsilon * 100)
	{
		return rand() % ACTION_COUNT;
	}

	if (Q[state][0] > Q[state][1])
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int UpdatePipes()
{
	int passedPipe = 0;

	for (int i = 0; i < PIPE_COUNT; i++)
	{
		pipes[i].x -= pipeSpeed;

		if (pipes[i].passed == 0 && pipes[i].x + PIPE_W < 120)
		{
			score++;
			pipes[i].passed = 1;
			passedPipe = 1;
		}

		if (pipes[i].x + PIPE_W < 0)
		{
			pipes[i].x = WIN_W;
			pipes[i].gapY = 120 + rand() % 220;
			pipes[i].passed = 0;
		}
	}

	return passedPipe;
}

void ApplyAction(Bird* bird, int action)
{
	if (jumpCooldown > 0)
	{
		jumpCooldown--;
	}

	if (action == 1 && jumpCooldown == 0)
	{
		bird->vy = -7.5f;
		jumpCooldown = 8;

#if !AI_MODE
		PlaySound(_T("flap.wav"), NULL, SND_FILENAME | SND_ASYNC);
#endif
	}
}

void AIControl(Bird* bird)
{
	lastState = GetState(bird);
	lastAction = ChooseAction(lastState);

	ApplyAction(bird, lastAction);
}

void InitQTable()
{
	for (int s = 0; s < STATE_COUNT; s++)
	{
		for (int a = 0; a < ACTION_COUNT; a++)
		{
			Q[s][a] = 0.0f;
		}
	}
}

float GetMaxQ(int state)
{
	if (Q[state][0] > Q[state][1])
	{
		return Q[state][0];
	}
	else
	{
		return Q[state][1];
	}
}

void UpdateQ(int state, int action, float reward, int nextState)
{
	Q[state][action] = Q[state][action] + alpha *
		(reward + gammaValue * GetMaxQ(nextState) - Q[state][action]);
}

void LoadQTable()
{
	FILE* fp = fopen("qtable.txt", "r");

	if (fp == NULL)
	{
		InitQTable();
		bestScore = 0;
		return;
	}

	fscanf(fp, "%d", &bestScore);
	fscanf(fp, "%f", &epsilon);

	for (int s = 0; s < STATE_COUNT; s++)
	{
		for (int a = 0; a < ACTION_COUNT; a++)
		{
			fscanf(fp, "%f", &Q[s][a]);
		}
	}

	fclose(fp);
}

int main()
{
	Bird bird;

	InitBird(&bird);

	InitGraphics();

	srand((unsigned int)time(NULL));
	LoadQTable();
	InitPipes();

	while (1)
	{
		if (gameState == GAME_MENU)
		{
			DrawMenu();

			if (GetAsyncKeyState(VK_SPACE) & 0x8000)
			{
				StartGame(&bird);
			}
			if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
			{
				break;
			}
		}
		else if (gameState == GAME_PLAYING)
		{
#if AI_MODE
			AIControl(&bird);
#else
			ProcessInput(&bird);
#endif

			UpdateBird(&bird);

			UpdateDifficulty();

			int passedPipe = UpdatePipes();

			CheckBirdBoundary(&bird);

			if (CheckCollision(&bird))
			{
				UpdateQ(lastState, lastAction, -100.0f, GetState(&bird));
				
				if (score > bestScore)
				{
					bestScore = score;
				}

				SaveQTable();

#if AI_MODE
				StartGame(&bird);
				continue;
#else
				gameState = GAME_OVER;
#endif
			}
			else
			{
				float reward = 1.0f;

				if (passedPipe == 1)
				{
					reward = 20.0f;
				}

				UpdateQ(lastState, lastAction, reward, GetState(&bird));
			}
			RenderGame(&bird);
		}
		else if (gameState == GAME_OVER)
		{
			RenderGame(&bird);
			DrawGameOver();
			FlushBatchDraw();

			if (GetAsyncKeyState('R') & 0x8000)
			{
				StartGame(&bird);
			}

			if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
			{
				gameState = GAME_MENU;
				Sleep(200);
			}
		}
		Sleep(16);
	}
	SaveQTable();
	EndBatchDraw();

	closegraph();

	return 0;
}