#ifndef NC_ADAS_RISK_H
#define NC_ADAS_RISK_H

#include "nc_adas_types.h"

/*
 * 객체 하나의 위험도를 판단한다.
 *
 * 현재 1차 구현:
 * - person(class 0)만 판단
 * - bbox bottom 위치를 이용
 *
 * 추후:
 * - car(class 1) 별도 기준 추가
 * - freespace / ego lane geometry 결과 연동
 */
AdasRiskLevel adas_evaluate_object(
    const AdasObject *obj,
    const AdasResult *r
);

/*
 * 현재 frame 안의 모든 객체를 평가하고
 * 가장 높은 위험도를 global_risk로 저장한다.
 */
AdasRiskLevel adas_evaluate_frame(
    AdasResult *r
);

#endif