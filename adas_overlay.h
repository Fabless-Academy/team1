#ifndef ADAS_OVERLAY_H
#define ADAS_OVERLAY_H

#include <stdbool.h>

#include "nc_opengl_interface.h"
#include "nc_opengl_ttf_font.h"

/*
 * adas_overlay가 화면에 표시할 위험 단계입니다.
 *
 * 실제 위험도 판단은 nc_adas_risk.c에서 수행하고,
 * wayland_npu_app.c에서 그 결과를 아래 값으로 변환해 전달합니다.
 * adas_overlay 자체에서는 위험도를 계산하지 않습니다.
 */
typedef enum {
    ADAS_OVERLAY_RISK_SAFE = 0,
    ADAS_OVERLAY_RISK_WARNING,
    ADAS_OVERLAY_RISK_DANGER
} AdasOverlayRiskLevel;

/*
 * 한 프레임에서 overlay에 표시할 정보입니다.
 * wayland_npu_app.c에서 값을 채운 뒤 adas_draw_overlay()에 전달합니다.
 */
typedef struct {
    /* nc_adas_risk의 최종 판단 결과를 변환한 표시용 위험 단계 */
    AdasOverlayRiskLevel level;

    /* 중앙 Person STOP frame과 사람이 겹쳤을 때 true */
    bool all_stop_requested;

    /* 현재 렌더링 FPS */
    float fps;

    /* 실제 NPU 추론 지연 시간(ms). 측정값이 없으면 0.0f 사용 */
    float latency_ms;
} AdasOverlayResult;

/*
 * 사람 bbox가 중앙 STOP frame과 겹치는지 검사합니다.
 *
 * x, y, width, height : 화면 좌표계 기준 bbox
 * return              : 겹치면 true, 아니면 false
 *
 * 내부 상태를 저장하지 않는 함수이므로 매 프레임 별도의 reset이 필요하지 않습니다.
 */
bool adas_person_intersects_stop_frame(float x,
                                       float y,
                                       float width,
                                       float height);

/*
 * ADAS overlay를 화면에 그립니다.
 *
 * result    : 위험 단계, ALL STOP, FPS, NPU latency 정보
 * npu_prog  : 안내선/STOP frame 렌더링용 OpenGL 프로그램
 * font      : 상태 문구 렌더링용 폰트
 * font_prog : 글자 렌더링용 OpenGL 폰트 프로그램
 */
void adas_draw_overlay(const AdasOverlayResult *result,
                       struct gl_npu_program npu_prog,
                       Font *font,
                       struct gl_font_program font_prog);

#endif /* ADAS_OVERLAY_H */
