#ifndef SOTN_DOPPLE_BODY_INTERNAL_H
#define SOTN_DOPPLE_BODY_INTERNAL_H
#include <SDL3/SDL.h>
#include <sotn/dopple_body.h>

typedef struct DoppleFrame
{
    SotnDoppleBody body;
    SotnDataView overlay;
    Uint8 knives;
    bool failed;
} DoppleFrame;

void DoppleSetStep(DoppleFrame *frame, Uint16 step);
void DoppleSetAnimation(DoppleFrame *frame, Uint16 id);
Sint32 DoppleDirection(DoppleFrame *frame);
void DoppleSpeed(DoppleFrame *frame, Sint32 speed);
void DoppleDecelerate(DoppleFrame *frame, Sint32 amount);
void DoppleAfterImage(DoppleFrame *frame, bool reset, Uint16 ticks);
void DoppleCancelAttack(DoppleFrame *frame);
void DoppleCrouch(DoppleFrame *frame, Uint16 index, Sint32 speed);
void DoppleStand(DoppleFrame *frame, Sint32 speed);
void DoppleWalk(DoppleFrame *frame, bool force);
void DoppleJump(DoppleFrame *frame);
void DoppleDoubleJump(DoppleFrame *frame);
void DoppleFall(DoppleFrame *frame);
void DoppleBackdash(DoppleFrame *frame);
bool DoppleBranches(DoppleFrame *frame, Uint32 branches);
void DoppleNormal(DoppleFrame *frame);
bool DoppleAttack(DoppleFrame *frame);
Uint8 DoppleByte(DoppleFrame *frame, size_t offset);
Sint16 DoppleSigned16(DoppleFrame *frame, size_t offset);
void DoppleRequest(DoppleFrame *frame, Uint16 kind, Uint16 id, Uint16 parameter);
#endif
