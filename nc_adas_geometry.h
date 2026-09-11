#ifndef NC_ADAS_GEOMETRY_H
#define NC_ADAS_GEOMETRY_H

#include <stdint.h>
#include "nc_adas_types.h"

/*
 * Detection bbox의 바닥 중앙 좌표를 계산한다.
 *
 * bbox:
 * x, y = 좌측 상단
 * w, h = width / height
 *
 * return:
 * bottom-center point
 */
AdasPoint adas_bottom_center(
    const AdasObject *obj
);


/*
 * 주어진 Overlay 좌표가
 * Segmentation Freespace 영역인지 확인한다.
 *
 * return:
 * 1 = freespace
 * 0 = freespace 아님 / 유효하지 않은 입력
 */
int adas_on_freespace(
    AdasPoint p,
    const AdasResult *r
);


/*
 * 주어진 Overlay 좌표가
 * Ego Lane 내부인지 확인한다.
 *
 * 화면 중앙을 기준으로
 * 가장 가까운 왼쪽 / 오른쪽 Lane을
 * Ego Lane 경계로 사용한다.
 *
 * return:
 * 1 = ego lane 내부
 * 0 = ego lane 밖 / lane 정보 부족
 */
int adas_in_ego_lane(
    AdasPoint p,
    const AdasResult *r
);

#endif