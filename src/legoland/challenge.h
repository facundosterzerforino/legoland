#pragma once

#include "image_sprite.h"
#include "render.h"

/* One row of the appraisal report (RunAppraisal); 19 ints. */
struct AppraisalRow {
    /* 0x00 */ int page; /* report page the row is on */
    /* 0x04 */ int x; /* indent */
    /* 0x08 */ int type; /* check result (1 passed, 0 failed); -1 failed-check advice, -2 summary, -3 continuation line */
    /* 0x0c */ int rnd; /* rand() % 5, chooses the mark's picture */
    /* 0x10 */ char *text;
    /* 0x14 */ int arg;
    /* 0x18 */ int bar; /* draw a value bar */
    /* 0x1c */ int value;
    /* 0x20 */ int goal;
    /* 0x24 */ int max;
    /* 0x28 */ int nids; /* speech ids queued when the row is shown */
    /* 0x2c */ int ids[8];
};

struct LocFile;
struct AdvisorObject;

unsigned int FUN_00443e30(struct AdvisorObject *param_1);
void UpdateAppraisalPageButtons(void);
void FUN_004437d0(struct Image *param_1, struct TextureNode *param_2);
void FUN_00444070(unsigned int param_1, unsigned int param_2);
void FUN_004441f0(void);
void FUN_0044db20(void);
void StartAppraisalTimer(void);
void StopAppraisalTimer(void);
void FUN_0044dc70(unsigned int param_1, unsigned int param_2);
unsigned int FUN_00443710(void);
void LoadTextureBitmaps(struct LocFile *param_1, const char *param_2);
unsigned int FUN_004443b0(unsigned int param_1, unsigned int param_2);
unsigned int FUN_004443e0(unsigned int param_1, unsigned int param_2);
unsigned int FUN_00444410(unsigned int param_1, unsigned int param_2);
void FUN_00444440(unsigned int param_1, unsigned int param_2);
void FUN_00444470(unsigned int param_1, unsigned int param_2);
void FUN_004444b0(unsigned int param_1, unsigned int param_2);
void FUN_004444f0(unsigned int param_1, unsigned int param_2);
void FUN_00444530(unsigned int param_1, unsigned int param_2);
void FUN_00444570(unsigned int param_1, unsigned int param_2);
void FUN_004445b0(unsigned int param_1, unsigned int param_2);
void FUN_004445f0(unsigned int param_1, unsigned int param_2);
void FUN_00444630(unsigned int param_1, unsigned int param_2);
void FUN_00444670(unsigned int param_1, unsigned int param_2);
void FUN_004446b0(unsigned int param_1, unsigned int param_2);
void FUN_004446f0(unsigned int param_1, unsigned int param_2);
void FUN_00444730(unsigned int param_1, unsigned int param_2);
void FUN_00444770(unsigned int param_1, unsigned int param_2);
void FUN_004447b0(unsigned int param_1, unsigned int param_2);
void FUN_004447f0(unsigned int param_1, unsigned int param_2);
void FUN_00444830(unsigned int param_1, unsigned int param_2);
void FUN_00444870(unsigned int param_1, unsigned int param_2);
void FUN_004448b0(unsigned int param_1, unsigned int param_2);
void FUN_004448f0(unsigned int param_1, unsigned int param_2);
void FUN_00444930(unsigned int param_1, unsigned int param_2);
void FUN_00444970(unsigned int param_1, unsigned int param_2);
void LoadAdvisorAnims(void);
void FreeAdvisorAnims(void);
int CheckAppraisalDue(void);
unsigned int SaveReport(void);
unsigned int LoadReport(void);
