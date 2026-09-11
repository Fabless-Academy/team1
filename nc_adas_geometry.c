#include <stdlib.h>

#include "nc_adas_geometry.h"


/*
 * Detection bbox에서 객체의 바닥 중앙점 계산
 *
 *        bbox
 *
 *   (x,y)
 *     ┌───────────┐
 *     │           │
 *     │  object   │
 *     │           │
 *     └─────●─────┘
 *           ↑
 *     bottom-center
 */
AdasPoint adas_bottom_center(
    const AdasObject *obj)
{
    AdasPoint p;

    p.x = 0;
    p.y = 0;

    if (obj == NULL) {
        return p;
    }

    /*
     * AdasObject는 nc_adas_extract.c에서
     *
     * src->bbox.x -> obj->x
     * src->bbox.y -> obj->y
     * src->bbox.w -> obj->w
     * src->bbox.h -> obj->h
     *
     * 로 저장된다.
     */
    p.x = (int)(
        obj->x +
        obj->w * 0.5f
    );

    p.y = (int)(
        obj->y +
        obj->h
    );

    return p;
}


/*
 * Bottom-center 등의 Overlay 좌표가
 * Segmentation Freespace 영역인지 검사한다.
 */
int adas_on_freespace(
    AdasPoint p,
    const AdasResult *r)
{
    int mx;
    int my;

    uint8_t mask_value;


    /*
     * 기본 입력 검사
     */
    if (r == NULL) {
        return 0;
    }

    if (r->freespace_mask == NULL) {
        return 0;
    }


    /*
     * Overlay 크기 검사
     */
    if (r->width <= 0 ||
        r->height <= 0) {

        return 0;
    }


    /*
     * Segmentation Mask 크기 검사
     */
    if (r->mask_width <= 0 ||
        r->mask_height <= 0) {

        return 0;
    }


    /*
     * 입력 Point가 Overlay 화면 내부인지 확인
     */
    if (p.x < 0 ||
        p.x >= r->width ||
        p.y < 0 ||
        p.y >= r->height) {

        return 0;
    }


    /*
     * Overlay 좌표
     *
     *       ↓ Scaling
     *
     * Segmentation Mask 좌표
     *
     * Overlay : width × height
     * Mask    : mask_width × mask_height
     */
    mx =
        p.x *
        r->mask_width /
        r->width;

    my =
        p.y *
        r->mask_height /
        r->height;


    /*
     * 변환된 Mask 좌표 범위 확인
     */
    if (mx < 0 ||
        mx >= r->mask_width ||
        my < 0 ||
        my >= r->mask_height) {

        return 0;
    }


    /*
     * Segmentation mask는 1차원 배열
     *
     * index =
     *     y * width + x
     */
    mask_value =
        r->freespace_mask[
            my * r->mask_width + mx
        ];


    /*
     * nc_adas_extract.c에서
     * freespace_value가 설정되어 있다.
     *
     * 현재 값:
     * ADAS_FREESPACE_VALUE = 1
     */
    if (mask_value ==
        r->freespace_value) {

        return 1;
    }

    return 0;
}


/*
 * 하나의 Lane에 대해
 *
 * 특정 y 좌표에서의 Lane x 좌표를 계산한다.
 *
 * Lane Detection 결과는 여러 Point로 구성되어 있으므로
 * 두 Point 사이를 Linear Interpolation한다.
 */
static int lane_x_at_y(
    const AdasLane *lane,
    int y,
    int *out_x)
{
    int i;

    int x0;
    int y0;

    int x1;
    int y1;

    int best_index;
    int best_distance;


    /*
     * 입력 검사
     */
    if (lane == NULL ||
        out_x == NULL) {

        return 0;
    }


    if (lane->point_cnt <= 0) {
        return 0;
    }


    /*
     * nc_adas_types.h 배열 크기를 넘는 값은
     * 정상적인 AdasLane으로 보지 않는다.
     *
     * adas_extract()에서도 이미 최대
     * ADAS_MAX_LANE_POINTS로 제한하고 있다.
     */
    if (lane->point_cnt >
        ADAS_MAX_LANE_POINTS) {

        return 0;
    }


    /*
     * Point가 하나밖에 없다면
     * 그 Point의 x 좌표 사용
     */
    if (lane->point_cnt == 1) {

        *out_x =
            lane->points[0].x;

        return 1;
    }


    /*
     * 주어진 y가 포함되는
     * Lane 선분을 찾는다.
     */
    for (i = 0;
         i < lane->point_cnt - 1;
         i++) {

        x0 =
            lane->points[i].x;

        y0 =
            lane->points[i].y;

        x1 =
            lane->points[i + 1].x;

        y1 =
            lane->points[i + 1].y;


        /*
         * y가 Point i와 i+1 사이인지 확인
         *
         * Lane Point의 순서가 위→아래 또는
         * 아래→위 어느 방향이어도 동작하도록 한다.
         */
        if ((y >= y0 && y <= y1) ||
            (y >= y1 && y <= y0)) {


            /*
             * 두 점의 y가 동일하다면
             * interpolation 불가능
             */
            if (y1 == y0) {

                *out_x = x0;

            } else {

                /*
                 * Linear Interpolation
                 *
                 *           (x1 - x0)(y - y0)
                 * x = x0 + -------------------
                 *                 y1 - y0
                 */
                *out_x =
                    x0 +
                    (x1 - x0) *
                    (y - y0) /
                    (y1 - y0);
            }

            return 1;
        }
    }


    /*
     * 해당 y를 포함하는 Lane 선분이 없는 경우
     *
     * 가장 가까운 y를 가진 Lane Point를 사용한다.
     */
    best_index = 0;

    best_distance =
        abs(
            lane->points[0].y -
            y
        );


    for (i = 1;
         i < lane->point_cnt;
         i++) {

        int distance;

        distance =
            abs(
                lane->points[i].y -
                y
            );


        if (distance <
            best_distance) {

            best_distance =
                distance;

            best_index =
                i;
        }
    }


    *out_x =
        lane->points[
            best_index
        ].x;

    return 1;
}


/*
 * Point가 Ego Lane 내부에 있는지 판단한다.
 *
 * 기준:
 *
 * 1. Point와 같은 y 높이에서 각 Lane의 x를 계산
 *
 * 2. 화면 중앙보다 왼쪽에 있는 Lane들 중
 *    중앙에 가장 가까운 Lane 선택
 *
 * 3. 화면 중앙보다 오른쪽에 있는 Lane들 중
 *    중앙에 가장 가까운 Lane 선택
 *
 * 4. Point.x가 두 Lane 사이이면 Ego Lane으로 판단
 */
int adas_in_ego_lane(
    AdasPoint p,
    const AdasResult *r)
{
    int i;

    int center_x;

    int left_x;
    int right_x;

    int found_left;
    int found_right;

    int lane_x;


    /*
     * 기본 입력 검사
     */
    if (r == NULL) {
        return 0;
    }


    /*
     * Ego Lane을 만들려면
     * 최소 좌/우 2개의 Lane이 필요
     */
    if (r->lane_count < 2) {
        return 0;
    }


    if (r->lane_count >
        ADAS_MAX_LANES) {

        return 0;
    }


    if (r->width <= 0 ||
        r->height <= 0) {

        return 0;
    }


    /*
     * Point의 Overlay 좌표 범위 확인
     */
    if (p.x < 0 ||
        p.x >= r->width ||
        p.y < 0 ||
        p.y >= r->height) {

        return 0;
    }


    /*
     * 차량 중심 ≈ 화면 중앙이라고 가정
     */
    center_x =
        r->width / 2;


    found_left = 0;
    found_right = 0;

    left_x = 0;
    right_x = 0;


    /*
     * 모든 검출 Lane 검사
     */
    for (i = 0;
         i < r->lane_count;
         i++) {


        /*
         * 사람 Foot Point와 동일한 y 높이에서
         * 현재 Lane의 x 위치 계산
         */
        if (!lane_x_at_y(
                &r->lanes[i],
                p.y,
                &lane_x)) {

            continue;
        }


        /*
         * 화면 중앙보다 왼쪽
         */
        if (lane_x < center_x) {

            /*
             * 왼쪽 Lane들 중
             * 화면 중앙에 가장 가까운 Lane 선택
             *
             * 즉 가장 큰 x
             */
            if (!found_left ||
                lane_x > left_x) {

                left_x =
                    lane_x;

                found_left =
                    1;
            }
        }


        /*
         * 화면 중앙보다 오른쪽
         */
        else if (lane_x > center_x) {

            /*
             * 오른쪽 Lane들 중
             * 화면 중앙에 가장 가까운 Lane 선택
             *
             * 즉 가장 작은 x
             */
            if (!found_right ||
                lane_x < right_x) {

                right_x =
                    lane_x;

                found_right =
                    1;
            }
        }
    }


    /*
     * 좌 / 우 Lane 둘 다 필요
     */
    if (!found_left ||
        !found_right) {

        return 0;
    }


    /*
     * 비정상적인 Lane 순서 방지
     */
    if (left_x >= right_x) {
        return 0;
    }


    /*
     *            Person
     *              ●
     *              │
     *
     * Left       Point.x       Right
     *  │            │            │
     *
     * left_x < point.x < right_x
     */
    if (p.x > left_x &&
        p.x < right_x) {

        return 1;
    }


    return 0;
}