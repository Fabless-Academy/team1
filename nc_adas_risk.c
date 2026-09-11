#include <stdio.h>

#include "nc_adas_risk.h"

/*
 * Trichimera Detection Class ID
 *
 * 0 : person
 * 1 : car
 * 2 : bus
 * 3 : truck
 * 4 : cycle
 * 5 : motorcycle
 */
#define PERSON_CLASS_ID        (0)
#define CAR_CLASS_ID           (1)

/*
 * 1차 Person Risk 기준
 *
 * bbox bottom 위치를 화면 높이로 나눈 비율을 사용한다.
 *
 * SAFE    : bottom_ratio < 0.55
 * WARNING : 0.55 <= bottom_ratio < 0.78
 * DANGER  : bottom_ratio >= 0.78
 */
#define PERSON_WARNING_THRESHOLD   (0.55f)
#define PERSON_DANGER_THRESHOLD    (0.78f)


AdasRiskLevel adas_evaluate_object(
    const AdasObject *obj,
    const AdasResult *r)
{
    float bottom_y;
    float bottom_ratio;

    /*
     * 잘못된 입력이면 안전 상태로 처리한다.
     */
    if (obj == NULL || r == NULL) {
        return RISK_SAFE;
    }

    if (r->height <= 0) {
        return RISK_SAFE;
    }

    /*
     * 현재 1차 구현에서는
     * person(class 0)만 Risk 판단한다.
     *
     * car 및 나머지 class는 이후 별도로 추가한다.
     */
    if (obj->class_id != PERSON_CLASS_ID) {
        return RISK_SAFE;
    }

    /*
     * bbox
     *
     * x, y : 좌측 상단
     * w, h : bbox 크기
     *
     * bottom_y는 객체 bbox의 가장 아래쪽 위치.
     */
    bottom_y =
        obj->y + obj->h;

    /*
     * overlay 높이를 기준으로
     * 0.0 ~ 1.0 비율로 변환한다.
     */
    bottom_ratio =
        bottom_y / (float)r->height;

    /*
     * 화면 아래쪽에 가까울수록
     * 위험도가 높다고 판단한다.
     */
    if (bottom_ratio >= PERSON_DANGER_THRESHOLD) {

        return RISK_DANGER;

    } else if (
        bottom_ratio >= PERSON_WARNING_THRESHOLD) {

        return RISK_WARNING;
    }

    return RISK_SAFE;
}


AdasRiskLevel adas_evaluate_frame(
    AdasResult *r)
{
    AdasRiskLevel global_risk =
        RISK_SAFE;

    if (r == NULL) {
        return RISK_SAFE;
    }

    /*
     * 현재 frame의 모든 Detection 객체를 확인한다.
     */
    for (int i = 0;
         i < r->object_count;
         i++) {

        const AdasObject *obj =
            &r->objects[i];

        AdasRiskLevel object_risk =
            adas_evaluate_object(
                obj,
                r
            );

        /*
         * 현재 테스트 단계에서는
         * person에 대한 로그만 출력한다.
         */
        if (obj->class_id == PERSON_CLASS_ID) {

            float bottom_y =
                obj->y + obj->h;

            float bottom_ratio =
                0.0f;

            if (r->height > 0) {
                bottom_ratio =
                    bottom_y /
                    (float)r->height;
            }

            printf(
                "[ADAS RISK] "
                "class=%d "
                "prob=%.2f "
                "bbox=(%.3f, %.3f, %.3f, %.3f) "
                "bottom_ratio=%.3f "
                "risk=%d\n",
                obj->class_id,
                obj->confidence,
                obj->x,
                obj->y,
                obj->w,
                obj->h,
                bottom_ratio,
                object_risk
            );
        }

        /*
         * 여러 객체 중 가장 높은 Risk를
         * frame 전체 Risk로 사용한다.
         */
        if (object_risk > global_risk) {
            global_risk =
                object_risk;
        }
    }

    /*
     * UI에서 사용할 수 있도록
     * AdasResult에도 결과를 저장한다.
     */
    r->global_risk =
        global_risk;

    printf(
        "[ADAS RISK] FINAL = %d\n",
        global_risk
    );

    return global_risk;
}