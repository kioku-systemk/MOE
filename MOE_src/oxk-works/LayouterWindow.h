#include "common.h"
#include "cdemo.h"


#define MODIFY_TRANSLATION		0
#define MODIFY_ROTATION			1
#define MODIFY_SCALING			2
#define MODIFY_SELECTION		3
#define MODIFY_ALPHA			4

void LayouterSelectClone(int);

void Render();
void LayouterKeyEvent(UCHAR, bool);
void LayouterMouseEvent(long, long, int, bool);
long GetFindAnimNumber(float select_keyframe, CAnimation* ani);
void InitLayouterWindow();

void ResetUndo();
void SetUndo();
void Undo();
void Redo();

void OnIdle();
void StartPlaying(const int mode);
void StopPlaying();
void UpdateFrameTransparency(float newTransparency);

long GetModifyMode();