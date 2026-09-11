#include <stdbool.h>
#include <stdio.h>

#include "adas_overlay.h"

/* 오버레이가 기준으로 사용하는 화면 해상도 */
#define ADAS_SCREEN_WIDTH   1920.0f
#define ADAS_SCREEN_HEIGHT  1080.0f

/* 화면에 표시할 Safe/Warning 기준선 */
#define ADAS_SAFE_WARNING_LINE_Y  620.0f

/* 사람이 닿으면 ALL STOP을 요청할 중앙 감시 프레임 */
#define ADAS_STOP_FRAME_LEFT    710.0f
#define ADAS_STOP_FRAME_BOTTOM  300.0f
#define ADAS_STOP_FRAME_WIDTH   500.0f
#define ADAS_STOP_FRAME_HEIGHT  480.0f

/* SAFE, WARNING, DANGER 순서의 화면 표시 색상 */
static const float risk_color[3][4] = {
    {0.10f, 0.85f, 0.20f, 0.90f},
    {1.00f, 0.70f, 0.05f, 0.90f},
    {0.95f, 0.10f, 0.08f, 0.90f}
};

static const char *risk_name(AdasOverlayRiskLevel level)
{
    switch (level) {
    case ADAS_OVERLAY_RISK_WARNING:
        return "WARNING";

    case ADAS_OVERLAY_RISK_DANGER:
        return "DANGER";

    case ADAS_OVERLAY_RISK_SAFE:
    default:
        return "SAFE";
    }
}

static void draw_safe_warning_line(struct gl_npu_program npu_prog)
{
    float line_color[4] = {1.00f, 0.70f, 0.05f, 0.85f};

    nc_opengl_draw_line(0.0f,
                        ADAS_SAFE_WARNING_LINE_Y,
                        ADAS_SCREEN_WIDTH,
                        ADAS_SAFE_WARNING_LINE_Y,
                        1,
                        line_color,
                        npu_prog);
}

static void draw_person_stop_frame(struct gl_npu_program npu_prog)
{
    float frame_color[4] = {0.00f, 0.85f, 1.00f, 0.90f};
    float frame_right = ADAS_STOP_FRAME_LEFT + ADAS_STOP_FRAME_WIDTH;
    float frame_top = ADAS_STOP_FRAME_BOTTOM + ADAS_STOP_FRAME_HEIGHT;

    nc_opengl_draw_line(ADAS_STOP_FRAME_LEFT,
                        ADAS_STOP_FRAME_BOTTOM,
                        frame_right,
                        ADAS_STOP_FRAME_BOTTOM,
                        1,
                        frame_color,
                        npu_prog);

    nc_opengl_draw_line(frame_right,
                        ADAS_STOP_FRAME_BOTTOM,
                        frame_right,
                        frame_top,
                        1,
                        frame_color,
                        npu_prog);

    nc_opengl_draw_line(frame_right,
                        frame_top,
                        ADAS_STOP_FRAME_LEFT,
                        frame_top,
                        1,
                        frame_color,
                        npu_prog);

    nc_opengl_draw_line(ADAS_STOP_FRAME_LEFT,
                        frame_top,
                        ADAS_STOP_FRAME_LEFT,
                        ADAS_STOP_FRAME_BOTTOM,
                        1,
                        frame_color,
                        npu_prog);
}

bool adas_person_intersects_stop_frame(float x,
                                       float y,
                                       float width,
                                       float height)
{
    float person_right;
    float person_top;
    float frame_right;
    float frame_top;

    if (width <= 0.0f || height <= 0.0f) {
        return false;
    }

    person_right = x + width;
    person_top = y + height;

    frame_right = ADAS_STOP_FRAME_LEFT + ADAS_STOP_FRAME_WIDTH;
    frame_top = ADAS_STOP_FRAME_BOTTOM + ADAS_STOP_FRAME_HEIGHT;

    /* bbox와 STOP frame이 한 점이라도 닿거나 겹치면 true */
    return (x <= frame_right &&
            person_right >= ADAS_STOP_FRAME_LEFT &&
            y <= frame_top &&
            person_top >= ADAS_STOP_FRAME_BOTTOM);
}

void adas_draw_overlay(const AdasOverlayResult *result,
                       struct gl_npu_program npu_prog,
                       Font *font,
                       struct gl_font_program font_prog)
{
    char status_text[64];
    float text_color[3];
    AdasOverlayRiskLevel level = ADAS_OVERLAY_RISK_SAFE;
    bool all_stop_requested = false;
    float fps = 0.0f;
    float latency_ms = 0.0f;

    if (result != NULL) {
        level = result->level;
        all_stop_requested = result->all_stop_requested;
        fps = result->fps;
        latency_ms = result->latency_ms;
    }

    if (level < ADAS_OVERLAY_RISK_SAFE ||
        level > ADAS_OVERLAY_RISK_DANGER) {
        level = ADAS_OVERLAY_RISK_SAFE;
    }

    /* Overlay는 판단하지 않고 전달받은 결과만 화면에 표시한다. */
    draw_safe_warning_line(npu_prog);
    draw_person_stop_frame(npu_prog);

    text_color[0] = risk_color[level][0];
    text_color[1] = risk_color[level][1];
    text_color[2] = risk_color[level][2];

    snprintf(status_text,
             sizeof(status_text),
             "RISK %s",
             risk_name(level));

    nc_opengl_draw_text(font,
                        status_text,
                        40.0f,
                        980.0f,
                        1.5f,
                        text_color,
                        (int)ADAS_SCREEN_WIDTH,
                        (int)ADAS_SCREEN_HEIGHT,
                        font_prog);

    snprintf(status_text,
             sizeof(status_text),
             "FPS %.1f  NPU %.1f ms",
             fps,
             latency_ms);

    nc_opengl_draw_text(font,
                        status_text,
                        40.0f,
                        1040.0f,
                        0.75f,
                        text_color,
                        (int)ADAS_SCREEN_WIDTH,
                        (int)ADAS_SCREEN_HEIGHT,
                        font_prog);

    if (all_stop_requested) {
        float stop_color[3] = {1.0f, 0.0f, 0.0f};

        nc_opengl_draw_text(font,
                            "ALL STOP",
                            820.0f,
                            560.0f,
                            1.5f,
                            stop_color,
                            (int)ADAS_SCREEN_WIDTH,
                            (int)ADAS_SCREEN_HEIGHT,
                            font_prog);
    }
}
